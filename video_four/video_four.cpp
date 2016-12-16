#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>
#include <algorithm>
#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\opencv.hpp"

using namespace std;
using namespace cv;

bool countour_area_comparision(Moments &a, Moments &b) {
	return a.m00 > b.m00;
}

int main() {
	freopen("video4.log", "w", stdout);

	VideoCapture cap("test4.avi");

	Mat fin;
	Point2f prevMidPoint;
	int currentFrame = 0;
	int frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
	cout << frameCount << endl << endl;
	cerr << frameCount << endl << endl;

	namedWindow("demo1", WINDOW_AUTOSIZE);
	namedWindow("source", WINDOW_AUTOSIZE);
	while (1) {

		// Load frame, slightly blur and threshold
		Mat frame, gray, blurred, temp;
		cap >> frame;
		if (frame.empty()) {
			cerr << "ended!" << endl;
			break;
		}

		frame = frame(Range(frame.rows * 3 / 4, frame.rows), Range::all());
		cvtColor(frame, gray, CV_BGR2GRAY);
		temp = frame;

		// Denoising (slow but works fine)
		fastNlMeansDenoising(gray, gray, 30.0, 7, 13);
		
		// Filter
		Mat canny_output;
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		// Use canny to detect edge
		// Canny(gray, canny_output, 100, 200);
		threshold(gray, canny_output, 150, 255, THRESH_BINARY);
		
		// Find contours
		findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

		// Get the moments
		vector<Moments> mu(contours.size());
		for (int i = 0; i < contours.size(); ++i) {
			mu[i] = moments(contours[i], false);

		}

		// Manually skipping light 
		cout << "(Frame " << currentFrame++ << ") ";
		cout << contours.size() << ": ";
		cerr << "(Frame " << currentFrame << ") ";
		cerr << contours.size() << ": ";
		sort(mu.begin(), mu.end(), countour_area_comparision);
		for (int i = 0; i < contours.size(); ++i) {
			cout << mu[i].m00 << ", ";
			cerr << mu[i].m00 << ", ";
		}
		cout << "(" << abs(mu[0].m00 - mu[1].m00) << " | " << abs(mu[1].m00 - mu[2].m00) << ")";
		cerr << "(" << abs(mu[0].m00 - mu[1].m00) << " | " << abs(mu[1].m00 - mu[2].m00) << ")";


		// Get the mass centers
		vector<Point2f> mc(contours.size());
		for (int i = 0; i < contours.size(); ++i) {
			mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
		}

		// Draw contours
		Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
		for (int i = 0; i < contours.size(); ++i) {
			Scalar color = Scalar(0, 255, 0);
			drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
			circle(drawing, mc[i], 4, color, -1, 8, 0);
		}

		// Drawing midpoint
		Scalar color = Scalar(0, 0, 255);
		Point2f midPoint = (mc[0] + mc[1]) * 0.5;

		cerr << endl;
		cout << endl;
		cerr << "mid: (" << midPoint.x << ", " << midPoint.y << ") - ";
		if (currentFrame != 1) {
			float
				dx = abs(midPoint.x - prevMidPoint.x),
				dy = abs(midPoint.y - prevMidPoint.y);

			cerr << "dx = " << dx << ", " << "dy = " << dy << endl;
			cout << "dx = " << dx << ", " << "dy = " << dy << endl;

			if (dx > 50.0) {
				cerr << "CRASH!!! Reuse previous point" << endl;
				midPoint = prevMidPoint;
			}
		}
		else {
			cerr << endl;
			cout << endl;
		}
		circle(drawing, midPoint, 4, color, -1, 8, 0);
		circle(temp, midPoint, 4, color, -1, 8, 0);

		prevMidPoint = midPoint;
		fin = drawing;

		imshow("demo1", fin);
		imshow("source", temp);

		cerr << endl;
		cout << endl;
		waitKey(20);
	}

	cap.release();

	return 0;
}

