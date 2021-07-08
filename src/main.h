#ifndef NUMBERRECOGNITION_H
#define NUMBERRECOGNITION_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "SudokuSolver.h"

using namespace std;
using namespace cv;

enum ProgramState {
	SCANNING, SCANNED, CONFIRMED, SOLVED
};

int image2numbers(Mat& number, Mat& demo_number);
int mostFrequent(vector<int> values);

#endif