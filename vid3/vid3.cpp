#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>
#include <queue>
#include <algorithm>
#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\opencv.hpp"


using namespace std;
using namespace cv;

bool countour_area_comparision(Moments &a, Moments &b) {
	return a.m00 > b.m00;
}

Point2d get_lane_largest_mass_center(Mat &frame, double brightness_dec, int gblur_ksize, int gblur_sigma_x, double thresh, double width_offset) {
	int x = brightness_dec;
	frame += Scalar(-x, -x, -x);

	GaussianBlur(frame, frame, Size(gblur_ksize, gblur_ksize), gblur_sigma_x);

	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	threshold(frame, canny_output, thresh, 255, THRESH_BINARY);

	// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
	if (contours.size() < 1) {
		frame = Mat::zeros(canny_output.size(), CV_8UC3);
		return Point2d(width_offset, 0);
	}

	// Get the moments
	vector<Moments> mu(contours.size());
	for (int i = 0; i < contours.size(); ++i) {
		mu[i] = moments(contours[i], false);
	}

	// Manually skipping light
	sort(mu.begin(), mu.end(), countour_area_comparision);

	// Get the mass centers
	vector<Point2d> mc(contours.size());
	for (int i = 0; i < contours.size(); ++i) {
		mc[i] = Point2d(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
	}

	// Draw contours
	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); ++i) {
		Scalar color = Scalar(0, 255, 0);
		drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
		circle(drawing, mc[i], 4, color, -1, 8, 0);
	}

	frame = drawing;

	mc[0].x += width_offset;
	return mc[0];
}

int main() {
	VideoCapture cap("test3.avi");

	int frame_count = cap.get(CV_CAP_PROP_FRAME_COUNT);
	cout << frame_count << endl;

	Mat fin;
	int current_frame = 0;
	Point2d prev_point, mid_point;
	namedWindow("demo1", WINDOW_AUTOSIZE);
	namedWindow("source", WINDOW_AUTOSIZE);

	while (1) {
		// If cannot read the video, which means it is the end of video!
		Mat frame, temp;
		cap >> frame;
		if (frame.empty()) {
			break;
		}

		cout << "Frame " << current_frame << ":" << endl;

		// We only focus at 1/4 bottom of video, so crop frame
		// Remember to add the offset 3/4 of row (y)
		double row_offset = (double)frame.rows * 3 / 4;
		frame = frame(Range(frame.rows * 3 / 4, frame.rows), Range::all());
		temp = frame;

		// Grayscale
		cvtColor(frame, frame, CV_BGR2GRAY);

		// Split frame into 2 halves
		vector<Mat> half;
		half.push_back(frame(Rect(0, 0, frame.cols / 2, frame.rows)).clone());
		half.push_back(frame(Rect(frame.cols / 2, 0, frame.cols / 2, frame.rows)).clone());

		// Process each half(lane) and get center of lane
		Point2d
			left_lane_center = get_lane_largest_mass_center(half[0], 48, 57, 1, 80, 0),
			right_lane_center = get_lane_largest_mass_center(half[1], 53, 17, 3, 115, frame.cols / 2);
		hconcat(half[0], half[1], fin);

		// Get (x, y) of the  midpoint
		// And draw it
		Scalar mid_color = Scalar(0, 0, 255);
		mid_point = (left_lane_center + right_lane_center) * 0.5;
		cout << "\tmid: (" << mid_point.x << ", " << mid_point.y << ")" << endl;

		// Rescue problem frame
		if (current_frame != 0) {
			double dx = abs(mid_point.x - prev_point.x);
			double dy = abs(mid_point.y - prev_point.y);

			cout << "\tdx = " << dx << ", " << "dy = " << dy << endl;

			if (dx > 7.0) {
				cerr << "CRASH!!! Reuse previous point" << endl;
				mid_point = prev_point;
			}
		}

		// ..draw
		circle(fin, mid_point, 4, mid_color, -1, 8, 0);
		circle(temp, mid_point, 4, mid_color, -1, 8, 0);
		
		// Show frame to windows
		imshow("demo1", fin);
		imshow("source", temp);

		// Wait to display frame
		waitKey(20);

		cout << endl;

		// Increate index of frame
		current_frame++;

		// Update prev point for interpolating (prevent problem frame)
		prev_point = mid_point;

		waitKey(20);
	}

	cap.release();

	return 0;
}
