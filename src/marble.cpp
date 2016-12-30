/**
 * This is code for processing with marble floor, single lane:
 *  + 03 - Nen da hoa, duong thang vach ngan.avi
 *
 * Though there're a little noise, but real problem is reflected light on the floor.
 * Thus, hard to detect 2-side contours and calculate center point.
 *
 * With problem frame, interpolating (get previous center(x, y)) is a considerable solution.
 *
 * Copyright 2016, @tim Team. All rights reserved.
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <queue>
#include <algorithm>

using namespace std;
using namespace cv;

bool countour_area_comparision(Moments &a, Moments &b) {
  return a.m00 > b.m00;
}

Point2d get_lane_largest_mass_center(
    Mat &input,
    Mat &output,
    double brightness_dec,
    int gblur_ksize,
    int gblur_sigma_x,
    double thresh,
    double width_offset) {
    // Increase brightness
    int x = brightness_dec;
    input += Scalar(-x, -x, -x);

    // Grayscale
    Mat gray;
    cvtColor(input, gray, CV_BGR2GRAY);

    // Blur
    GaussianBlur(gray, gray, Size(gblur_ksize, gblur_ksize), gblur_sigma_x);

    // Detect edges
    Mat canny_output;
    threshold(gray, canny_output, thresh, 255, THRESH_BINARY);

    // Find contours
    vector<Vec4i> hierarchy;
    vector<vector<Point>> contours;
    findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
    if (contours.size() < 1) {
        return Point2d(width_offset, 0);
    }

    // Get the moments
    vector<Moments> mu(contours.size());
    for (size_t i = 0; i < contours.size(); ++i) {
        mu[i] = moments(contours[i], false);
    }

    // Manually skipping light
    sort(mu.begin(), mu.end(), countour_area_comparision);

    // Get the mass centers
    vector<Point2d> mc(contours.size());
    for (size_t i = 0; i < contours.size(); ++i) {
        mc[i] = Point2d(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
    }

    // Draw contours
    // And `ouput` is the target we want to draw on
    for (size_t i = 0; i < contours.size(); ++i) {
        Scalar color = Scalar(0, 255, 0);
        drawContours(output, contours, i, color, 2, 8, hierarchy, 0, Point());
        circle(output, mc[i], 4, color, -1, 8, 0);
    }

    mc[0].x += width_offset;
    return mc[0];
}

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

    /**
     * Output to stdout follow below syntax:
     *
     *      N
     *      frame_id x_center y_center
     *
     * with:
     *      + N: denotes number of frames
     *      + frame_id (int): index of frame, follow 0-index
     *      + x_center (int): x of midpoint's center
     *      + y_center (int): y of midpoint's center
     */

    // Output N
    int frame_count = cap.get(CV_CAP_PROP_FRAME_COUNT);
    cout << frame_count << endl;

    /**
     * Show window with contour drawing for easy watching process:
     */
    namedWindow(video, WINDOW_AUTOSIZE);

    /**
     * Loop for continuously receiving frame
     */
    int current_frame = 0;
    Point2d prevpoint, midpoint;
    while (1) {
        // If cannot read the video, which means it is the end of video!
        Mat frame;
        cap >> frame;
        if (frame.empty()) {
          break;
        }

        // Get only 1/4 bottom rows
        double row_offset = (double)frame.rows * 3 / 4;
        Mat crop = frame(Range(row_offset, frame.rows), Range::all());

        // Split frame into 2 halves
        vector<Mat> half;
        half.push_back(crop(Rect(0, 0, crop.cols / 2, crop.rows)).clone());
        half.push_back(crop(Rect(crop.cols / 2, 0, crop.cols / 2, crop.rows)).clone());

        // Process each half(lane) and get center of lane
        // half_drawing is just for visual purpose
        vector<Mat> half_drawing;
        half_drawing.push_back(half[0]);
        half_drawing.push_back(half[1]);

        Point2d left_lane_center = get_lane_largest_mass_center(half[0], half_drawing[0], 48, 57, 1, 80, 0);
        Point2d right_lane_center = get_lane_largest_mass_center(half[1], half_drawing[1], 53, 17, 3, 115, frame.cols / 2);

        // Join 2 halves for visualizing
        Mat drawing;
        hconcat(half_drawing[0], half_drawing[1], drawing);

        // Get (x, y) of the  midpoint
        // And draw it
        Scalar mid_color = Scalar(0, 0, 255);
        midpoint = (left_lane_center + right_lane_center) * 0.5;

        // Rescue problem frame
        if (current_frame != 0) {
            double dx = abs(midpoint.x - prevpoint.x);
            double dy = abs(midpoint.y - prevpoint.y);

            // go out too far
            if (dx > 7.0) {
                midpoint = prevpoint;
            }
        }

        circle(drawing, midpoint, 4, mid_color, -1, 8, 0);

        // Show frame to windows
        imshow(video, drawing);

        // Offset for midpoint
        // Output: frame_id x_center y_center
        cout << current_frame << " " << (int)midpoint.x << " " << (int)(midpoint.y + row_offset) << endl;

        // Increate index of frame
        current_frame++;

        // Update prev point for interpolating (prevent problem frame)
        prevpoint = midpoint;

        // Wait to display frame
        waitKey(20);
    }

    cap.release();

    return 0;
}
