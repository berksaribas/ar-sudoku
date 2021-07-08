#include "main.h"
#include "Renderer.h"
#include "input.h"
#include "UIButton.h"

using namespace std;
using namespace cv;


#define LIVESTREAM 0			// use prerecorded video or camera
#define ADAPTIVETHRESHOLD 1		// use threshold slider or adaptive threshold
#define RESOLUTION 600			// Size of the rectified sudoku grid
#define SUBPIXEL 1				// Interpolation for subpixel accuracy of the corner points
#define NUMBERS 1
#define DEBUGOPTIONS 0

const int fps = 30;				// frames per second of the video
constexpr int WIN_WIDTH = 640;	// window width
constexpr int WIN_HEIGHT = 480;	// window height
constexpr int WIN_FOV = 90;		// fov for the renderer (perspective projection)

constexpr int BTN_WIDTH = 90;	// button width
constexpr int BTN_HEIGHT = 45;	// button height

Input* m_pInput = nullptr;

SquareData* square_data = new SquareData[81];	//This is used to feed the renderer
SudokuData solved_sudoku;						//Our SudokuSolver works with SudokuData
ProgramState state = ProgramState::SCANNING;	//The current state of the program. SCANNING -> SCANNED -> CONFIRMED -> SOLVED
SudokuSolver solver;							//This is the sudoku solver instance
int board[9][9];								//Array to keep recognized numbers

