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

    Mat edges;
    namedWindow(video, WINDOW_AUTOSIZE);
    while (1) {
        Mat frame;
        capture >> frame;
        if (frame.empty()) {
            break;
        }

        // Get only 1/4 bottom rows
        Mat crop = frame(Range(frame.rows * 3 / 4, frame.rows), Range::all());
        
        cvtColor(crop, edges, COLOR_BGR2GRAY);
        GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);

        imshow(video, edges);
        waitKey(20); // wait to display frame
    }


    waitKey(0);
    return 0;
}
