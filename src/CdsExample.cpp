// Cuoc dua so, de thi mau

#include <opencv2\opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

void convolution() {
	Mat src = imread("../data/cuocduaso/example.png", 0);

	// Construct kernel (all entries initialized to 0)
	Mat
		hx(3, 3, CV_32F, Scalar(0)),
		hy(3, 3, CV_32F, Scalar(0));

	// Assign each kernel val
	hx.at<float>(0, 0) = -1;
	hx.at<float>(0, 2) = 1;
	hx.at<float>(1, 0) = -2;
	hx.at<float>(1, 2) = 2;
	hx.at<float>(2, 0) = -1;
	hx.at<float>(2, 2) = 1;

	hy.at<float>(0, 0) = -1;
	hy.at<float>(0, 1) = -2;
	hy.at<float>(0, 2) = -1;
	hy.at<float>(2, 0) = 1;
	hy.at<float>(2, 1) = 2;
	hy.at<float>(2, 2) = 1;

	// Filter jx, jy
	Mat jx, jy;
	filter2D(src, jx, src.depth(), hx);
	filter2D(src, jy, src.depth(), hy);

	// Process the final j
	Mat j;
	j.create(src.size(), src.type());
	for (int r = 0; r < src.rows; r++) {
		for (int c = 0; c < src.cols; c++) {
			j.at<uchar>(r, c) = 
				saturate_cast<uchar>(
					sqrt(pow(jx.at<uchar>(r, c), 2) + pow(jy.at<uchar>(r, c), 2)));
		}
	}


	namedWindow("Source");
	namedWindow("Convolution");
	imshow("Source", src);
	imshow("Convolution", j);

	waitKey(0);
}

int main() {

	convolution();

	

	return 0;
}