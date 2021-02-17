#pragma once
namespace cv { class Mat; }
enum class ThresholdMethod {
	Default, Huang, Shanbhag, Li, Minimum, MaxEntropy, Otsu
};

int autoThreshold(cv::Mat& src, const ThresholdMethod method, const bool darkBackground = false);
int getThreshold(cv::Mat& src, const ThresholdMethod method = ThresholdMethod::Default);
int defaultIsoData(cv::Mat& hist);
int IJIsoData(cv::Mat& hist);
int Huang(cv::Mat& hist);
int Shanbhag(cv::Mat& hist);
int Li(cv::Mat& hist);
int Minimum(cv::Mat& hist);
int MaxEntropy(cv::Mat& hist);
int Otsu(cv::Mat& hist);