int main() {

	Renderer renderer;
	int n = 0;
	int values[9][9][10];		
	Mat frame;					// image
	Mat bw_frame;				// balck-white image
	VideoCapture video;			// video
    int cubePose=40;           // the center position of the navigator

	Mat TransMatrix(Size(3, 3), CV_32FC1);
	Mat recent_sudoku;


#if LIVESTREAM
	video.open(0);
#endif
	//initializing the renderer and input manager
	renderer.init(WIN_WIDTH, WIN_HEIGHT, WIN_FOV);
	m_pInput = Input::initialize(*renderer.getWindow());

	//Creating buttons
	UIButton m_BtnConfirm(BTN_TYPE::BTN_CONFIRM, BTN_CONDITION::BTN_NORMAL, SquareData(WIN_WIDTH - (BTN_WIDTH * 0.5f), WIN_HEIGHT - (BTN_HEIGHT * 0.5f), BTN_WIDTH, BTN_HEIGHT, 0, true, true));
	UIButton m_BtnHelp(BTN_TYPE::BTN_HELP, BTN_CONDITION::BTN_NORMAL, SquareData(WIN_WIDTH - (2.0f * BTN_WIDTH) - (BTN_WIDTH * 0.5f), WIN_HEIGHT - BTN_HEIGHT * 0.5f, BTN_WIDTH, BTN_HEIGHT, 0, true, true));
	UIButton m_BtnSolution(BTN_TYPE::BTN_SOLUTION, BTN_CONDITION::BTN_NORMAL, SquareData(BTN_WIDTH, WIN_HEIGHT - (BTN_HEIGHT * 0.5f), BTN_WIDTH, BTN_HEIGHT, 0, true, true));
	UIButton m_BtnRescan(BTN_TYPE::BTN_RESCAN, BTN_CONDITION::BTN_NORMAL, SquareData(WIN_WIDTH - (BTN_WIDTH * 0.5f), WIN_HEIGHT - (BTN_HEIGHT * 0.5f) - BTN_HEIGHT, BTN_WIDTH, BTN_HEIGHT, 0, true, true));
	UIButton* m_ArrBtns[BTN_TYPE::BTN_COUNT]{ &m_BtnConfirm, &m_BtnHelp, &m_BtnSolution, &m_BtnRescan };

	//at first no button should be visible.
	m_BtnConfirm.visibilityToggle(false);
	m_BtnHelp.visibilityToggle(false);
	m_BtnSolution.visibilityToggle(false);
	m_BtnRescan.visibilityToggle(false);

	//if no camera available (i.e. LIFESTREAM is 0, use fallback videos)
	if (!video.isOpened())
	{
		cout << "No camera was found!\n";
		video.open("../SudokuVideo.MP4");			// open prerecorded video
        //video.open("/Users/yangzonglin/Ar_project/SudokuVideo.MP4"); //for Mac Path
		if (!video.isOpened()) {
			cout << "No video!" << endl;
			exit(0);
		}
	}

#if DEBUGOPTIONS
	namedWindow("Sudoku Solver Interface", CV_WINDOW_FREERATIO);		// window containing the video and augmentions
	namedWindow("grid", CV_WINDOW_FREERATIO);							// window containing the rectified grid
#endif

#if !ADAPTIVETHRESHOLD
	int Slider = 180;													// slider for manual threshold option
	createTrackbar("Threshold", "Sudoku Solver Interface", &Slider, 255, 0);
#endif


	while (video.read(frame))											// loop through the video for each image
	{
		//Confirm the board -> solve sudoku.
		//Set the program state
		//Show the current buttons for the new state
		if (m_BtnConfirm.manageButton())
		{
			solved_sudoku = solver.produce_sudoku_data(board);
			solved_sudoku = solver.solve(solved_sudoku);

			if (solved_sudoku.answer) {
				for (int r = 0; r < 9; r++) {
					for (int c = 0; c < 9; c++) {
						square_data[c * 9 + r].number = solved_sudoku.board[r][c];
					}
				}

				state = ProgramState::CONFIRMED;
				m_BtnConfirm.visibilityToggle(false);
				m_BtnHelp.visibilityToggle(true);
				m_BtnSolution.visibilityToggle(true);
			}
			else {
				state = ProgramState::SCANNING;
				n = 0;
				m_BtnHelp.visibilityToggle(false);
				m_BtnSolution.visibilityToggle(false);
				m_BtnConfirm.visibilityToggle(false);
			}

		}
		//Show single solution
		if (m_BtnHelp.manageButton())
		{
			square_data[cubePose].number = solved_sudoku.board[cubePose%9][cubePose/9];
			square_data[cubePose].is_visible = true;
		}
		//Show the solution
		if (m_BtnSolution.manageButton())
		{
			for (int r = 0; r < 9; r++) {
				for (int c = 0; c < 9; c++) {
					square_data[c * 9 + r].number = solved_sudoku.board[r][c];
					square_data[c * 9 + r].is_visible = true;
				}
			}
			m_BtnHelp.visibilityToggle(false);
			m_BtnSolution.visibilityToggle(false);
			state = ProgramState::SOLVED;
		}
		//Rescan
		if (m_BtnRescan.manageButton())
		{
			state = ProgramState::SCANNING;
			n = 0;
			m_BtnHelp.visibilityToggle(false);
			m_BtnSolution.visibilityToggle(false);
			m_BtnConfirm.visibilityToggle(false);
		}

		///////////////INPUT

		//Called only once when the key is put down
		//These are used to navigate on the board
		if (Input::IsKeyPutDown(GLFW_KEY_UP))
		{
            if ( cubePose % 9==0)  //prevent top boarder
                cubePose+=8;
            else
                cubePose--;
		}
        if (Input::IsKeyPutDown(GLFW_KEY_DOWN))
        {
            if ( cubePose % 9==8) //prevent bottom boarder
                cubePose-=8;
            else
                cubePose++;
        }
		if (Input::IsKeyPutDown(GLFW_KEY_RIGHT))
		{
            if (cubePose>=72) //prevent right boarder
                cubePose-=72;
            else
                cubePose+=9;
		}
        if (Input::IsKeyPutDown(GLFW_KEY_LEFT))
        {
            if (cubePose<=8) //prevent left boarder
                cubePose+=72;
            else
                cubePose-=9;
        }

		// Getting the pressed number (to edit sudoku values)
		int pressed_number = 0;
		if (Input::IsKeyPutDown(GLFW_KEY_1) || Input::IsKeyPutDown(GLFW_KEY_KP_1)) pressed_number = 1;
		if (Input::IsKeyPutDown(GLFW_KEY_2) || Input::IsKeyPutDown(GLFW_KEY_KP_2)) pressed_number = 2;
		if (Input::IsKeyPutDown(GLFW_KEY_3) || Input::IsKeyPutDown(GLFW_KEY_KP_3)) pressed_number = 3;
		if (Input::IsKeyPutDown(GLFW_KEY_4) || Input::IsKeyPutDown(GLFW_KEY_KP_4)) pressed_number = 4;
		if (Input::IsKeyPutDown(GLFW_KEY_5) || Input::IsKeyPutDown(GLFW_KEY_KP_5)) pressed_number = 5;
		if (Input::IsKeyPutDown(GLFW_KEY_6) || Input::IsKeyPutDown(GLFW_KEY_KP_6)) pressed_number = 6;
		if (Input::IsKeyPutDown(GLFW_KEY_7) || Input::IsKeyPutDown(GLFW_KEY_KP_7)) pressed_number = 7;
		if (Input::IsKeyPutDown(GLFW_KEY_8) || Input::IsKeyPutDown(GLFW_KEY_KP_8)) pressed_number = 8;
		if (Input::IsKeyPutDown(GLFW_KEY_9) || Input::IsKeyPutDown(GLFW_KEY_KP_9)) pressed_number = 9;

		if (pressed_number > 0) {
			//if the scanned sudoku is not confirmed yet, user can change scanned sudoku numbers
			//this is useful if our number recognition fails
			if (state == ProgramState::SCANNED && square_data[cubePose].is_provided) {
				board[cubePose % 9][cubePose / 9] = pressed_number;
				square_data[cubePose].number = pressed_number;
			}
			//if the scanned sudoku is confirmed, then user can change empty boxes on the sudoku
			//i.e. solve sudoku on the run themselves
			else if(state == ProgramState::CONFIRMED && square_data[cubePose].is_provided == false) {
				square_data[cubePose].number = pressed_number;
				square_data[cubePose].is_visible = true;
			}
		}

		//Update
		m_pInput->Update();
		///////////////INPUT


		cvtColor(frame, bw_frame, CV_BGR2GRAY);							// switch to grayscale
#if ADAPTIVETHRESHOLD													// switch threshold option to get a black-white image
		adaptiveThreshold(bw_frame, bw_frame, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 33, 5);
#else
		threshold(bw_frame, bw_frame, Slider, 255, THRESH_BINARY);
#endif

		vector<vector<Point>> contours;
		findContours(bw_frame, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);	// storing corner points of image contours

		vector<Point> approx_contour;
		for (int k = 0; k < contours.size(); k++) {
			approxPolyDP(contours[k], approx_contour, arcLength(contours[k], true) * 0.02, true);	// converting contours to polygons

			Rect Rectangle = boundingRect(approx_contour);
			if (approx_contour.size() != 4 ||																// filter for quadrangles
				Rectangle.height < 100 || Rectangle.width < 100 || Rectangle.width > frame.cols - 10 ||		// and minimum size of 200 pixels in width and height
				Rectangle.height > frame.rows - 10) continue;												// don't exceed frame boundaries
			
			float ratio = norm(approx_contour[0] - approx_contour[1]) / norm(approx_contour[1] - approx_contour[2]); // nearly quadratic
			if (ratio < 0.8 || ratio > 1.3)continue;

			Mat border_test(Size(50, 200), CV_8UC1);		// looking for grid border-lines
			Point2f border_position[4];
			bool noSudokuGrid = false;
			for (int i = 0; i < 1; i++) {
				for (int j = 1; j < 3; j++) {
					border_position[0] = approx_contour[i] + j / 3. * (approx_contour[(i + 1) % 4] - approx_contour[i]) * 0.9;
					border_position[1] = approx_contour[i] + j / 3. * (approx_contour[(i + 1) % 4] - approx_contour[i]) * 1.1;
					border_position[2] = approx_contour[i + 2] + (j % 2 + 1) / 3. * (approx_contour[(i + 3) % 4] - approx_contour[i + 2]) * 0.9;
					border_position[3] = approx_contour[i + 2] + (j % 2 + 1) / 3. * (approx_contour[(i + 3) % 4] - approx_contour[i + 2]) * 1.1;

					Mat TransMatrix(Size(3, 3), CV_32FC1);
					Point2f targetCorners[4];									// extract rectified sudoku border
					targetCorners[0] = { -0.5, -0.5 };
					targetCorners[1] = { 50 - 0.5, -0.5 };
					targetCorners[2] = { 50 - 0.5, 200 - 0.5 };
					targetCorners[3] = { -0.5, 200 - 0.5 };
					TransMatrix = getPerspectiveTransform(border_position, targetCorners);
					warpPerspective(bw_frame, border_test, TransMatrix, Size(50, 200));


					vector<Vec2f> borders;
					Canny(border_test, border_test, 50, 200, 3);
					HoughLines(border_test, borders, 2, CV_PI / 360, 150, 0, 0);	// find lines
					if (borders.size() < 1) {										// break if no line exists
						noSudokuGrid = true;
						break;
					}
				}
			}
			if (noSudokuGrid)continue;


			polylines(frame, approx_contour, true, Scalar(0, 0, 255), 1);		// highlight the found quadrangle in the image

			Point2f targetCorners[4];									// used later for rectifying the sudoku grid
			targetCorners[0] = { -0.5, -0.5 };
			targetCorners[1] = { RESOLUTION - 0.5, -0.5 };
			targetCorners[2] = { RESOLUTION - 0.5, RESOLUTION - 0.5 };
			targetCorners[3] = { -0.5, RESOLUTION - 0.5 };

			
			Point2f corners[4];											// finding the up left corner of the sudoku grid
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
#if DEBUGOPTIONS
			circle(frame, corners[0], 10, Scalar(200, 100, 200));		// highlighting the up left corner of the sudoku grid
#endif

#if SUBPIXEL															// getting more accurate corner points of the sudoku grid
			Point2f Inters[4];
			float Parameters[16];
			Mat Lines(Size(4, 4), CV_32F, Parameters);

			for (int i = 0; i < 4; i++) {

				Point2f delta = (Point2f(corners[(i + 1) % 4] - corners[i])) / 7.0;
				Point2f direction = Point2f(-delta.y, delta.x);
				direction = direction / norm(direction);

				int len = (int)norm(delta) * 1.5;
				Size stripesize;
				stripesize.height = (len < 5) ? 5 : len + 1 - len % 2;	// minimum length 5 & uneven
				stripesize.width = 3;									// fixed width 3
				Mat stripe(stripesize, CV_8UC1);						// creating stripes along the contours

				Point2f subPixelPos[6];
				for (int n = 1; n < 7; n++) {
					Point2f circle_position = (Point2f)corners[i] + n * delta;	// 6 seperation points per contour between two corner points
#if DEBUGOPTIONS
					circle(frame, circle_position, 1, Scalar(255, 255, 0), -1);	// highlight seperator point position in the image
#endif

					for (int h = 0; h < stripesize.height; h++) {				// fill the stripe with intensity values
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

								stripe.at<uchar>(h, w) = bw_frame.at<uchar>(P.y, P.x) * (1 - dP.x) * (1 - dP.y)	// interpolation of intensity values
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
					// Sobel for contours orthogonal to the stripe
					vector<double> sobelValues(stripesize.height - 2);
					for (int y = 0; y < stripesize.height - 2; y++) {

						sobelValues[y] =
							// first row of convolution
							+1. * stripe.at<uchar>(y, 0)
							+ 2. * stripe.at<uchar>(y, 1)
							+ 1. * stripe.at<uchar>(y, 2)
							// third row of convolution
							- 1. * stripe.at<uchar>(y + 2, 0)
							- 2. * stripe.at<uchar>(y + 2, 1)
							- 1. * stripe.at<uchar>(y + 2, 2);
					}
					double x[3];
					double y[3];
					y[1] = (double)sobelValues[0];
					x[1] = 0.;
					for (int s = 1; s < sobelValues.size(); s++) {
						if (sobelValues[s] >= y[1]) {
							y[1] = (double)sobelValues[s]; // max Intensity
							x[1] = s;
						}
					}
					x[0] = x[1] - 1;
					x[2] = x[1] + 1;
					y[0] = (x[1] > 0) ? (double)sobelValues[x[0]] : 0;			// do not exceed stripe boundaries
					y[2] = (x[2] < sobelValues.size()) ? (double)sobelValues[x[2]] : 0;

					// solve y[i] = a x[i]^2 + b x[i] + c
					double a = (-y[0] + 2 * y[1] - y[2]) / (-pow(x[0], 2) + 2 * pow(x[1], 2) - pow(x[2], 2));
					double b = y[1] - y[0] + a * (pow(x[0], 2) - pow(x[1], 2));
					double c = y[0] - a * pow(x[0], 2) - b * x[0];

					// derive f'=2ax+b and solve for x
					double X = -b / (2. * a);

					subPixelPos[n - 1] = circle_position + (X - x[1]) * direction;	// exact position of the separating point
			}

				Mat ExactPoints(Size(1, 6), CV_32FC2, subPixelPos);
				fitLine(ExactPoints, Lines.col(i), CV_DIST_L2, 0, 0.01, 0.01);		// linear regression trough exact sepatation points

				// for drawing the calculated lines
				// Point2i Point1 = Point2i(Parameters[8 + i] - 50 * Parameters[i], Parameters[12 + i] - 50 * Parameters[4 + i]);
				// Point2i Point2 = Point2i(Parameters[8 + i] + 50 * Parameters[i], Parameters[12 + i] + 50 * Parameters[4 + i]);

			}

			float a[2], b[2], c[2], d[2], lambda_1, lambda_2;
			// line equations
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

				// Calculate intersection points
				Inters[(i + 1) % 4].x = a[0] + lambda_1 * b[0];
				Inters[(i + 1) % 4].y = a[1] + lambda_1 * b[1];
			}
			TransMatrix = getPerspectiveTransform(Inters, targetCorners);
			recent_sudoku = frame.clone();
#else
			TransMatrix = getPerspectiveTransform(corners, targetCorners);
#endif
			Mat grid(Size(RESOLUTION, RESOLUTION), CV_8UC1);
			warpPerspective(bw_frame, grid, TransMatrix, Size(RESOLUTION, RESOLUTION));
#if DEBUGOPTIONS
			imshow("grid", grid);
#endif

#if NUMBERS
			if (n < 10) { // average over 10 iterations --> higher number recognition accuracy

				Mat number;
				Mat demo_number;

				float delta = RESOLUTION / 9;
				for (int c = 0; c < 9; c++) {
					for (int r = 0; r < 9; r++) {
						// subimage of the grid containing one number
						// offset choosen via trial and error
						grid(Rect(delta * c + 10, delta * r + 10, delta - 15, delta - 13)).copyTo(number);
						threshold(number, number, 100, 255, THRESH_BINARY); // get rid of gray scale around the borders of the number
						cvtColor(number, demo_number, CV_GRAY2BGR);			// for demo purposes (further functions only work on bw images)

						// call number recognition program here
						values[r][c][n] = image2numbers(number, demo_number);
					}
				}

				// line(demo_number, Point(33, 33), Point(33, 33), Scalar(100, 255, 100));
				// cv::imshow("grid", demo_number);
				// __debugbreak();

				if (n == 9) {
					for (int r = 0; r < 9; r++) {
						for (int c = 0; c < 9; c++) {

							vector<int> input;
							input.clear();
							for (int m = 0; m < 10; m++) {
								input.push_back(values[r][c][m]);
							}
							int recognized_number = mostFrequent(input);;
							board[r][c] = recognized_number;
#if DEBUGOPTIONS
							cout << recognized_number;	// output recognized numbers
							cout << "\t";
#endif
						}
#if DEBUGOPTIONS
						cout << "\n";
#endif
					}

					for (int r = 0; r < 9; r++) {
						for (int c = 0; c < 9; c++) {
							square_data[c * 9 + r] = SquareData(delta * c + 10, delta * r + 10, delta - 15, delta - 13, board[r][c], true, false);
							if (board[r][c] == 0) {
								square_data[c * 9 + r].is_provided = false;
							}
							else {
								square_data[c * 9 + r].is_visible = true;
							}
						}
					}

					state = ProgramState::SCANNED;
					m_BtnConfirm.visibilityToggle(true);
					m_BtnRescan.visibilityToggle(true);
				}
				n++;
			}
#endif
		}
#if DEBUGOPTIONS
		imshow("Sudoku Solver Interface", frame);
#endif

		if (recent_sudoku.empty() || state == ProgramState::SCANNING) {
			glm::mat3 matrix = glm::make_mat3((double*)TransMatrix.data);
			renderer.render(frame, glm::inverse(matrix), nullptr, cubePose, m_ArrBtns);
		}
		else {
			glm::mat3 matrix = glm::make_mat3((double*)TransMatrix.data);
			renderer.render(recent_sudoku, glm::inverse(matrix), square_data, cubePose, m_ArrBtns);
		}

		//__debugbreak();
		if (waitKey(fps) == 27) break;

	}

	m_pInput->destroy();
	m_pInput = nullptr;
	return 0;
}
