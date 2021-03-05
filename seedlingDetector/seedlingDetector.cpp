
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
#include "FillHoles.hpp"

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
	GaussianBlur(srcMedianF, srcGausF, Size(5, 5), 1, 1);

	Mat thresholded_dst = srcGausF.clone();
	Mat thresholded_dst_new = Mat::zeros(src.size(), CV_8UC1);
	autoThreshold(thresholded_dst, ThresholdMethod::Default);
	thresholded_dst = ~thresholded_dst;
	//watershedProcess(thresholded_dst, thresholded_dst, 41);
	fillHoles(thresholded_dst);
	Mat thresholded_labels, thresholded_stats, thresholded_centroids, thresholded_doubleStats;
	int count = analyzeParticles(thresholded_dst, thresholded_labels, thresholded_stats, thresholded_centroids, thresholded_doubleStats, ParticleAnalyzer::FOUR_CONNECTED | ParticleAnalyzer::EXCLUDE_EDGE_PARTICLES, 20);

	int topStart = 150;
	int bottomStart = thresholded_dst.rows - 150;
	for (int i = 0; i < thresholded_dst.cols; i++) {
		for (int j = topStart; j < bottomStart; j++) {
			// You can now access the pixel value with cv::Vec3b
			//std::cout << "value: " << thresholded_dst.at<uchar>(i, j) << std::endl;
			cv::circle(thresholded_dst_new, Point(i, j), 0, 255, -1);
		}
	}
	bitwise_and(thresholded_dst, thresholded_dst_new, filteredImage);
	Mat filteredImage_labels, filteredImage_stats, filteredImage_centroids, filteredImage_doubleStats, roiMask;
	int count2 = analyzeParticles(filteredImage, filteredImage_labels, filteredImage_stats, filteredImage_centroids, filteredImage_doubleStats, ParticleAnalyzer::FOUR_CONNECTED | ParticleAnalyzer::EXCLUDE_EDGE_PARTICLES, 50);
	Mat filteredImageNew = filteredImage_labels > 0;

	//Mat test = Mat::zeros(src.size(), CV_8UC1);
	
	int value = 0;
	int whitePixelCounter = 0;
	int tempy = 0;
	int maxIntensity = 0;
	int highestIntensityColumnIndex = 0;
	int lastWhitePixelInLine = 0;
	int countHeight = 0;
	for (int y = 0; y < filteredImageNew.cols; y++)
	{
		for (int x = 0; x < filteredImageNew.rows; x++)
		{
			//cout << "point: " << Point(x, y) << endl;
			value = filteredImageNew.at<uchar>(x, y);
			if (value == 255){
				whitePixelCounter++;
				//cout << "whitePixelCounter: " << whitePixelCounter << endl;
			}
			if (whitePixelCounter > maxIntensity)
			{
				maxIntensity = whitePixelCounter;
				highestIntensityColumnIndex = y;
			}
			if (tempy != y)
			{
			    whitePixelCounter = 0;
			}
			tempy = y;
			//cout << "maxIntensity: " << maxIntensity << endl;
			//cout << "highestIntensityColumnIndex: " << highestIntensityColumnIndex << endl;
		}
	}

	cout << "highestIntensityColumnIndex: " << highestIntensityColumnIndex << endl;

	for (int y = highestIntensityColumnIndex; y < highestIntensityColumnIndex+1; y++)
	{
		for (int x = 0; x < filteredImageNew.rows; x++)
		{
			value = filteredImageNew.at<uchar>(x, y);
			if (value == 255) {
				lastWhitePixelInLine = x;
				//cout << "whitePixelCounter: " << whitePixelCounter << endl;
			}
			//			test.at<uchar>(x, y) = 255;
			//cout << "point: " << Point(x, y) << endl;
		}
	}

	//cout << "lastWhitePixelInLine: " << lastWhitePixelInLine << endl;

	int horizontalMarginValue = 50;
	int verticalMarginValue = 30;
	int leftStart = lastWhitePixelInLine - horizontalMarginValue;
	int rightStart = lastWhitePixelInLine + horizontalMarginValue;
	int rectWidth = horizontalMarginValue*2;
	int currentValue = 0;
	bool rectengleDetected = false;
	Mat seedlingArea;
	for (int x = leftStart; x < leftStart +1; x++)
	{
		for (int y = lastWhitePixelInLine- verticalMarginValue; y > 0; y--)
		{	
			countHeight++;

			cout << "point111: " << Point(x, y) << endl;

			currentValue = filteredImageNew.at<uchar>(y, x);
			//test.at<uchar>(y, x) = 255;
			if(currentValue == 255 && rectengleDetected==false)
			{
				cout << "currentValue: " << currentValue << endl;
				cv::Rect rectSeedling(x, y, rectWidth, countHeight);
				seedlingArea = filteredImageNew(rectSeedling);
				rectengleDetected = true;
				//rectangle(test, rect, Scalar(255, 255, 255));
			}
		}
	}
	Mat seedlingAreaEroded, seedlingAreaDilated;
	morphologyEx(seedlingArea, seedlingAreaEroded, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 3);
	morphologyEx(seedlingAreaEroded, seedlingAreaDilated, MORPH_DILATE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 3);
	

	//imwrite("seedlingArea.png", seedlingArea);

	//cout << "geldi: " << endl;

	//test(bounds).setTo(255);
	
	return result;
}


	//Rect bounds;
	//int maxHeightRoi = 0;
	//for (int i = 1; i < filteredImage_stats.rows; i++) {

	//	int currentHeightOfRoi = filteredImage_stats.at<int>(i, CC_STAT_HEIGHT);
	//	cout << "Currentheight: " << currentHeightOfRoi << endl;
	//	if (currentHeightOfRoi > maxHeightRoi)
	//	{
	//		maxHeightRoi = currentHeightOfRoi;
	//		bounds = getRect(filteredImage, filteredImage_stats, i);
	//		bounds.x = bounds.x - 100;
	//		bounds.y = bounds.y - 100;
	//		bounds.width = bounds.width + 150;
	//		bounds.height = bounds.height + 150;
	//		//roi = filteredImage(bounds) == i;
	//	}
	//}

	//int widthRrect = bounds.width;
	//int heightRect = bounds.height;
	//int x0 = bounds.x;
	//int y0 = bounds.y;
	//cout << "x0: " << x0 << endl;
	//cout << "y0: " << y0 << endl;
	//cout << "Width: " << widthRrect << endl;
	//cout << "Height: " << heightRect << endl;
	//int widthN = x0 + widthRrect;
	//int heightN = y0 + heightRect;
	//cout << "WidthN: " << widthN << endl;
	//cout << "HeightN: " << heightN << endl;
