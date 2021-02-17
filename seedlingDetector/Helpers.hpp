#pragma once
#include <string>
#include <opencv2/core/mat.hpp>

void show(const std::string& name, const cv::Mat& src, const float sizeMult = 0.5);
void getHistogramGrayscale(cv::Mat& src, cv::Mat& hist, const int histSizeBins, const float range[], cv::Mat& mask);
void getHistogramGrayscale(cv::Mat& src, cv::Mat& hist, cv::Mat& mask);
void getHistogramGrayscale(cv::Mat& src, cv::Mat& hist);
std::string type2str(const int type);
void pruneSkeletonEnds(cv::Mat& src, cv::Mat& dst);
void binaryToColored(cv::Mat& src, cv::Mat& dst, cv::Vec3b color);