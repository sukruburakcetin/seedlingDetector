
//seedlingDetector.cpp : This file contains the 'main' function. Program execution begins and ends there.

/*Show Image and Pause*/

//imshow("Display window", dst);
//int k1 = waitKey(0); // Wait for a keystroke in the window

/*Color Deconvolution*/

//Mat channels[3]; //0: H, 1: DAB, 2: not used
//ColourDeconvolution cd;
//split(cd.process(src, "H DAB", prefs.vector01, prefs.vector02, prefs.vector03, prefs.vector11, prefs.vector12, prefs.vector13), channels);
//channels[2].release();
//channels[2] = NULL;
//dst = channels[1].clone();

/*Connected Component*/

//Mat labels, stats, centroids, doubleStats;
//int count = analyzeParticles(thresholded_dst, labels, stats, centroids, doubleStats, ParticleAnalyzer::FOUR_CONNECTED | ParticleAnalyzer::EXCLUDE_EDGE_PARTICLES, 0);

/*Adaptive Thresholding*/

//adaptiveThreshold(dst, thresholded_dst, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 171, 0);

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "ColourDeconvolution.hpp"
#include "seedlingDetector.hpp"
#include <opencv2/imgproc.hpp>
#include "AutoThreshold.hpp"
#include "ParticleAnalyzer.hpp"
#include "WatershedCustom.hpp";
#include "RankFilters.hpp"

using namespace std;
using namespace cv;

seedlingDetectorResult seedlingDetector(cv::Mat& src, cv::Mat& dst, const seedlingDetectorPreferences& prefs)
{
	seedlingDetectorResult result;
	
	Mat rgbColorSpace, labColorSpace, srcMedianF, srcGausF, filteredImage;

	cvtColor(src, rgbColorSpace, COLOR_RGBA2RGB);
	cvtColor(rgbColorSpace, labColorSpace, COLOR_RGB2Lab);

	Mat lab[3];
	split(labColorSpace, lab);
	src = lab[0];

	medianFilter(src, srcMedianF, 2.f);
	GaussianBlur(srcMedianF, srcGausF, Size(5, 5),1,1);


	Mat thresholded_dst = srcGausF.clone();
	Mat thresholded_dst_new = Mat::zeros(src.size(), CV_8UC1);
	autoThreshold(thresholded_dst, ThresholdMethod::Default);
	thresholded_dst = ~thresholded_dst;
	watershedProcess(thresholded_dst, thresholded_dst, 41);

	Mat thresholded_labels, thresholded_stats, thresholded_centroids, thresholded_doubleStats;
	int count = analyzeParticles(thresholded_dst, thresholded_labels, thresholded_stats, thresholded_centroids, thresholded_doubleStats, ParticleAnalyzer::FOUR_CONNECTED | ParticleAnalyzer::EXCLUDE_EDGE_PARTICLES, 20);

	Mat filterRegion = thresholded_labels > 0;

	int topStart = 200;
	int bottomStart = thresholded_dst.rows - 200;
	for (int i = 0; i < thresholded_dst.cols; i++) {
		for (int j = topStart; j < bottomStart; j++){
			// You can now access the pixel value with cv::Vec3b
			//std::cout << "value: " << thresholded_dst.at<uchar>(i, j) << std::endl;
			cv::circle(thresholded_dst_new, Point(i, j), 0, 255, -1);
		}
	}


	bitwise_and(thresholded_dst_new, filterRegion, filteredImage);
	Mat filteredImage_labels, filteredImage_stats, filteredImage_centroids, filteredImage_doubleStats, roiMask;
	int count2 = analyzeParticles(filteredImage, filteredImage_labels, filteredImage_stats, filteredImage_centroids, filteredImage_doubleStats, ParticleAnalyzer::FOUR_CONNECTED | ParticleAnalyzer::EXCLUDE_EDGE_PARTICLES, 20);
	//Mat roi;
	Mat test = Mat::zeros(src.size(), CV_8UC1);
	Rect bounds;
	int maxHeightRoi = 0;
	for (int i = 1; i < filteredImage_stats.rows; i++) {

	
		
		//const Mat dilateKernel = getStructuringElement(MORPH_ELLIPSE, Size(7, 7));

		int currentHeightOfRoi = filteredImage_stats.at<int>(i, CC_STAT_HEIGHT);
		cout << "Currentheight: " << currentHeightOfRoi << endl;
		if (currentHeightOfRoi > maxHeightRoi)
		{
			maxHeightRoi = currentHeightOfRoi;
			bounds = getRect(filteredImage, filteredImage_stats, i);
			//roi = filteredImage(bounds) == i;
			
		}

		
		//Mat clone_roi = roi.clone();
	

		
		//cout << "centerRoi: " << center << endl;
		
	}

	test(bounds).setTo(255);

	/*int centroidPointX = Point(filteredImage_centroids.at<double>(i, 0), filteredImage_centroids.at<double>(i, 1)).x;
	int centroidPointY = Point(filteredImage_centroids.at<double>(i, 0), filteredImage_centroids.at<double>(i, 1)).y;*/
	//Rect region_rect = Rect(centroidPointX - (filteredImage.cols / 4), centroidPointY - (filteredImage.rows / 4), filteredImage.cols / 2, filteredImage.rows / 2);
	//Mat regionImage = filteredImage(region_rect).clone();
	return result;
}