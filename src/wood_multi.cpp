/**
 * This is code for processing with wood floor, multi lane:
 *  + 05 - Nen go, nhieu lan duong (multilane).avi
 *
 * With problem frame, interpolating (get previous center(x, y)) is a considerable solution.
 *
 * Copyright 2016, @tim Team. All rights reserved.
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>

using namespace std;
using namespace cv;

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
     * Prepare for output video (just a crop of origin)
     */
    VideoWriter ov(
        "center.avi",
        CV_FOURCC('M', 'J', 'P', 'G'),
        // CV_FOURCC('P', 'I', 'M', '1'),
        30,
        Size(cap.get(CV_CAP_PROP_FRAME_WIDTH), (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT) / 4),
        true
    );

    /**
     * Output to stdout follow below syntax:
     *
     *      N
     *      frame_id x_center y_center
     *
     * with:
     *      + N: denotes number of frames
     *      + frame_id: index of frame, follow 0-index
     *      + x_center: x of midpoint's center
     *      + y_center: y of midpoint's center
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

    int default_thresh = 149;
    int thresh = default_thresh;

    int current_frame = 0;
    Point2d prevpoint;
    while (1) {
        // If cannot read the video, which means it is the end of video!
        Mat frame;
        cap >> frame;
        if (frame.empty()) {
            break;
        }

        // We only focus at 1/4 bottom of video, so crop frame
        // Remember to add the offset 3/4 of row (y)
        double row_offset = (double)frame.rows * 3 / 4;
        Mat crop = frame(Range(frame.rows * 3 / 4, frame.rows), Range::all());

        // Grayscale
        Mat gray;
        cvtColor(crop, gray, CV_BGR2GRAY);

        // Blur
        blur(gray, gray, Size(9, 9));

        bool can_stop = false;
        while (!can_stop) {
            Mat threshold_output, temp_gray = gray.clone();
            vector<vector<Point>> contours;
            vector<Vec4i> hierarchy;

            // Detect edges using Threshold
            threshold(gray, threshold_output, thresh, thresh * 2, THRESH_BINARY);

            // Find contours
            findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

            // Get the moments
            vector<Moments> mu(contours.size());
            for (size_t i = 0; i < contours.size(); i++) {
                mu[i] = moments(contours[i], false);
            }

            // Get the mass centers
            Point2d midpoint;
            int valid_points = 0;
            vector<Point2d> mc(contours.size());
            for (size_t i = 0; i < contours.size(); ++i) {
                mc[i] = Point2d(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
                if (mu[i].m00 >= 10 && mc[i].x >= 225.0 && mc[i].y <= 530.0) {
                    valid_points++;
                    midpoint += mc[i];
                }
            }
            midpoint /= valid_points;

            // Manually process several first frames by retaking min_thresh_val
            if (midpoint.x <= 300) {
                thresh = 112;
                gray = temp_gray.clone();
                continue;
            }

            // Draw contours
            Mat drawing = crop.clone();
            for (size_t i = 0; i< contours.size(); i++) {
                Scalar color = Scalar(0, 255, 0);
                drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
                circle(drawing, mc[i], 4, color, -1, 8, 0);
            }

            // Rescue problem frame
            if (current_frame != 0) {
                double dx = abs(midpoint.x - prevpoint.x);
                double dy = abs(midpoint.y - prevpoint.y);

                /// Process by condition (need to be taken in to consider the most)
                if (dx > 15.0) {
                    midpoint = prevpoint;
                    if (dx > 100.0) {
                        midpoint.x -= ((int)dx) / 100;
                    }
                } else if (dx < 10.0 && dy > 10.0) {
                    midpoint = prevpoint;
                    if (11.0 < dy && dy < 35.0) {
                        midpoint.x -= ((int)round(dx)) % 10;
                    }
                } else if (dx > 10.0 && dy > 10.0) {
                    midpoint = prevpoint;
                    midpoint.x -= ((int)round(dx)) % 10;
                }
            }

            circle(drawing, midpoint, 4, Scalar(0, 0, 255), -1, 8, 0);

            // Show frame to windows
            imshow(video, drawing);

            // write drawing to Output
            ov.write(drawing);

            // Offset for midpoint, too
            cout << current_frame << " " << (int)midpoint.x << " " << (int)(midpoint.y + row_offset) << endl;

            // Recover default state
            can_stop = true;
            thresh = default_thresh;

            // Increate index of frame
            current_frame++;

            // Update prev point for interpolating (prevent problem frame)
            prevpoint = midpoint;

            // Wait to display frame
            waitKey(20);
        }
    }

    cap.release();

    return 0;
}
