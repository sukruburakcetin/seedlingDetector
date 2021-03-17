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
	// | ParticleAnalyzer::EXCLUDE_EDGE_PARTICLES
	int count = analyzeParticles(thresholded_dst, thresholded_labels, thresholded_stats, thresholded_centroids, thresholded_doubleStats, ParticleAnalyzer::FOUR_CONNECTED, 20);

	thresholded_dst = thresholded_labels > 0;
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
	int count2 = analyzeParticles(filteredImage, filteredImage_labels, filteredImage_stats, filteredImage_centroids, filteredImage_doubleStats, ParticleAnalyzer::FOUR_CONNECTED | ParticleAnalyzer::EXCLUDE_EDGE_PARTICLES, 400);
	Mat filteredImageNew = filteredImage_labels > 0;

	//Mat test = Mat::zeros(src.size(), CV_8UC1);

	int value = 0;
	int whitePixelCounter = 0;
	int tempy = 0, tempyNew = 0, tempyNewSecond = 0;
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
			if (value == 255) {
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
	/*-----highestIntensityColumnIndex == yukar�dan a�a��ya en yo�un column(vertical line)-----*/
	cout << "highestIntensityColumnIndex: " << highestIntensityColumnIndex << endl;

	for (int y = highestIntensityColumnIndex; y < highestIntensityColumnIndex + 1; y++)
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

	/*------lastWhitePixelInLine == en yo�un column(vertical line)daki taban obje pikseli------*/
	cout << "lastWhitePixelInLine: " << lastWhitePixelInLine << endl;

	//int rightStart = lastWhitePixelInLine + horizontalMarginValue;

	int horizontalMarginValue = 30; // margin that is direct the point left 
	int verticalMarginValue = 30; // margin that is direct the point top 
	int leftStart = highestIntensityColumnIndex - horizontalMarginValue;
	int bottomStartRect = lastWhitePixelInLine - verticalMarginValue; // from bottom highintense row to row0
	int rectWidth = horizontalMarginValue * 2;
	int rectWidthAlt = rectWidth * 8;
	int currentValue = 0, whitePointsAtCurrentRow = 0, sumWhitePixels = 0;
	bool rectengleDetected = false;
	bool check = false;
	Mat seedlingArea, complateSeedlingArea, complateSeedlingAreaBlanked;
	for (int y = leftStart; y < leftStart + 1; y++)
	{
		for (int x = bottomStartRect; x > 0; x--)
		{
			countHeight++;

			//cout << "currenPoint: " << Point(x, y) << endl;
			//cout << "bottomStartRect: " << bottomStartRect << endl;
			//filteredImageNew.at<uchar>(x, y) = 255;
			//test.at<uchar>(y, x) = 255;

			currentValue = filteredImageNew.at<uchar>(x, y);
			if (currentValue == 255 && rectengleDetected == false)
			{
				//cout << "currentValue: " << currentValue << endl;
				//cout << "x: " << x << endl;
				//cout << "y: " << y << endl;
				cv::Rect rectSeedling(y, x, rectWidth, countHeight);
				cv::Rect rectComplateSeedling(y-200, 0, rectWidthAlt, bottomStartRect);

				seedlingArea = filteredImageNew(rectSeedling);
				complateSeedlingArea = filteredImageNew(rectComplateSeedling);
				rectengleDetected = true;
				//rectangle(test, rect, Scalar(255, 255, 255));
			}
		}
	}
	Mat seedlingAreaEroded, seedlingAreaDilated;
	morphologyEx(seedlingArea, seedlingAreaEroded, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 3);
	morphologyEx(seedlingAreaEroded, seedlingAreaDilated, MORPH_DILATE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 3);

	int startLeft = seedlingAreaDilated.cols;
	int startAlternate = seedlingAreaDilated.rows - 1;
	int tempNextPoint = 0, rowIsCounted = 0;
	bool rowIsCountedCheck = false;
	int whitePointsSuccesfulStreakAtCurrentRow = 0, whitePointsSuccesfulStreakAtCurrentRowNew = 0, whitePixelsCurrentRow = 0, reminder = 0;
	//cout << "startLeft: " << startLeft << endl;
	//cout << "startAlternate: " << startAlternate << endl;


	/*--------this block calculates average white pixel value by looking down to up and left to right by 10 pixel--------*/
	int startPoint = seedlingAreaDilated.rows - 1;
	int endPoint = startPoint /2;
	//cout << "startPoint: " << startPoint << endl;
	for (int x = startPoint; x > endPoint; x--)
	{
		for (int y = 0; y < seedlingAreaDilated.cols; y++)
		{
			//cout << "point: " << Point(x, y) << endl;
			//seedlingAreaDilated.at<uchar>(x, y) = 255;
			value = seedlingAreaDilated.at<uchar>(x, y);
			/*cout << "value: " << value << endl;*/
			tempNextPoint = seedlingAreaDilated.at<uchar>(x, y + 1);

			if (value == 255 && tempNextPoint == 255) {
				whitePointsSuccesfulStreakAtCurrentRow = whitePointsSuccesfulStreakAtCurrentRow + 1;
				/*seedlingAreaDilated.at<uchar>(x, y) = 0;*/
				//cout << "whitePointsSuccesfulStreakAtCurrentRow: " << whitePointsSuccesfulStreakAtCurrentRow << endl;

				sumWhitePixels = sumWhitePixels + 1;

			}
			else if (whitePointsSuccesfulStreakAtCurrentRow > 8 && rowIsCountedCheck == false)
			{
				rowIsCounted = rowIsCounted + 1;
				rowIsCountedCheck = true;
			}
			else if (tempyNew != x)
			{
				whitePointsSuccesfulStreakAtCurrentRow = 0;
				rowIsCountedCheck = false;
				//cout << "sumWhitePixels: " << sumWhitePixels << endl;
			}
			tempyNew = x;
		}
	}

	int averageWhitePixels = (sumWhitePixels + rowIsCounted) / rowIsCounted;

	//cout << "averageWhitePixels: " << averageWhitePixels << endl;

	int avrgWhiteCouplePxCountInCurrentRow = averageWhitePixels / 2;
	int leafStartPixelRowAmountInBody = 0, bodyToLeafMargin = 10;
	for (int x = 0; x < seedlingAreaDilated.rows; x++)
	{
		for (int y = 0; y < seedlingAreaDilated.cols; y++)
		{
			//cout << "x: " << x << endl;
			//cout << "y: " << y << endl;

			value = seedlingAreaDilated.at<uchar>(x, y);
			//cout << "value: " << value << endl;

			tempNextPoint = seedlingAreaDilated.at<uchar>(x, y + 1);
			if (value == 255 && tempNextPoint == 255) {
				whitePointsSuccesfulStreakAtCurrentRowNew = whitePointsSuccesfulStreakAtCurrentRowNew + 1;
				/*seedlingAreaDilated.at<uchar>(x, y) = 0;*/
				//cout << "point: " << Point(x, y) << endl;
				//seedlingArea.at<uchar>(x, y) = 0;
				//cout << "whitePointsSuccesfulStreakAtCurrentRow: " << whitePointsSuccesfulStreakAtCurrentRowNew << endl;

				//cout << "pointbefore: " << Point(x, y) << endl;
				//cout << "whitePointsSuccesfulStreakAtCurrentRowNew: " << whitePointsSuccesfulStreakAtCurrentRowNew << endl;

			}
			else if (whitePointsSuccesfulStreakAtCurrentRowNew >= averageWhitePixels + bodyToLeafMargin && check == false)
			{
				//cout << "whitePointsSuccesfulStreakAtCurrentRow: " << whitePointsSuccesfulStreakAtCurrentRowNew << endl;

				for (int z = 0; z < seedlingArea.rows; z++)
				{
					seedlingArea.at<uchar>(x, z) = 0;
				}
				check = true;
				leafStartPixelRowAmountInBody = leafStartPixelRowAmountInBody + 1;
			}
			else if (tempyNewSecond != x)
			{
				whitePointsSuccesfulStreakAtCurrentRowNew = 0;
				check = false;
			}
			//cout << "tempyNewSecondBefore: " << tempyNewSecond << endl;
			tempyNewSecond = x;
			//cout << "tempyNewSecondAfter: " << tempyNewSecond << endl;

		}
	}

	cout << "leafStartPixelRowAmountInBody: " << leafStartPixelRowAmountInBody << endl;
	int seedlingHeight = (seedlingArea.rows + verticalMarginValue) - leafStartPixelRowAmountInBody;
	cout << "seedlingHeight: " << seedlingHeight << " pixel." << endl;

	return result;
}

