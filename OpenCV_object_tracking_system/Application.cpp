#include <sstream>
#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "Serial.h"

#define len 6

using namespace std;
using namespace cv;

//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
//default capture width and height
const int FRAME_WIDTH = 640;//640;
const int FRAME_HEIGHT = 360;//360;

//for arduino.
const int Xmax = FRAME_WIDTH / 2;
const int Ymax = FRAME_HEIGHT / 2;

int blur_Size = 1;
int SENSITIVITY_VALUE = 0;

//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT * FRAME_WIDTH / 1.5;
//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";

bool calibrationMode;//used for showing debugging windows, trackbars etc.

bool mouseIsDragging;//used for showing a rectangle on screen as user clicks and drags mouse
bool mouseMove;
bool rectangleSelected;

bool objectFound;// true when object detected

cv::Point initialClickPoint, currentMousePoint; //keep track of initial point clicked and current position of mouse
cv::Rect rectangleROI; //this is the ROI that the user has selected
vector<int> H_ROI, S_ROI, V_ROI;// HSV values from the click/drag ROI region stored in separate vectors so that we can sort them easily


void on_trackbar(int, void*)
{//This function gets called whenever a
 // trackbar position is changed
 //for now, this does nothing.
}
void createTrackbars() {
	//create window for trackbars

	namedWindow(trackbarWindowName, WINDOW_GUI_EXPANDED);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf_s(TrackbarName, "H_MIN", H_MIN);
	sprintf_s(TrackbarName, "H_MAX", H_MAX);
	sprintf_s(TrackbarName, "S_MIN", S_MIN);
	sprintf_s(TrackbarName, "S_MAX", S_MAX);
	sprintf_s(TrackbarName, "V_MIN", V_MIN);
	sprintf_s(TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, 255, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, 255, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, 255, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, 255, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, 255, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, 255, on_trackbar);


}
void clickAndDrag_Rectangle(int event, int x, int y, int flags, void* param) {
	//only if calibration mode is true will we use the mouse to change HSV values
	if (1/*calibrationMode == false*/) {
		//get handle to video feed passed in as "param" and cast as Mat pointer
		Mat* videoFeed = (Mat*)param;

		if (event == EVENT_LBUTTONDOWN && mouseIsDragging == false)
		{
			//keep track of initial point clicked
			initialClickPoint = cv::Point(x, y);
			//user has begun dragging the mouse
			mouseIsDragging = true;
		}
		/* user is dragging the mouse */
		if (event == EVENT_MOUSEMOVE && mouseIsDragging == true)
		{
			//keep track of current mouse point
			currentMousePoint = cv::Point(x, y);
			//user has moved the mouse while clicking and dragging
			mouseMove = true;
		}
		/* user has released left button */
		if (event == EVENT_LBUTTONUP && mouseIsDragging == true)
		{
			//set rectangle ROI to the rectangle that the user has selected
			rectangleROI = Rect(initialClickPoint, currentMousePoint);

			//reset boolean variables
			mouseIsDragging = false;
			mouseMove = false;
			rectangleSelected = true;
		}

		if (event == EVENT_RBUTTONDOWN) {
			//user has clicked right mouse button
			//Reset HSV Values
			H_MIN = 0;
			S_MIN = 0;
			V_MIN = 0;
			H_MAX = 255;
			S_MAX = 255;
			V_MAX = 255;

		}
		if (event == EVENT_MBUTTONDOWN) {

			//user has clicked middle mouse button
			//enter code here if needed.
		}
	}

}
void recordHSV_Values(cv::Mat frame, cv::Mat hsv_frame) {

	//save HSV values for ROI that user selected to a vector
	if (mouseMove == false && rectangleSelected == true) {

		//clear previous vector values
		if (H_ROI.size() > 0) H_ROI.clear();
		if (S_ROI.size() > 0) S_ROI.clear();
		if (V_ROI.size() > 0)V_ROI.clear();
		//if the rectangle has no width or height (user has only dragged a line) then we don't try to iterate over the width or height
		if (rectangleROI.width < 1 || rectangleROI.height < 1) {} //cout << "Please drag a rectangle, not a line" << endl;
		else {
			for (int i = rectangleROI.x; i < rectangleROI.x + rectangleROI.width; i++) {
				//iterate through both x and y direction and save HSV values at each and every point
				for (int j = rectangleROI.y; j < rectangleROI.y + rectangleROI.height; j++) {
					//save HSV value at this point
					H_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[0]);
					S_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[1]);
					V_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[2]);
				}
			}
		}
		//reset rectangleSelected so user can select another region if necessary
		rectangleSelected = false;
		//set min and max HSV values from min and max elements of each array

		if (H_ROI.size() > 0) {
			//NOTE: min_element and max_element return iterators so we must dereference them with "*"
			H_MIN = *std::min_element(H_ROI.begin(), H_ROI.end());
			H_MAX = *std::max_element(H_ROI.begin(), H_ROI.end());
			//cout << "MIN 'H' VALUE: " << H_MIN << endl;
			//cout << "MAX 'H' VALUE: " << H_MAX << endl;
		}
		if (S_ROI.size() > 0) {
			S_MIN = *std::min_element(S_ROI.begin(), S_ROI.end());
			S_MAX = *std::max_element(S_ROI.begin(), S_ROI.end());
			//cout << "MIN 'S' VALUE: " << S_MIN << endl;
			//cout << "MAX 'S' VALUE: " << S_MAX << endl;
		}
		if (V_ROI.size() > 0) {
			V_MIN = *std::min_element(V_ROI.begin(), V_ROI.end());
			V_MAX = *std::max_element(V_ROI.begin(), V_ROI.end());
			//cout << "MIN 'V' VALUE: " << V_MIN << endl;
			//cout << "MAX 'V' VALUE: " << V_MAX << endl;
		}

	}

	if (mouseMove == true) {
		//if the mouse is held down, we will draw the click and dragged rectangle to the screen
		rectangle(frame, initialClickPoint, cv::Point(currentMousePoint.x, currentMousePoint.y), cv::Scalar(0, 255, 0), 1, 8, 0);
	}


}
string intToString(int number) {


	std::stringstream ss;
	ss << number;
	return ss.str();
}
void drawObject(int x, int y, Mat& frame) {

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!


	//'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window)

	circle(frame, Point(x, y), 20, Scalar(0, 255, 0), 2);
	if (y - 25 > 0)
		line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);
	if (y + 25 < FRAME_HEIGHT)
		line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
	if (x - 25 > 0)
		line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
	if (x + 25 < FRAME_WIDTH)
		line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);

	putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);

}
void morphOps(Mat& thresh) {

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);



}
void trackFilteredObject(int& x, int& y, Mat threshold, Mat& cameraFeed) {

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	int largestIndex = 0;
	objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects < MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we save a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area > MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea) {
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
					//save index of largest contour to use with drawContours
					largestIndex = index;
				}
				else objectFound = false;


			}
			//let user know you found an object
			if (objectFound == true) {
				putText(cameraFeed, "Tracking Object", Point(0, 40), 1, 1, Scalar(0, 255, 0), 2);
				//draw object location on screen
				drawObject(x, y, cameraFeed);
				//draw largest contour
				//drawContours(cameraFeed, contours, largestIndex, Scalar(0, 255, 255), 2);
			}

		}
		else {
			putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
			objectFound = false;
		}
	}
}

