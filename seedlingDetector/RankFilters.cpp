#include "RankFilters.hpp"
#include "RankFilters.hpp"
#include "opencv2/imgproc.hpp"
#include "LambdaParallel.hpp"

#include "opencv2/highgui.hpp"
#include <iostream>
#include "LambdaParallel.hpp"

using namespace cv;

int* makeLineRadii(float radius, int& kLength)
{
	if (radius >= 1.5f && radius < 1.75f) //this code creates the same sizes as the previous RankFilters
		radius = 1.75f;
	else if (radius >= 2.5f && radius < 2.85f)
		radius = 2.85f;
	const int r2 = static_cast<int>(radius * radius) + 1;
	const int kRadius = static_cast<int>(sqrt(r2 + 1e-10));
	const int kHeight = 2 * kRadius + 1;
	kLength = 2 * kHeight + 2;
	int* kernel = new int[kLength];
	kernel[2 * kRadius] = -kRadius;
	kernel[2 * kRadius + 1] = kRadius;
	int nPoints = 2 * kRadius + 1;
	for (int y = 1; y <= kRadius; y++)
	{ //lines above and below center together
		const int dx = static_cast<int>(sqrt(r2 - y * y + 1e-10));
		kernel[2 * (kRadius - y)] = -dx;
		kernel[2 * (kRadius - y) + 1] = dx;
		kernel[2 * (kRadius + y)] = -dx;
		kernel[2 * (kRadius + y) + 1] = dx;
		nPoints += 4 * dx + 2; //2*dx+1 for each line, above&below
	}
	kernel[kLength - 2] = nPoints;
	kernel[kLength - 1] = kRadius;
	return kernel;
}

void medianFilter(Mat& src, Mat& dst, const float radius)
{
	CV_Assert(src.depth() == CV_8U);
	CV_Assert(src.channels() == 1);
	int kLength;
	int* lineRadii = makeLineRadii(radius, kLength);
	Mat inp;
	if (src.data == dst.data) {
		inp = src.clone();
	}
	else {
		inp = src;
		dst.create(src.size(), src.type());
	}

	int kHeight = (kLength - 2) / 2;
	int kRadius = (kHeight - 1) / 2;
	parallel_for(Range(0, inp.rows), [&](const Range& range)
		{
			std::vector<uchar> values;
			values.reserve(lineRadii[kLength - 2]);
			for (int i = range.start; i < range.end; i++)
			{
				for (int j = 0; j < inp.cols; j++)
				{
					for (int y = 0; y < kHeight; y++)
					{
						int yIndex = i + y - kRadius;
						if (yIndex < 0 || yIndex >= inp.rows) continue;
						for (int x = lineRadii[y * 2]; x <= lineRadii[y * 2 + 1]; x++)
						{
							int xIndex = j + x;
							if (xIndex < 0 || xIndex >= inp.cols) continue;
							values.push_back(inp.at<uchar>(yIndex, xIndex));
						}
					}
					std::sort(values.begin(), values.end());
					dst.at<uchar>(i, j) = values[values.size() / 2];
					values.clear();
				}
			}
		});

	delete[] lineRadii;
}