/* ---------------------------------------------CODE ARCHIVE--------------------------------------------*/

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



/*code that checks white pixels left to right and up to down*/
	//for (int x = 0; x < seedlingAreaDilated.rows; x++)
	//{
	//	for (int y = 0; y < seedlingAreaDilated.cols; y++)
	//	{
	//		cout << "y: " << y << endl;
	//		cout << "x: " << x << endl;
	//		value = seedlingAreaDilated.at<uchar>(x, y);
	//		cout << "value: " << value << endl;
	//		tempNextPoint = seedlingAreaDilated.at<uchar>(x, y + 1);
	//		/*		cout << "point: " << Point(x, y) << endl;
	//				cout << "value: " << value << endl;
	//				cout << "tempNextPoint: " << tempNextPoint << endl;*/
	//				/*		cout << "y: " << y << endl;
	//						cout << "x: " << x << endl;
	//						cout << "tempyNew: " << tempyNew << endl;*/

	//		if (value == 255 && tempNextPoint == 255) {
	//			whitePointsSuccesfulStreakAtCurrentRow = whitePointsSuccesfulStreakAtCurrentRow + 1;
	//			/*seedlingAreaDilated.at<uchar>(x, y) = 0;*/
	//			//cout << "whitePointsSuccesfulStreakAtCurrentRow: " << whitePointsSuccesfulStreakAtCurrentRow << endl;

	//			sumWhitePixels = sumWhitePixels + 1;

	//		}
	//		else if (whitePointsSuccesfulStreakAtCurrentRow > 8 && rowIsCountedCheck == false)
	//		{
	//			rowIsCounted = rowIsCounted + 1;
	//			rowIsCountedCheck = true;
	//		}
	//		else if (tempyNew != x)
	//		{
	//			whitePointsSuccesfulStreakAtCurrentRow = 0;
	//			rowIsCountedCheck = false;
	//			cout << "sumWhitePixels: " <<  sumWhitePixels << endl;
	//		}
	//		tempyNew = x;
	//	}
	//}
	//vector<Point> whitee_pixels;   // output, locations of non-zero pixels
	//cout << "rowIsCounted: " << rowIsCounted << endl;
	//findNonZero(seedlingAreaDilated, whitee_pixels);
	//cout << "Cloud all white pixels: " << whitee_pixels.size() << endl;