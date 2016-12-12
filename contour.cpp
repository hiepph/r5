#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

Mat image, gray;

int thresh = 100;
int max_thresh = 255;

RNG rng(12345);

void thresh_callback(int, void* );

int main( int, char** argv )
{
    string image_name = argv[1];
    image = imread(image_name);
    if (image.empty()) {
        cerr << "No image supplied ..." << endl;
        return -1;
    }

    cvtColor(image, gray, COLOR_BGR2GRAY);
    blur(gray, gray, Size(3,3));

    namedWindow(image_name, WINDOW_AUTOSIZE);

    imshow(image_name, image);
    createTrackbar("Canny thresh:", image_name, &thresh, max_thresh, thresh_callback);
    thresh_callback(0, 0);

    waitKey(0);
    return(0);
}

void thresh_callback(int, void* )
{
    Mat canny_output;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    Canny(gray, canny_output, thresh, thresh * 2, 3);
    findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

    // Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
    Mat drawing = image.clone();
    for(size_t i = 0; i < contours.size(); ++i) {
        Scalar color = Scalar(0, 255, 0); // green
        drawContours(drawing, contours, (int)i, color, 2, 8, hierarchy, 0, Point());
    }

    namedWindow("Contours", WINDOW_AUTOSIZE);
    imshow("Contours", drawing);
}
