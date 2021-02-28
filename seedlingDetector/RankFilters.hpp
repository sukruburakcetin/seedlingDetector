#pragma once

namespace cv { class Mat; }

void medianFilter(cv::Mat& src, cv::Mat& dst, const float radius);
void minFilter(cv::Mat& src, cv::Mat& dst, const float radius);
void maxFilter(cv::Mat& src, cv::Mat& dst, const float radius);
cv::Mat makeKernel(const float radius, int& nPoints);
void meanFilter(cv::Mat& src, cv::Mat& dst, const float radius);