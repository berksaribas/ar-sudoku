#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/ml/ml.hpp">
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;
using namespace cv::ml;


#define LIFESTREAM 0
#define ADAPTIVETHRESHOLD 1
#define RESOLUTION 400
#define SUBPIXEL 0
#define NUMBER 1

const int fps = 30;

const int train_samples = 4;
const int classes = 10;
const int sizex = 20;
const int sizey = 30;
const int ImageSize = sizex * sizey;
char pathToImages[] = "../numbers_images";

void PreProcessImage(Mat* inImage, Mat* outImage, int sizex, int sizey);
void LearnFromImages(Mat* trainData, Mat* trainClasses);
//void RunSelfTest(KNearest& knn2);
void AnalyseImage(KNearest& knearest);

int main() {
	Mat frame;
	Mat bw_frame;
	VideoCapture video;

#if LIFESTREAM
	video.open(0);
#endif

	if (!video.isOpened())
	{
		cout << "No camera was found!\n";
		video.open("../SudokuVideo.MP4");
		if (!video.isOpened()) {
			cout << "No video!" << endl;
			exit(0);
		}
	}

	namedWindow("Sudoku Solver Interface", CV_WINDOW_FREERATIO);
	namedWindow("grid", CV_WINDOW_FREERATIO);

#if !ADAPTIVETHRESHOLD
	int Slider = 180;
	createTrackbar("Threshold", "Sudoku Solver Interface", &Slider, 255, 0);
#endif

	/// <summary>
	/// DATA TRAINING
	/// </summary>
	/// <returns></returns>
	Mat trainData(classes * train_samples, ImageSize, CV_32FC1);
	Mat trainClasses(classes * train_samples, 1, CV_32FC1);

	LearnFromImages(&trainData, &trainClasses);

	Ptr<ml::KNearest> knearest = KNearest::create();
	knearest->train(trainData, ml::ROW_SAMPLE, trainClasses);

	////DATA TRAINING

	while (video.read(frame))
	{
		cvtColor(frame, bw_frame, CV_BGR2GRAY);
#if ADAPTIVETHRESHOLD
		adaptiveThreshold(bw_frame, bw_frame, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 33, 5);
#else
		threshold(bw_frame, bw_frame, Slider, 255, THRESH_BINARY);
#endif

		vector<vector<Point>> contours;
		findContours(bw_frame, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

		vector<Point> approx_contour;
		for (int k = 0; k < contours.size(); k++) {
			approxPolyDP(contours[k], approx_contour, arcLength(contours[k], true) * 0.02, true);
			
			Rect Rectangle = boundingRect(approx_contour);
			if (approx_contour.size() != 4 ||
				Rectangle.height < 200 || Rectangle.width < 200 || Rectangle.width > frame.cols - 10 ||
				Rectangle.height > frame.rows - 10) continue;

			polylines(frame, approx_contour, true, Scalar(0, 0, 255), 1);

			Point2f targetCorners[4];
			targetCorners[0] = { -0.5, -0.5 };
			targetCorners[1] = { RESOLUTION - 0.5, -0.5 };
			targetCorners[2] = { RESOLUTION - 0.5, RESOLUTION - 0.5 };
			targetCorners[3] = { -0.5, RESOLUTION - 0.5 };

			Mat TransMatrix(Size(3, 3), CV_32FC1);
			Point2f corners[4];
			int minWeight = 1e5;
			int minWeightIndex;
			for (int i = 0; i < 4; i++) {
				int weight = approx_contour[i].x + approx_contour[i].y;
				if (minWeight > weight) {
					minWeight = weight;
					minWeightIndex = i;
				}
			}
			for (int i = 0; i < 4; i++) {
				corners[i] = approx_contour[(minWeightIndex + i) % 4];
			}
			circle(frame, corners[0], 10, Scalar(200, 100, 200));

#if SUBPIXEL
			Point2f Inters[4];
			float Parameters[16];
			Mat Lines(Size(4, 4), CV_32F, Parameters);

			for (int i = 0; i < approx_contour.size(); i++) {
				
				Point2f delta = (Point2f(approx_contour[(i + 1) % 4] - approx_contour[i])) / 7.0;
				Point2f direction = Point2f(-delta.y, delta.x);
				direction = direction / norm(direction);

				// minimum length 5 & uneven
				int len = (int)norm(delta) * 1.5;
				Size stripesize;
				stripesize.height = (len < 5) ? 5 : len + 1 - len % 2;
				stripesize.width = 3;
				Mat stripe(stripesize, CV_8UC1);

				Point2f subPixelPos[6];
				for (int n = 1; n < 7; n++) {
					Point2f circle_position = (Point2f)approx_contour[i] + n * delta;


					circle(frame, circle_position, 1, Scalar(255, 255, 0), -1);


					for (int h = 0; h < stripesize.height; h++) {
						float h_rel = h - floorf(stripesize.height / 2);
#if STRIPES
						circle(frame, circle_position + h_rel * direction, 1, color);
#endif
						for (int w = 0; w < stripesize.width; w++) {
							float w_rel = w - floorf(stripesize.width / 2);
							Point2f stripeElement = (circle_position + h_rel * direction + w_rel * Point2f(-direction.y, direction.x));
							Point2i P = Point2i(floorf(stripeElement.x), floorf(stripeElement.y));

							Point2f dP = stripeElement - (Point2f)P;

							if (P.x > 0 && P.x < bw_frame.cols - 1 &&
								P.y > 0 && P.y < bw_frame.rows - 1) {

								stripe.at<uchar>(h, w) = bw_frame.at<uchar>(P.y, P.x) * (1 - dP.x) * (1 - dP.y)
									+ bw_frame.at<uchar>(P.y, P.x + 1) * dP.x * (1 - dP.y)
									+ bw_frame.at<uchar>(P.y + 1, P.x) * (1 - dP.x) * dP.y
									+ bw_frame.at<uchar>(P.y + 1, P.x + 1) * dP.x * dP.y;
							}
							else {
								// Matrix dimensions of bw_frame are exceeded [640 x 360]
								// bw value 0 << 127 << 256
								stripe.at<uchar>(h, w) = 127;
							}
						}
					}
					// Sobel
					vector<double> sobelValues(stripesize.height - 2);
					for (int y = 0; y < stripesize.height - 2; y++) {

						sobelValues[y] =
							// first row
							+1. * stripe.at<uchar>(y, 0)
							+ 2. * stripe.at<uchar>(y, 1)
							+ 1. * stripe.at<uchar>(y, 2)
							// third row
							- 1. * stripe.at<uchar>(y + 2, 0)
							- 2. * stripe.at<uchar>(y + 2, 1)
							- 1. * stripe.at<uchar>(y + 2, 2);
					}
					double x[3];
					double y[3]; // max Intensity
					y[1] = (double)sobelValues[0];
					x[1] = 0.;
					for (int s = 1; s < sobelValues.size(); s++) {
						if (sobelValues[s] >= y[1]) {
							y[1] = (double)sobelValues[s];
							x[1] = s;
						}
					}
					x[0] = x[1] - 1;
					x[2] = x[1] + 1;
					y[0] = (x[1] > 0) ? (double)sobelValues[x[0]] : 0;
					y[2] = (x[2] < sobelValues.size()) ? (double)sobelValues[x[2]] : 0;

					double a = (-y[0] + 2 * y[1] - y[2]) / (-pow(x[0], 2) + 2 * pow(x[1], 2) - pow(x[2], 2));
					double b = y[1] - y[0] + a * (pow(x[0], 2) - pow(x[1], 2));
					double c = y[0] - a * pow(x[0], 2) - b * x[0];
					// f'=2ax+b
					double X = -b / (2. * a);

					subPixelPos[n - 1] = circle_position + (X - x[1]) * direction;
				}

				Mat ExactPoints(Size(1, 6), CV_32FC2, subPixelPos);
				fitLine(ExactPoints, Lines.col(i), CV_DIST_L2, 0, 0.01, 0.01);

				Point2i Point1 = Point2i(Parameters[8 + i] - 50 * Parameters[i], Parameters[12 + i] - 50 * Parameters[4 + i]);
				Point2i Point2 = Point2i(Parameters[8 + i] + 50 * Parameters[i], Parameters[12 + i] + 50 * Parameters[4 + i]);

			}

			float a[2], b[2], c[2], d[2], lambda_1, lambda_2;
			// a + b * lambda_1
			// c + d * lambda_2
			for (int i = 0; i < 4; i++) {
				b[0] = Parameters[i];
				b[1] = Parameters[4 + i];
				a[0] = Parameters[8 + i];
				a[1] = Parameters[12 + i];

				d[0] = Parameters[(i + 1) % 4];
				d[1] = Parameters[4 + (i + 1) % 4];
				c[0] = Parameters[8 + (i + 1) % 4];
				c[1] = Parameters[12 + (i + 1) % 4];

				lambda_2 = (c[1] - a[1] - (c[0] - a[0]) / b[0] * b[1]) / (d[0] * b[1] / b[0] - d[1]);
				lambda_1 = (c[0] - a[0] + lambda_2 * d[0]) / b[0];

				Inters[i].x = a[0] + lambda_1 * b[0];
				Inters[i].y = a[1] + lambda_1 * b[1];
			}
			TransMatrix = getPerspectiveTransform(Inters, targetCorners);
#else		
			TransMatrix = getPerspectiveTransform(corners, targetCorners);
#endif
			Mat grid(Size(RESOLUTION, RESOLUTION), CV_32FC1);
			warpPerspective(bw_frame, grid, TransMatrix, Size(RESOLUTION, RESOLUTION));
			imshow("grid", grid);


#if NUMBER
			// call number recognition program here
			Mat untouchedGrid(Size(RESOLUTION, RESOLUTION), CV_32FC1);
			warpPerspective(frame, untouchedGrid, TransMatrix, Size(RESOLUTION, RESOLUTION));
			imshow("untouchedGrid", untouchedGrid);
			
			float delta = 40.0f ;// ((float)RESOLUTION) / 9.0f;
			for (int c = 0; c < 9; c++) {
				for (int r = 0; r < 9; r++) {
					// subimage of the grid containing one number
					// offset choosen via trial and error
					//grid(Rect(delta* c + 10, delta* r + 10, delta - 15, delta - 13)).copyTo(number);

					//untouchedGrid(Rect(delta* c + 10, delta* r + 10, delta - 15, delta - 13)).copyTo(number);
					Mat number;
					untouchedGrid(Rect(delta* c + 3 * (c + 1), delta* r + (4 * (r + 1)), delta - 1, delta)).copyTo(number);

					Mat sample2(1, ImageSize, CV_32FC1);
					Mat stagedImage;
					PreProcessImage(&number, &stagedImage, number.size().width, number.size().height);

					for (int n = 0; n < ImageSize; n++)
					{
						sample2.at<float>(n) = stagedImage.data[n];
					}
					float detectedClass = knearest->findNearest(sample2, 1, noArray());
					////std::cout <<"{"<<c<<" , "<<r <<"}  :: " << detectedClass << "\n";

					/*if (c == 4 && r == 0)
					{
						imshow("10", number);
						std::cout << "{" << c << " , " << r << "}  :: " << detectedClass << "\n";
					}*/
				}
			}
#endif

		}

		imshow("Sudoku Solver Interface", frame);
		//__debugbreak();
		if (waitKey(fps) == 27) break;
	}

	return 0;
}


void PreProcessImage(Mat* inImage, Mat* outImage, int sizex, int sizey)
{
	Mat grayImage, blurredImage, thresholdImage, contourImage, regionOfInterest;

	vector<vector<Point> > contours;
	cvtColor(*inImage, grayImage, COLOR_BGR2GRAY);
	GaussianBlur(grayImage, blurredImage, Size(5, 5), 2, 2);
	adaptiveThreshold(blurredImage, thresholdImage, 255, 1, 1, 11, 2);
	thresholdImage.copyTo(contourImage);
	findContours(contourImage, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	int idx = 0;
	size_t area = 0;
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (area < contours[i].size())
		{
			idx = i;
			area = contours[i].size();
		}
	}

	Rect rec = boundingRect(contours[idx]);

	regionOfInterest = thresholdImage(rec);

	resize(regionOfInterest, *outImage, Size(sizex, sizey));

}

void LearnFromImages(Mat* trainData, Mat* trainClasses)
{
	Mat img;
	char file[255];
	for (int j = 0; j < train_samples; j++)
	{
		for (int i = 0; i < classes; i++)
		{
			sprintf(file, "%s/%d/%d.png", pathToImages, j, i);
			img = imread(file, 1);
			if (!img.data)
			{
				cout << "File " << file << " not found\n";
				exit(1);
			}
			Mat outfile;
			PreProcessImage(&img, &outfile, img.size().width, img.size().height);
			for (int n = 0; n < ImageSize; n++)
			{
				trainData->at<float>(i * ImageSize + n) = outfile.data[n];
			}
			*trainClasses->ptr<float>(i) = i;
		}
	}
}

//void RunSelfTest(KNearest& knn2)
//{
//	Mat img;
//	Mat sample2(1, ImageSize, CV_32FC1);
//	// SelfTest
//	char file[255];
//	int z = 0;
//	while (z++ < 10)
//	{
//		int iSecret = rand() % 10;
//		//cout << iSecret;
//		sprintf(file, "%s/%d.png", pathToImages, iSecret);
//		img = imread(file, 1);
//		Mat stagedImage;
//		PreProcessImage(&img, &stagedImage, sizex, sizey);
//		for (int n = 0; n < ImageSize; n++)
//		{
//			sample2.at<float>(n) = stagedImage.data[n];
//		}
//		float detectedClass = knn2.findNearest(sample2, 1, noArray());
//		if (iSecret != (int) ((detectedClass)))
//		{
//			cout << "Falsch. Ist " << iSecret << " aber geraten ist "<< (int) ((detectedClass));
//			exit(1);
//		}
//		cout << "Richtig " << (int) ((detectedClass)) << "\n";
//		imshow("single", img);
//		waitKey(0);
//	}
//
//}

void AnalyseImage(KNearest& knearest)
{

	Mat sample2(1, ImageSize, CV_32FC1);

	Mat image, gray, blur, thresh;

	vector < vector<Point> > contours;
	image = imread("../numbers_images/buchstaben.png", 1);

	cvtColor(image, gray, COLOR_BGR2GRAY);
	GaussianBlur(gray, blur, Size(5, 5), 2, 2);
	adaptiveThreshold(blur, thresh, 255, 1, 1, 11, 2);
	findContours(thresh, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	for (size_t i = 0; i < contours.size(); i++)
	{
		vector < Point > cnt = contours[i];
		if (contourArea(cnt) > 50)
		{
			Rect rec = boundingRect(cnt);
			if (rec.height > 28)
			{
				Mat roi = image(rec);
				Mat stagedImage;
				PreProcessImage(&roi, &stagedImage, roi.size().width, roi.size().height);
				for (int n = 0; n < ImageSize; n++)
				{
					*sample2.ptr<float>(n) = stagedImage.data[n];
				}
				float result = knearest.findNearest(sample2, 1, noArray());
				rectangle(image, Point(rec.x, rec.y), Point(rec.x + rec.width, rec.y + rec.height), Scalar(0, 0, 255), 2);

				imshow("all", image);
				cout << result << "\n";

				imshow("single", stagedImage);
				waitKey(0);
			}

		}

	}
}