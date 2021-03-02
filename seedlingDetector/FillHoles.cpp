#include "FillHoles.hpp"
#include "opencv2/imgproc.hpp"

using namespace cv;

void fillHoles(cv::Mat& src, const unsigned char foreground, const unsigned char background) {
	CV_Assert(src.type() == CV_8UC1);

	const int width = src.cols;
	const int height = src.rows;

	uchar* ptr0 = src.ptr<uchar>();
	uchar* ptre = src.ptr<uchar>(height - 1);
	for (int i = 0; i < width; i++) {
		if (ptr0[i] == background) floodFill(src, Point(i, 0), 127);
		if (ptre[i] == background) floodFill(src, Point(i, height - 1), 127);
	}
	for (int i = 0; i < height; i++) {
		uchar* ptr = src.ptr<uchar>(i);
		if (ptr[0] == background) floodFill(src, Point(0, i), 127);
		if (ptr[width - 1] == background) floodFill(src, Point(width - 1, i), 127);
	}
	Mat lut(1, 256, CV_8U, foreground);
	lut.at<uchar>(127) = background;
	LUT(src, lut, src);
}