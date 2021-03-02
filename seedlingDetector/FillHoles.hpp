#pragma once

namespace cv { class Mat; }

void fillHoles(cv::Mat& src, const unsigned char foreground = 255, const unsigned char background = 0);