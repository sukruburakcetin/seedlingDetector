
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
	int count2 = analyzeParticles(filteredImage, filteredImage_labels, filteredImage_stats, filteredImage_centroids, filteredImage_doubleStats, ParticleAnalyzer::FOUR_CONNECTED | ParticleAnalyzer::EXCLUDE_EDGE_PARTICLES, 200);
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

	//cout << "highestIntensityColumnIndex: " << highestIntensityColumnIndex << endl;

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

	//cout << "lastWhitePixelInLine: " << lastWhitePixelInLine << endl;

	int horizontalMarginValue = 50;
	int verticalMarginValue = 30;
	int leftStart = lastWhitePixelInLine - horizontalMarginValue;
	int rightStart = lastWhitePixelInLine + horizontalMarginValue;
	int bottomStartRect = lastWhitePixelInLine - verticalMarginValue;
	int rectWidth = horizontalMarginValue * 2;
	int currentValue = 0, whitePointsAtCurrentRow = 0, sumWhitePixels = 0;
	bool rectengleDetected = false;
	bool check = false;
	Mat seedlingArea, complateSeedlingArea, complateSeedlingAreaBlanked;
	for (int x = leftStart; x < leftStart + 1; x++)
	{
		for (int y = bottomStartRect; y > 0; y--)
		{
			countHeight++;

			//cout << "point111: " << Point(x, y) << endl;
			//cout << "bottomStartRect: " << bottomStartRect << endl;

			currentValue = filteredImageNew.at<uchar>(y, x);
			//test.at<uchar>(y, x) = 255;
			if (currentValue == 255 && rectengleDetected == false)
			{
				//cout << "currentValue: " << currentValue << endl;
				//cout << "x: " << x << endl;
				//cout << "y: " << y << endl;

				cv::Rect rectSeedling(x, y, rectWidth, countHeight);
				cv::Rect rectComplateSeedling(x, 0, rectWidth, bottomStartRect);

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

	for (int x = 0; x < seedlingAreaDilated.rows; x++)
	{
		for (int y = 0; y < seedlingAreaDilated.cols; y++)
		{
			value = seedlingAreaDilated.at<uchar>(x, y);
			tempNextPoint = seedlingAreaDilated.at<uchar>(x, y + 1);
			/*		cout << "point: " << Point(x, y) << endl;
					cout << "value: " << value << endl;
					cout << "tempNextPoint: " << tempNextPoint << endl;*/
					/*		cout << "y: " << y << endl;
							cout << "x: " << x << endl;
							cout << "tempyNew: " << tempyNew << endl;*/

			if (value == 255 && tempNextPoint == 255) {
				whitePointsSuccesfulStreakAtCurrentRow = whitePointsSuccesfulStreakAtCurrentRow + 1;
				/*seedlingAreaDilated.at<uchar>(x, y) = 0;*/
				//cout << "whitePointsSuccesfulStreakAtCurrentRow: " << whitePointsSuccesfulStreakAtCurrentRow << endl;

				sumWhitePixels = sumWhitePixels + 1;
				if (whitePointsSuccesfulStreakAtCurrentRow > 8 && rowIsCountedCheck == false)
				{
					rowIsCounted = rowIsCounted + 1;
					rowIsCountedCheck = true;
				}
			}
			else if (tempyNew != x)
			{
				whitePointsSuccesfulStreakAtCurrentRow = 0;
				rowIsCountedCheck = false;
			}
			tempyNew = x;
		}
	}
	//vector<Point> whitee_pixels;   // output, locations of non-zero pixels
	//cout << "rowIsCounted: " << rowIsCounted << endl;
	//findNonZero(seedlingAreaDilated, whitee_pixels);
	//cout << "Cloud all white pixels: " << whitee_pixels.size() << endl;

	int averageWhitePixels = (sumWhitePixels + rowIsCounted) / rowIsCounted;

	cout << "averageWhitePixels: " << averageWhitePixels << endl;

	int avrgWhiteCouplePxCountInCurrentRow = averageWhitePixels / 2;
	int dest = seedlingAreaDilated.cols;
	int leafStartPixelRowAmount = 0;
	for (int x = 0; x < seedlingAreaDilated.rows; x++)
	{
		for (int y = 0; y < dest; y++)
		{
			cout << "x: " << x << endl;
			cout << "y: " << y << endl;

			value = seedlingAreaDilated.at<uchar>(x, y);
			cout << "value: " << value << endl;

			tempNextPoint = seedlingAreaDilated.at<uchar>(x, y + 1);
			if (value == 255 && tempNextPoint == 255) {
				whitePointsSuccesfulStreakAtCurrentRowNew = whitePointsSuccesfulStreakAtCurrentRowNew + 1;
				/*seedlingAreaDilated.at<uchar>(x, y) = 0;*/
				//cout << "point: " << Point(x, y) << endl;
				//seedlingArea.at<uchar>(x, y) = 0;
				//cout << "whitePointsSuccesfulStreakAtCurrentRow: " << whitePointsSuccesfulStreakAtCurrentRowNew << endl;
				cout << "pointbefore: " << Point(x, y) << endl;
				cout << "whitePointsSuccesfulStreakAtCurrentRowNew: " << whitePointsSuccesfulStreakAtCurrentRowNew << endl;

			}
			else if (whitePointsSuccesfulStreakAtCurrentRowNew >= averageWhitePixels + 10 && check ==false)
			{
				for (int z = 0; z < whitePointsSuccesfulStreakAtCurrentRowNew +2; z++)
				{
					seedlingArea.at<uchar>(x, z) = 0;
				}
				check = true;
				leafStartPixelRowAmount = leafStartPixelRowAmount;
			}
			if (tempyNewSecond != x)
			{
				whitePointsSuccesfulStreakAtCurrentRowNew = 0;
				check = false;
			}
			tempyNewSecond = x;
			cout << "tempyNewSecondAfter: " << tempyNewSecond << endl;

		}
	}
	cout << "ok: " << endl;
	return result;
}
