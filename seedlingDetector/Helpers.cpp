#include "Helpers.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

void show(const std::string& name, const Mat& src, const float sizeMult)
{
	namedWindow(name, WINDOW_NORMAL);
	resizeWindow(name, src.cols * sizeMult, src.rows * sizeMult);
	imshow(name, src.clone());
}

///Hist vector de yapýbilir
void getHistogramGrayscale(Mat& src, Mat& hist) {
	const int bins = 256;
	int histSize[] = { bins };
	float range[] = { 0.f, 256.f };
	const float* histRange[] = { range };
	int channels[] = { 0 };
	calcHist(&src, 1, channels, Mat(), hist, 1, histSize, histRange, true, false);
	hist.convertTo(hist, CV_32S);
}

void getHistogramGrayscale(Mat& src, Mat& hist, Mat& mask) {
	const int bins = 256;
	int histSize[] = { bins };
	float range[] = { 0.f, 256.f };
	const float* histRange[] = { range };
	int channels[] = { 0 };
	calcHist(&src, 1, channels, mask, hist, 1, histSize, histRange, true, false);
	hist.convertTo(hist, CV_32S);
}

void getHistogramGrayscale(Mat& src, Mat& hist, const int histSizeBins, const float range[], Mat& mask) {
	int histSize[] = { histSizeBins };
	//float range[] = { 0.f, 256.f };
	const float* histRange[] = { range };
	int channels[] = { 0 };
	calcHist(&src, 1, channels, mask, hist, 1, histSize, histRange, false, false);
	hist.convertTo(hist, CV_32S);
}

std::string type2str(const int type) {
	std::string r;

	const uchar depth = type & CV_MAT_DEPTH_MASK;
	const uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}

void pruneSkeletonEnds(Mat& src, Mat& dst)
{
	dst = imread("C:/images/skel-out.png", IMREAD_GRAYSCALE);
}

Mat lut(1, 256, CV_8UC3);

void binaryToColored(Mat& src, Mat& dst, Vec3b color)
{
	dst.create(src.size(), CV_8UC3);
	cvtColor(src, dst, CV_GRAY2BGR);
	Vec3b* lp = lut.ptr<Vec3b>();
	for (int i = 0; i <= 255; ++i)
		lp[i] = Vec3b(0, 0, 0);
	lp[255] = color;
	LUT(dst, lut, dst);
}