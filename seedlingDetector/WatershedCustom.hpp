#pragma once

namespace cv { class Mat; }

void watershedProcess(cv::Mat& src, cv::Mat& dst, const double tolerance = 20.5);
void findMaximumPoints(cv::Mat& src, cv::Mat& dst, const double tolerance = 20.5);