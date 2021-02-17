#pragma once
#include <climits>
#include <vector>
#include <opencv2/core.hpp>


namespace ParticleAnalyzer
{
	const int EXCLUDE_EDGE_PARTICLES = 1;
	const int FOUR_CONNECTED = 2;
	const int BORDERED = 4;
	const int MEASURE_CIRCULARITY = 8;
}

const int PA_STAT_CIRCULARITY = 0;

/// Returns the number of particles found. Result is stored in labels. If MEASURE_CIRCULARITY is given, doubleStats must also be given.
int analyzeParticles(cv::Mat& src, cv::Mat& labels, cv::Mat& stats, cv::Mat& centroids, cv::Mat& doubleStats, const int options = 0, int minSize = 0, int maxSize = INT_MAX,
	const double minRelativeSize = -1.0, const double maxRelativeSize = -1.0, const double minCircularity = 0.0, const double maxCircularity = 1.0);
int analyzeParticles(cv::Mat& src, cv::Mat& labels, cv::Mat& stats, cv::Mat& centroids, const int options = 0, int minSize = 0, int maxSize = INT_MAX,
	const double minRelativeSize = -1.0, const double maxRelativeSize = -1.0);
int analyzeParticles(cv::Mat& src, cv::Mat& labels, cv::Mat& stats, cv::Mat& centroids, cv::Mat& doubleStats, const cv::Mat& mask, const int options = 0, int minSize = 0, int maxSize = INT_MAX,
	const double minRelativeSize = -1.0, const double maxRelativeSize = -1.0, const double minCircularity = 0.0, const double maxCircularity = 1.0);
int analyzeParticles(cv::Mat& src, cv::Mat& labels, cv::Mat& stats, cv::Mat& centroids, const cv::Mat& mask, const int options = 0, int minSize = 0, int maxSize = INT_MAX,
	const double minRelativeSize = -1.0, const double maxRelativeSize = -1.0);
cv::Rect getRect(const cv::Mat& labels, const cv::Mat& stats, const unsigned int region, const int rectPadding = 0);
cv::Mat getRoiMask(const cv::Mat& labels, const cv::Mat& stats, const unsigned int region);
cv::Mat randomlyColorParticles(cv::Mat& labels, cv::Mat& stats);
cv::Mat randomlyColorParticles(cv::Mat& labels, const int labelCount);
void removeEdgeParticles(cv::Mat& src, const bool darkBackground = false);