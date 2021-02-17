#include "ParticleAnalyzer.hpp"
#include "opencv2/imgproc.hpp"
#include "vector"

using namespace cv;

const double PI = 3.14159265358979323846;

void removeEdgeParticles(Mat& src, const bool darkBackground)
{
	const int bg = darkBackground ? 0 : 255;
	const int fg = 255 - bg;
	for (int i = 0; i < src.rows; i++)
	{
		if (src.at<uchar>(i, 0) == bg)
		{
			floodFill(src, Point(0, i), fg);
		}
		if (src.at<uchar>(i, src.cols - 1) == bg)
		{
			floodFill(src, Point(src.cols - 1, i), fg);
		}
	}
	for (int i = 0; i < src.cols; i++)
	{
		if (src.at<uchar>(0, i) == bg)
		{
			floodFill(src, Point(i, 0), fg);
		}
		if (src.at<uchar>(src.rows - 1, i) == bg)
		{
			floodFill(src, Point(i, src.rows - 1), fg);
		}
	}
}

int analyzeParticles(Mat& src, Mat& labels, Mat& stats, Mat& centroids, const int options, int minSize, int maxSize, const double minRelativeSize, const double maxRelativeSize)
{
	Mat emptyMat = Mat();
	return analyzeParticles(src, labels, stats, centroids, emptyMat, options, minSize, maxSize, minRelativeSize, maxRelativeSize, 0, 1);
}

int analyzeParticles(Mat& src, Mat& labels, Mat& stats, Mat& centroids, const Mat& mask, const int options, int minSize, int maxSize, const double minRelativeSize, const double maxRelativeSize)
{
	Mat emptyMat = Mat();
	if (mask.empty()) {
		return analyzeParticles(src, labels, stats, centroids, emptyMat, options, minSize, maxSize, minRelativeSize, maxRelativeSize, 0, 1);
	}
	else {
		CV_Assert(mask.type() == CV_8UC1);
		CV_Assert(mask.size() == src.size());
		Mat maskedSrc = src & mask;
		return analyzeParticles(maskedSrc, labels, stats, centroids, emptyMat, options, minSize, maxSize, minRelativeSize, maxRelativeSize, 0, 1);
	}
}

int analyzeParticles(Mat& src, Mat& labels, Mat& stats, Mat& centroids, Mat& doubleStats, const Mat& mask, const int options, int minSize, int maxSize, const double minRelativeSize, const double maxRelativeSize, const double minCircularity, const double maxCircularity)
{
	if (mask.empty()) {
		return analyzeParticles(src, labels, stats, centroids, doubleStats, options, minSize, maxSize, minRelativeSize, maxRelativeSize, minCircularity, maxCircularity);
	}
	else {
		CV_Assert(mask.type() == CV_8UC1);
		CV_Assert(mask.size() == src.size());
		Mat maskedSrc = src & mask;
		return analyzeParticles(maskedSrc, labels, stats, centroids, doubleStats, options, minSize, maxSize, minRelativeSize, maxRelativeSize, minCircularity, maxCircularity);
	}
}

int analyzeParticles(Mat& src, Mat& labels, Mat& stats, Mat& centroids, Mat& doubleStats, const int options, int minSize, int maxSize, const double minRelativeSize, const double maxRelativeSize, const double minCircularity, const double maxCircularity)
{
	CV_Assert(src.type() == CV_8UC1);
	if (maxSize == 0)
	{
		labels = Mat::zeros(src.size(), CV_32SC1);
		stats.create(1, 6, CV_32SC1);
		int* ptr = stats.ptr<int>();
		ptr[CC_STAT_AREA] = src.cols * src.rows;
		ptr[CC_STAT_LEFT] = 0;
		ptr[CC_STAT_TOP] = 0;
		ptr[CC_STAT_WIDTH] = src.cols;
		ptr[CC_STAT_HEIGHT] = src.rows;
		return 0;
	}
	const bool isFourConnected = options & ParticleAnalyzer::FOUR_CONNECTED;
	const bool excludeEdgeParticles = options & ParticleAnalyzer::EXCLUDE_EDGE_PARTICLES;
	const bool bordered = options & ParticleAnalyzer::BORDERED;

	const bool limitSize = minSize > 0 || maxSize < INT_MAX;
	const bool limitCircularity = minCircularity > 0.0 || maxCircularity < 1.0;
	const bool measureCircularity = options & ParticleAnalyzer::MEASURE_CIRCULARITY || limitCircularity;

	Mat input;
	if (bordered) {
		input = src.clone();
		removeEdgeParticles(input, true);
		input = ~input;
	}
	else if (excludeEdgeParticles) {
		input = src.clone();
		removeEdgeParticles(input);
	}
	else {
		input = src;
	}

	int res = connectedComponentsWithStats(input, labels, stats, centroids, isFourConnected ? 4 : 8, CV_32S);

	if (measureCircularity) {
		doubleStats.create(res, 1, CV_64FC1);
		for (int i = 0; i < res; i++)
		{
			const int area = stats.at<int>(i, CC_STAT_AREA);
			const int perimeter = stats.at<int>(i, CC_STAT_HEIGHT) * 2 + stats.at<int>(i, CC_STAT_WIDTH) * 2;
			const double circularity = perimeter == 0 ? 0.0 : 4.0 * PI * (static_cast<double>(area) / static_cast<double>(perimeter * perimeter));
			doubleStats.at<double>(i, PA_STAT_CIRCULARITY) = circularity;
		}
	}

	if (minRelativeSize > 0 || maxRelativeSize > 0)
	{
		const int totalArea = input.cols * input.rows - stats.at<int>(0, CC_STAT_AREA);
		const double averageArea = static_cast<double>(totalArea) / (stats.rows - 1);
		if (minRelativeSize > 0) minSize = static_cast<int>(minRelativeSize * averageArea);
		if (maxRelativeSize > 0) maxSize = static_cast<int>(maxRelativeSize * averageArea);
	}
	if (limitSize || limitCircularity)
	{
		Mat newStats, newCentroids, newDoubleStats;
		std::vector<Mat> statRows, centroidRows, doubleStatRows;

		statRows.push_back(stats.row(0));
		centroidRows.push_back(centroids.row(0));
		if (measureCircularity) doubleStatRows.push_back(doubleStats.row(0));

		std::vector<int> lookUpTable(res);
		int newi = 1;
		for (int i = 1; i < res; i++)
		{
			const int area = stats.at<int>(i, CC_STAT_AREA);
			bool include = true;
			if (limitSize) {
				include = area >= minSize && area <= maxSize;
			}
			if (limitCircularity || measureCircularity) {
				const double circularity = doubleStats.at<double>(i, PA_STAT_CIRCULARITY);
				include = limitCircularity ? include && circularity >= minCircularity && circularity <= maxCircularity : include;
			}

			if (include)
			{
				statRows.push_back(stats.row(i));
				centroidRows.push_back(centroids.row(i));
				if (measureCircularity) doubleStatRows.push_back(doubleStats.row(i));
				lookUpTable[i] = newi;
				newi++;
			}
		}
		vconcat(statRows.data(), newi, newStats);
		vconcat(centroidRows.data(), newi, newCentroids);
		if (measureCircularity) vconcat(doubleStatRows.data(), newi, newDoubleStats);

		int nRows = labels.rows;
		int nCols = labels.cols;
		if (labels.isContinuous())
		{
			nCols *= nRows;
			nRows = 1;
		}
		//paralel yapýlabilir
		for (int i = 0; i < nRows; ++i)
		{
			int* p = labels.ptr<int>(i);
			for (int j = 0; j < nCols; ++j)
			{
				p[j] = lookUpTable[p[j]];
			}
		}

		newStats.at<int>(0, CC_STAT_AREA) = countNonZero(labels == 0);
		stats = newStats;
		centroids = newCentroids;
		if (measureCircularity) doubleStats = newDoubleStats;
		res = newi;
	}
	return res;
}

