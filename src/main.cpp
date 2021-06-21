#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;


#define LIFESTREAM 0
#define ADAPTIVETHRESHOLD 1
#define RESOLUTION 400
#define SUBPIXEL 0
#define NUMBER 1

const int fps = 30;

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
		video.open("SudokuVideo.MP4");
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
			Mat grid(Size(RESOLUTION, RESOLUTION), CV_8UC1);
			warpPerspective(bw_frame, grid, TransMatrix, Size(RESOLUTION, RESOLUTION));
			imshow("grid", grid);

#if NUMBER
			Mat number;
			float delta = RESOLUTION / 9;
			for (int c = 0; c < 9; c++) {
				for (int r = 0; r < 9; r++) {
					// subimage of the grid containing one number
					// offset choosen via trial and error
					grid(Rect(delta * c +10, delta * r +10, delta -15, delta -13)).copyTo(number);
					// imshow("grid", number);

					// call number recognition program here
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