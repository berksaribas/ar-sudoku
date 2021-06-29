#include "main.h"

int image2numbers(Mat& number, Mat& demo_number) {
	Mat quad;
	vector<Point> approx_contour;
	vector<vector<Point>> found_contours;
	vector<vector<Point>> demo_contours;

	findContours(number, found_contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
	demo_contours.clear();
	bool inside = true;
	for (int j = 0; j < found_contours.size(); j++) {
		for (int i = 0; i < found_contours[j].size(); i++) {
			if (found_contours[j][i].x < 1 || found_contours[j][i].y < 1 || // get rid of outer borders
				found_contours[j][i].x > number.size().width - 1 ||
				found_contours[j][i].y > number.size().height - 1) inside = false;
		}
		if (inside) {
			if (arcLength(found_contours[j], true) > 30) {
				demo_contours.push_back(found_contours[j]);
			}
		}
	}

	cv::drawContours(demo_number, demo_contours, -1, Scalar(255, 0, 0), 1, 1);
	switch (demo_contours.size()) {
	case 0: return 0;
	case 1:
		
		if (arcLength(demo_contours[0], true) < 120) return 1;
		else if (arcLength(demo_contours[0], true) < 155) return 7;
		else if ((int)number.at<uchar>(Point(32, 17)) == 255 && (int)number.at<uchar>(Point(34, 17)) == 255
			&& (int)number.at<uchar>(Point(36, 17)) == 255) return 5;
		else if ((int)number.at<uchar>(Point(32, 33)) == 255 && (int)number.at<uchar>(Point(34, 33)) == 255
			&& (int)number.at<uchar>(Point(36, 33)) == 255) return 2;
		else return 3;
		break;
	case 2:
		for (int a = 0; a < 2; a++) {
			if (arcLength(demo_contours[a], true) < 70) {
				if (mean(demo_contours[a])[1] > 29) {
					return 6;	// inside contour in the lower section of the image
					break;
				}
			}
			else {
				if (arcLength(demo_contours[0], true) < 44) {
					return 4;
				}
				else return 9;
			}
		}
		break;
	case 3: return 8;
	default: -1; // error
	}
}

int mostFrequent(vector<int> values) {
	int freq[10] = { 0 };
	int number = 0;
	for (int n = 0; n < values.size(); n++) {
		freq[values.at(n)] ++;
	}
	int max = 0;
	for (int n = 0; n < values.size(); n++) {
		if (freq[values.at(n)] > max) {
			max = freq[n];
			number = n;
		}
	}
	return number;
}