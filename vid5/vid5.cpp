#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>
#include <queue>

using namespace std;
using namespace cv;

int default_thresh = 149;

int main(int argc, char** argv) {
  /**
   * Read input video from first command line argument
   *      $ ./prog <video_path>
   *
   * Break program immediately and return -1 if no input video specified,
   * or if something error happened (wrong type of video)
   *
   */

  if (argc != 2) {
      printf("error: %s <video_path>", argv[0]);
      return -1;
  }

  string video = argv[1];
  VideoCapture cap(video);
  if (!cap.isOpened()) {
      cout << "error reading " << video << endl;
      return -1;
  }

	int thresh = default_thresh;

	Mat fin;
	int current_frame = 0;
	Point2d prev_point, mid_point;
	namedWindow("demo1", WINDOW_AUTOSIZE);
	namedWindow("source", WINDOW_AUTOSIZE);

	while (1) {
		Mat frame, gray, temp;
		cap >> frame;
		if (frame.empty()) {
			break;
		}

		cout << "Frame " + to_string(current_frame) + ": " << endl;

		frame = frame(Range(frame.rows * 3 / 4, frame.rows), Range::all());
		temp = frame.clone();
		cvtColor(frame, gray, CV_BGR2GRAY);
		blur(gray, gray, Size(9, 9));

		bool can_stop = false;

		while (!can_stop) {
			Mat threshold_output, temp_gray = gray.clone();
			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;

			/// Detect edges using Threshold
			threshold(gray, threshold_output, thresh, thresh * 2, THRESH_BINARY);

			/// Find contours
			findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

			/// Get the moments
			vector<Moments> mu(contours.size());
			for (size_t i = 0; i < contours.size(); i++)
			{
				mu[i] = moments(contours[i], false);

			}

			///  Get the mass centers:
			Point2d mid_point;
			int valid_points = 0;
			vector<Point2d> mc(contours.size());
			for (size_t i = 0; i < contours.size(); i++)
			{
				mc[i] = Point2d(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
				cout << mu[i].m00 << "(" << mc[i].x << "," << mc[i].y << ")\t";
				if (mu[i].m00 >= 10 && mc[i].x >= 225.0 && mc[i].y <= 530.0) { // last: 225.0
					valid_points++;
					mid_point += mc[i];
				}
			}
			mid_point /= valid_points;
			cout << endl << "valid: " << valid_points << endl;
			cout << "calculated_center: (" << mid_point.x << "," << mid_point.y << ")" << endl;

			/// Manually process several first frames by retaking min_thresh_val
			if (mid_point.x <= 300) {
				thresh = 112;
				gray = temp_gray.clone();
				cout << "SOMETHING WRONG!! RECALCULATED..." << endl;
				continue;
			}

			/// Draw contours
			Mat drawing = Mat::zeros(threshold_output.size(), CV_8UC3);
			for (size_t i = 0; i< contours.size(); i++)
			{
				Scalar color = Scalar(0, 255, 0);
				drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
				circle(drawing, mc[i], 4, color, -1, 8, 0);
			}

			/// Rescue problem frame
			if (current_frame != 0) {
				double dx = abs(mid_point.x - prev_point.x);
				double dy = abs(mid_point.y - prev_point.y);

				cout << "\tdx = " << dx << ", " << "dy = " << dy << endl;

				/// Process by condition (need to be taken in to consider the most)
				if (dx > 15.0) {
					cerr << "CRASH!!! Reuse previous point" << endl;
					mid_point = prev_point;
					if (dx > 100.0) {
						mid_point.x -= ((int)dx) / 100;
					}
					cout << "previous_center: (" << mid_point.x << "," << mid_point.y << ")" << endl;

				} else if (dx < 10.0 && dy > 10.0) {
					cerr << "CRASH!!! Reuse previous point" << endl;
					mid_point = prev_point;
					mid_point.x -= dx;
					cout << "previous_center: (" << mid_point.x << "," << mid_point.y << ")" << endl;

				} else if (dx > 10.0 && dy > 10.0) {
					cerr << "CRASH!!! Reuse previous point" << endl;
					mid_point = prev_point;
					// mid_point.x -= dx;
					cout << "previous_center: (" << mid_point.x << "," << mid_point.y << ")" << endl;
				}
			}

			/// ..finally draw
			Scalar color = Scalar(0, 0, 255);
			circle(drawing, mid_point, 4, color, -1, 8, 0);
			circle(temp, mid_point, 4, color, -1, 8, 0);

			fin = drawing.clone();
			imshow("demo1", fin);
			imshow("source", temp);

			/// Recover default state
			can_stop = true;
			thresh = default_thresh;

			/// Change to some val > 10000 to comfortably see each frame
			waitKey(20);

			current_frame++;

			prev_point = mid_point;

			cout << endl;
		}
	}

	cap.release();

	return 0;
}