cv::Rect getRect(const cv::Mat& labels, const cv::Mat& stats, const unsigned int region, const int rectPadding)
{
	int const* ptr = stats.ptr<int>(region);
	if (rectPadding == 0) {
		return Rect(ptr[CC_STAT_LEFT], ptr[CC_STAT_TOP], ptr[CC_STAT_WIDTH], ptr[CC_STAT_HEIGHT]);
	}
	else {
		int left = ptr[CC_STAT_LEFT] - rectPadding;
		int top = ptr[CC_STAT_TOP] - rectPadding;
		int width = ptr[CC_STAT_WIDTH] + 2 * rectPadding;
		int height = ptr[CC_STAT_HEIGHT] + 2 * rectPadding;
		left = left >= 0 ? left : 0;
		top = top >= 0 ? top : 0;
		width = left + width < labels.cols ? width : labels.cols - 1 - left;
		height = top + height < labels.rows ? height : labels.rows - 1 - top;

		return Rect(left, top, width, height);
	}
}

cv::Mat getRoiMask(const cv::Mat& labels, const cv::Mat& stats, const unsigned int region)
{
	int const* ptr = stats.ptr<int>(region);
	const Rect roi(ptr[CC_STAT_LEFT], ptr[CC_STAT_TOP], ptr[CC_STAT_WIDTH], ptr[CC_STAT_HEIGHT]);
	return labels(roi) == region;
}


cv::Mat randomlyColorParticles(cv::Mat& labels, cv::Mat& stats)
{
	std::vector<Vec3b> colors;
	for (size_t i = 0; i < stats.rows; i++)
	{
		const int b = theRNG().uniform(0, 255);
		const int g = theRNG().uniform(0, 255);
		const int r = theRNG().uniform(0, 255);
		colors.push_back(Vec3b(static_cast<uchar>(b), static_cast<uchar>(g), static_cast<uchar>(r)));
	}
	Mat dst = Mat::zeros(labels.size(), CV_8UC3);
	for (int i = 0; i < labels.rows; i++)
	{
		for (int j = 0; j < labels.cols; j++)
		{
			const int index = labels.at<int>(i, j);
			if (index > 0)
				dst.at<Vec3b>(i, j) = colors[index - 1];
			else
				dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
		}
	}
	return dst;
}

cv::Mat randomlyColorParticles(cv::Mat& labels, const int labelCount)
{
	std::vector<Vec3b> colors;
	for (size_t i = 0; i < labelCount; i++)
	{
		const int b = theRNG().uniform(0, 255);
		const int g = theRNG().uniform(0, 255);
		const int r = theRNG().uniform(0, 255);
		colors.push_back(Vec3b(static_cast<uchar>(b), static_cast<uchar>(g), static_cast<uchar>(r)));
	}
	Mat dst = Mat::zeros(labels.size(), CV_8UC3);
	for (int i = 0; i < labels.rows; i++)
	{
		for (int j = 0; j < labels.cols; j++)
		{
			const int index = labels.at<int>(i, j);
			if (index > 0)
				dst.at<Vec3b>(i, j) = colors[index - 1];
			else
				dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
		}
	}
	return dst;
}