//-----------------------------Arduino---------------------------------------//
void ResetHSV(int* hmax, int* hmin, int* smax, int* smin, int* vmax, int* vmin) {
	*hmax = 255;
	*hmin = 0;
	*smax = 255;
	*smin = 0;
	*vmax = 255;
	*vmin = 0;
}
void showInfor(Mat frame, bool inforP, int X, int Y, int Z, bool arduino, int x, int y) {
	if (inforP) {
		line(frame, Point(0, FRAME_HEIGHT / 2), Point(FRAME_WIDTH, FRAME_HEIGHT / 2), Scalar(0, 255, 0));
		line(frame, Point(FRAME_WIDTH / 2, 0), Point(FRAME_WIDTH / 2, FRAME_HEIGHT), Scalar(0, 255, 0));

		if (objectFound) {
			line(frame, Point(Xmax, Ymax), Point(x, y), Scalar(0, 255, 0));
		}

		putText(frame, "X : " + intToString(X) + "%" + "  Y : " + intToString(Y) + "%" + "  Z : " + intToString(Z) + "  Arduino : " + intToString(arduino), Point(10, 20), 1, 1, Scalar(0, 0, 255), 2);
	}
}
void intTochar(int x, char* ptr) {

	char num[10] = { '0','1','2','3','4','5','6','7','8','9' };

	if (x < 10) {
		*ptr = '0';
		*(ptr + 1) = num[x];
	}
	else if (x < 100) {
		*(ptr) = num[(x - x % 10) / 10];
		*(ptr + 1) = num[x % 10];
	}
	else {
		*ptr = '0';
		*(ptr + 1) = '0';
	}

}
int pluseV(int value) {
	if (value >= 0)  return value;
	else if (value < 0)  return value * (-1);
	else return 0;
}
void Converter(int x, int y, int* X, int* Y, int* Z) {

	x = x - Xmax;
	y = Ymax - y;

	if (x >= 0 && y >= 0) *Z = 0;
	else if (x > 0 && y < 0) *Z = 1;
	else if (x <= 0 && y <= 0) *Z = 2;
	else if (x < 0 && y > 0) *Z = 3;
	else *Z = 0;

	*X = (pluseV(x) * 99) / Xmax;
	*Y = (pluseV(y) * 99) / Ymax;
}
void displayInfor() {
	cout << "____________________________________________________________________";
	cout << "\n Note : \n";
	cout << "         press 'esc' for Exit\n";
	cout << "         press 'C' for calibration Mode\n";
	cout << "         press 'A' for show Information\n";
	cout << "         press 'S' for Reset HSV values\n";
	cout << "         press 'D' for start/stop communicate with arduino\n";
	cout << endl;
	cout << "Drag a rectangle, for filter the colours" << endl;
	cout << "____________________________________________________________________";
	cout << endl;
}
//-------------------------------END-----------------------------------------//