void minFilter(Mat& src, Mat& dst, const float radius)
{
	CV_Assert(src.depth() == CV_8U);
	//CV_Assert(src.isContinuous());
	int kLength;
	int* lineRadii = makeLineRadii(radius, kLength);
	Mat inp;
	if (src.data == dst.data) {
		inp = src.clone();
	}
	else {
		inp = src;
		dst.create(src.size(), src.type());
	}

	int kHeight = (kLength - 2) / 2;
	int kRadius = (kHeight - 1) / 2;

	int width = inp.cols;
	//uchar *srcPtr = src.ptr<uchar>();
	//uchar *dstPtr = dst.ptr<uchar>();
	parallel_for(Range(0, inp.rows), [&](const Range& range)
		{
			std::vector<uchar> values;
			values.reserve(lineRadii[kLength - 2]);
			for (int i = range.start; i < range.end; i++)
			{
				for (int j = 0; j < width; j++)
				{
					for (int y = 0; y < kHeight; y++)
					{
						int yIndex = i + y - kRadius;
						if (yIndex < 0 || yIndex >= inp.rows) continue;
						for (int x = lineRadii[y * 2]; x <= lineRadii[y * 2 + 1]; x++)
						{
							int xIndex = j + x;
							if (xIndex < 0 || xIndex >= inp.cols) continue;
							values.push_back(inp.at<uchar>(yIndex, xIndex));
							//values.push_back(srcPtr[yIndex*width + xIndex]);
						}
					}
					dst.at<uchar>(i, j) = *std::min_element(values.begin(), values.end());
					//printf("S: %d, min: %d\n", values.size() , (int)dst.at<uchar>(i, j));
					//dstPtr[i*width + j] = *std::min_element(values.begin(), values.end());
					values.clear();
				}
			}
		});

	delete[] lineRadii;
}

void maxFilter(Mat& src, Mat& dst, const float radius)
{
	CV_Assert(src.depth() == CV_8U);
	//CV_Assert(src.isContinuous());
	int kLength;
	int* lineRadii = makeLineRadii(radius, kLength);
	Mat inp;
	if (src.data == dst.data) {
		inp = src.clone();
	}
	else {
		inp = src;
		dst.create(src.size(), src.type());
	}

	int kHeight = (kLength - 2) / 2;
	int kRadius = (kHeight - 1) / 2;

	int width = inp.cols;
	//uchar *srcPtr = src.ptr<uchar>();
	//uchar *dstPtr = dst.ptr<uchar>();
	parallel_for(Range(0, inp.rows), [&](const Range& range)
		{
			std::vector<uchar> values;
			values.reserve(lineRadii[kLength - 2]);
			for (int i = range.start; i < range.end; i++)
			{
				for (int j = 0; j < width; j++)
				{
					for (int y = 0; y < kHeight; y++)
					{
						int yIndex = i + y - kRadius;
						if (yIndex < 0 || yIndex >= inp.rows) continue;
						for (int x = lineRadii[y * 2]; x <= lineRadii[y * 2 + 1]; x++)
						{
							int xIndex = j + x;
							if (xIndex < 0 || xIndex >= inp.cols) continue;
							values.push_back(inp.at<uchar>(yIndex, xIndex));
							//values.push_back(srcPtr[yIndex*width + xIndex]);
						}
					}
					dst.at<uchar>(i, j) = *std::max_element(values.begin(), values.end());
					//printf("S: %d, min: %d\n", values.size() , (int)dst.at<uchar>(i, j));
					//dstPtr[i*width + j] = *std::min_element(values.begin(), values.end());
					values.clear();
				}
			}
		});

	delete[] lineRadii;
}

/*
void minFilter(Mat& src, Mat& dst, float radius)
{
Mat kernel = makeKernel(radius);
erode(src, dst, kernel);
}*/

void meanFilter(Mat& src, Mat& dst, const float radius)
{
	int nPoints;
	Mat kernel = makeKernel(radius, nPoints);
	kernel /= nPoints;
	filter2D(src, dst, -1, kernel);
}


Mat makeKernel(const float radius, int& nPoints)
{
	int kLength;
	int* lineRadii = makeLineRadii(radius, kLength);

	nPoints = lineRadii[kLength - 2];

	const int kHeight = (kLength - 2) / 2;
	const int kRadius = (kHeight - 1) / 2;

	Mat kernel = Mat::zeros(kHeight, kHeight, CV_32FC1);
	for (int y = 0; y < kHeight; y++)
	{
		for (int x = lineRadii[y * 2]; x <= lineRadii[y * 2 + 1]; x++)
		{
			kernel.at<float>(y, x + kRadius) = 1.f;
		}
	}

	delete[] lineRadii;
	return kernel;
}