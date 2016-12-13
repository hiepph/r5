#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("ERROR: %s <video_path>", argv[0]);
        return -1;
    }

    string video = argv[1];
    VideoCapture capture(video);
    if (!capture.isOpened()) {
        cout << "ERROR reading " << video << endl;
        return -1;
    }

    namedWindow(video, WINDOW_AUTOSIZE);
    
    int currentCount = 0;
    int seemRight = 0;
    int frameCount = capture.get(CV_CAP_PROP_FRAME_COUNT);
    cout << frameCount << endl;

    while (1) {
        Mat frame;
        capture >> frame;
        if (frame.empty()) {
            break;
        }

        // Get only 1/4 bottom rows
        double row_offset = (double)frame.rows * 3 / 4;
        Mat crop = frame(Range(row_offset, frame.rows), Range::all());
        Mat gray;
        cvtColor(crop, gray, CV_BGR2GRAY);
        blur(gray, gray, Size(4, 4));

        Mat canny_output;
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;

        // Use canny to detect edge
        threshold(gray, canny_output, 95, 255, THRESH_BINARY); // param3: 80-quite sure for vid1
        findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
        // Get the moments
        vector<Moments> mu(contours.size());
        for (size_t i = 0; i < contours.size(); ++i) {
            mu[i] = moments(contours[i], false);
        }

        // Get the mass centers
        vector<Point2f> mc(contours.size());
        for (size_t i = 0; i < contours.size(); ++i) {
            mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
        }

        // Draw contours
        Point2f midPoint;
        Mat drawing = crop.clone();
        for (size_t i = 0; i < contours.size(); ++i) {
          Scalar color = Scalar(0, 255, 0);
          drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
          circle(drawing, mc[i], 4, color, -1, 8, 0);
          midPoint += mc[i];
        }

        if (contours.size() == 2) {
          Scalar color = Scalar(0, 0, 255);
          midPoint *= 0.5;
          circle(drawing, midPoint, 4, color, -1, 8, 0);
          seemRight++;
        }

        imshow("demo1", drawing);

        // offset for midpoint
        midPoint.y += row_offset; 

        // Debug
        if (contours.size() == 2) {
            cout << currentCount++ << " " << midPoint.x << " " << midPoint.y << endl;
        }

        imshow(video, gray);
        waitKey(20); // wait to display frame
    }

    waitKey(0);
    return 0;
}
