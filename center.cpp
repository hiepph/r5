#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
  /**
   * Read input video from first command line argument
   *      $ ./prog <video_path>
   *
   * Break program immediately and return -1 if no input video specified,
   * or if something error happened (wrong type of video)
   *
   */

    if (argc != 2) {
        printf("ERROR: %s <video_path>", argv[0]);
        return -1;
    }

    string video = argv[1];
    VideoCapture cap(video);
    if (!cap.isOpened()) {
        cout << "ERROR reading " << video << endl;
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

        // Grayscale
        Mat gray;
        cvtColor(crop, gray, CV_BGR2GRAY);

        // Blur
        blur(gray, gray, Size(4, 4));

        // Detect edges
        Mat canny_output;
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        threshold(gray, canny_output, 95, 255, THRESH_BINARY);

        // Find contours
        findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

        // Get the moments
        vector<Moments> mu(contours.size());
        for (size_t i = 0; i < contours.size(); ++i) {
            mu[i] = moments(contours[i], false);
        }

        // Get the mass centers
        vector<Point2d> mc(contours.size());
        for (size_t i = 0; i < contours.size(); ++i) {
            mc[i] = Point2d(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
        }

        // Intialize midpoint
        Point2d midpoint;
        // Then draw contours
        Mat drawing = crop.clone();
        for (size_t i = 0; i < contours.size(); ++i) {
          Scalar contour_color = Scalar(0, 255, 0);
          drawContours(drawing, contours, i, contour_color, 2, 8, hierarchy, 0, Point());
          circle(drawing, mc[i], 4, contour_color, -1, 8, 0);

          // By the way, increase (x, y) of center point by summing 2-side contours
          midpoint += mc[i];
        }

        // Get (x, y) of the  midpoint
        // And draw it
        midpoint *= 0.5;
        circle(drawing, midpoint, 4, Scalar(0, 0, 255), -1, 8, 0);

        // Show frame to windows
        imshow(video, drawing);

        // wait to display frame
        waitKey(20);

        // Offset for midpoint
        // Output frame_id x_center y_center
        cout << current_frame << " " << (int)midpoint.x << " " << (int)(midpoint.y + row_offset) << endl;

        // Increate index of frame
        current_frame++;
    }

    return 0;
}
