#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>

using namespace std;
using namespace cv;

bool countour_area_comparision(Moments &a, Moments &b) {
    return a.m00 > b.m00;
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

    int current_frame = 0;
    Point2d mid_point, prev_point;
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
        blur(gray, gray, Size(4, 4));

        // Denoising
        fastNlMeansDenoising(gray, gray, 30.0, 7, 13);

        // Detect edges
        Mat canny_output;
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        threshold(gray, canny_output, 150, 255, THRESH_BINARY);

        // Find contours
        findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

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
        Mat drawing = crop.clone();
        for (size_t i = 0; i < contours.size(); ++i) {
            Scalar contour_color = Scalar(0, 255, 0);
            drawContours(drawing, contours, i, contour_color, 2, 8, hierarchy, 0, Point());
            circle(drawing, mc[i], 4, contour_color, -1, 8, 0);
        }

        // Get (x, y) of the  midpoint
        // And draw it
        mid_point = (mc[0] + mc[1]) * 0.5;
        Scalar mid_color = Scalar(0, 0, 255);

        // Rescue problem frame
        if (current_frame != 0) {
          double dx = abs(mid_point.x - prev_point.x);
          double dy = abs(mid_point.y - prev_point.y);

          if ((dx > 50.0) || (dy > frame.rows / 4)) {
            mid_point = prev_point;
          }
        }

        circle(drawing, mid_point, 4, mid_color, -1, 8, 0);

        // Show frame to windows
        imshow(video, drawing);

        // Wait to display frame
        waitKey(20);

        // Increate index of frame
        current_frame++;

        // Update prev point for interpolating (prevent problem frame)
        prev_point = mid_point;

        // Offset for midpoint 
        // Output frame_id x_center y_center
        mid_point.y += row_offset;
        cout << current_frame << " " << mid_point.x << " " << mid_point.y  << endl;
    }

    return 0;
}