int main(int argc, char** argv) {

	//-----------------------------Arduino---------------------------------------//
	bool inforP = false;//show information
	bool arduino = false;
	int X = 0;//converted data for send X,Y,Z
	int Y = 0;
	int Z = 0;
	char data[len];//char for send to arduino
	char chr[4] = { 'a','b','c','d' };
	char stopM[len] = { '0','0','0','0','e','\n' };
	char left[len] = { '0','0','0','0','l','\n' };
	char right[len] = { '0','0','0','0','r','\n' };
	char up[len] = { '0','0','0','0','u','\n' };
	char down[len] = { '0','0','0','0','p','\n' };

	//Convert char array to LPWSTR
	const char* port = argv[1];
	size_t size = strlen(port) + 1;
	LPWSTR portName = new wchar_t[size];
	size_t outSize;
	mbstowcs_s(&outSize, portName, size, port, size - 1);

	Serial Arduino(portName);

	if (Arduino.IsConnected()) cout << "- Arduino successfully connected - \n" << endl;

	else {
		cout << "\n Arduino is not connected" << endl;
	}

	Sleep(3000);
	//-------------------------------END-----------------------------------------//

	//some boolean variables for different functionality within this
	//program
	bool trackObjects = true;
	bool useMorphOps = true;
	calibrationMode = true;
	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	//matrix storage for HSV image
	Mat HSV;
	//matrix storage for binary threshold image
	Mat threshold;
	//x and y values for the location of the object
	int x = 0, y = 0;
	//video capture object to acquire webcam feed
	VideoCapture capture;
	//Arg 2 camera number
	int cam = atoi(argv[2]);
	//open capture object at location zero (default location for webcam)
	capture.open(cam);
	//set height and width of capture frame
	capture.set(CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	//must create a window before setting mouse callback
	cv::namedWindow(windowName);
	//set mouse callback function to be active on "Webcam Feed" window
	//we pass the handle to our "frame" matrix so that we can draw a rectangle to it
	//as the user clicks and drags the mouse
	cv::setMouseCallback(windowName, clickAndDrag_Rectangle, &cameraFeed);
	//initiate mouse move and drag to false 
	mouseIsDragging = false;
	mouseMove = false;
	rectangleSelected = false;

	//-----------------------------Arduino---------------------------------------//

	displayInfor();

	//-------------------------------END-----------------------------------------//

	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	bool mainOne = true;
	while (mainOne) {
		//store image to matrix
		capture.read(cameraFeed);
		//convert frame from BGR to HSV colorspace
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		//set HSV values from user selected region
		recordHSV_Values(cameraFeed, HSV);
		//filter HSV image between values and store filtered image to
		//threshold matrix
		inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
		//perform morphological operations on thresholded image to eliminate noise
		//and emphasize the filtered object(s)
		if (useMorphOps)morphOps(threshold);
		//pass in thresholded frame to our object tracking function
		//this function will return the x and y coordinates of the
		//filtered object
	//--------------------------------------------------------------------------------------//
		createTrackbar("Blur", "Thresholded Image", &blur_Size, 100);
		createTrackbar("SENSITIVITY", "Thresholded Image", &SENSITIVITY_VALUE, 255);
		if (blur_Size == 0) blur_Size = 1;
		blur(threshold, threshold, Size(blur_Size, blur_Size));
		cv::threshold(threshold, threshold, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		//--------------------------------------------------------------------------------------//
		if (trackObjects)
			trackFilteredObject(x, y, threshold, cameraFeed);

		//show frames 
		if (calibrationMode == true) {

			destroyWindow(windowName1);
			destroyWindow(windowName2);
			destroyWindow(trackbarWindowName);

		}
		else {
			//create slider bars for HSV filtering
			createTrackbars();
			imshow(windowName1, HSV);
			imshow(windowName2, threshold);


		}

		//-----------------------------Arduino---------------------------------------//

		if (objectFound == false || arduino == false) {
			X = 0;
			Y = 0;
			Z = 0;
		}
		else {
			Converter(x, y, &X, &Y, &Z);
		}

		showInfor(cameraFeed, inforP, X, Y, Z, arduino, x, y);

		intTochar(X, &data[0]);
		intTochar(Y, &data[2]);
		data[4] = chr[Z];
		data[len - 1] = '\n';
		if (arduino && Arduino.IsConnected()) {
			Arduino.WriteData(&data[0], len);
		}
		//-------------------------------END-----------------------------------------//

		imshow(windowName, cameraFeed);

		//96+n u=21 j=10 h=8 k=11
		switch (waitKey(ARDUINO_WAIT_TIME)) {
		case 117: //'u'
			if (Arduino.IsConnected() && !arduino) {
				Arduino.WriteData(&up[0], len);
			}
			break;
		case 106: //'j'
			if (Arduino.IsConnected() && !arduino) {
				Arduino.WriteData(&down[0], len);
			}
			break;
		case 104: //'h'
			if (Arduino.IsConnected() && !arduino) {
				Arduino.WriteData(&left[0], len);
			}
			break;
		case 107: //'k'
			if (Arduino.IsConnected() && !arduino) {
				Arduino.WriteData(&right[0], len);
			}
			break;
		case 115:// press 'S' for reset HSV values
			ResetHSV(&H_MAX, &H_MIN, &S_MAX, &S_MIN, &V_MAX, &V_MIN);
			objectFound = false;
			break;
		case 97:// press 'A' for show information
			inforP = !inforP;
			break;
		case 100:// press 'D' start/stop communication with arduino

			if (Arduino.IsConnected()) {
				arduino = !arduino;
				//Arduino.WriteData(&stopM[0], len);
			}

			break;
		case 99://if user presses 'C', toggle calibration mode
			calibrationMode = !calibrationMode;
			break;
		case 27://press esc for exit
			mainOne = false;

			if (Arduino.IsConnected()) {
				Arduino.WriteData(&stopM[0], len);
			}

			capture.release();
			destroyAllWindows();
			cout << "\n-- Programe Terminated --\n";
			break;
		default:
			break;
		}
	}

	Sleep(1000);
	return 0;
}
