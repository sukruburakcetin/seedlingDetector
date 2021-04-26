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



void determineThresholdSeedlingToLeaf(int y, int x, int leftStart2, int bottomStartRect2, int& heightSeedling,
	int currentValue2,
	bool& rowCheckIsDone, bool finalWhitePixelRight2, int sumWhitePixelToTheRight2,
	Mat filteredImageNew, Mat filteredImageNew_3D, int currentValueRight,
	int currentValueLeft, bool finalWhitePixelLeft2, int sumWhitePixelToTheLeft2,
	int& rowThicknessWhenCollapsed, int& currentHeightAtWhitePoint,
	int seedlingThickness);

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
	int topStart = 150, bottom_margin = 150;
	int bottomStart = thresholded_dst.rows - bottom_margin;
	for (int i = 0; i < thresholded_dst.cols; i++) {
		for (int j = topStart; j < bottomStart; j++) {
			// You can now access the pixel value with cv::Vec3b
			//std::cout << "value: " << thresholded_dst.at<uchar>(i, j) << std::endl;
			cv::circle(thresholded_dst_new, Point(i, j), 0, 255, -1);
		}
	}
	bitwise_and(thresholded_dst, thresholded_dst_new, filteredImage);
	Mat filteredImage_labels, filteredImage_stats, filteredImage_centroids, filteredImage_doubleStats, roiMask;
	//watershedProcess(filteredImage, filteredImageNew, 41);

	int count2 = analyzeParticles(filteredImage, filteredImage_labels, filteredImage_stats, filteredImage_centroids, filteredImage_doubleStats, ParticleAnalyzer::FOUR_CONNECTED | ParticleAnalyzer::EXCLUDE_EDGE_PARTICLES, 10000);
	// 
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
	cout << "highestIntensityColumnIndex(y): " << highestIntensityColumnIndex << endl;

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
	cout << "lastWhitePixelInLine(x): " << lastWhitePixelInLine << endl;


	Mat filteredImageNew_3D = Mat::zeros(filteredImageNew.size(), CV_8UC3);
	cvtColor(filteredImageNew, filteredImageNew_3D, COLOR_GRAY2RGB);
	circle(filteredImageNew_3D, Point(highestIntensityColumnIndex, lastWhitePixelInLine), 0, Scalar(0, 255, 0), -1);


	/*---------------calculates seedlingThickness by searching left and right whitePixels---------------------*/
	int currentValueLeft = 0, currentValueRight = 0, sumWhitePixelToTheLeft = -1, sumWhitePixelToTheRight = 0, sumWhitePixelToTheLeft2 = -1, sumWhitePixelToTheRight2 = 0, artifactMarginValue = 50;
	int filteredPointOfHICI = lastWhitePixelInLine - artifactMarginValue;
	bool finalWhitePixelLeft = false, finalWhitePixelRight = false, finalWhitePixelLeft2 = false, finalWhitePixelRight2 = false, isLeftOriented = false, isRightOriented = false;
	//cout << "filteredImageNew.cols: " << filteredImageNew.cols << endl;

	for (int x = filteredPointOfHICI; x < filteredPointOfHICI + 1; x++)
	{
		for (int y = highestIntensityColumnIndex; y > 0; y--)
		{
			//cout << "pointLeft: " << Point(x, y) << endl;
			//filteredImageNew.at<uchar>(x, y) = 0;
			currentValueLeft = filteredImageNew.at<uchar>(x, y);
			if (currentValueLeft == 255 && finalWhitePixelLeft == false) {
				sumWhitePixelToTheLeft = sumWhitePixelToTheLeft + 1;
			}
			else if (currentValueLeft == 0)
			{
				finalWhitePixelLeft = true;
			}
		}
	}

	//cout << "sumWhitePixelToTheLeft: " << sumWhitePixelToTheLeft << endl;
	for (int x = filteredPointOfHICI; x < filteredPointOfHICI + 1; x++)
	{
		for (int y = highestIntensityColumnIndex; y < filteredImageNew.cols; y++)
		{
			//cout << "pointRight: " << Point(x, y) << endl;
			//filteredImageNew.at<uchar>(x, y) = 0;
			currentValueRight = filteredImageNew.at<uchar>(x, y);
			if (currentValueRight == 255 && finalWhitePixelRight == false) {
				sumWhitePixelToTheRight = sumWhitePixelToTheRight + 1;
			}
			else if (currentValueRight == 0)
			{
				finalWhitePixelRight = true;
			}

		}
	}
	//cout << "sumWhitePixelToTheRight: " << sumWhitePixelToTheRight << endl;

	int seedlingThickness = sumWhitePixelToTheLeft + sumWhitePixelToTheRight;
	cout << "seedlingThickness(OguzhanHoca): " << seedlingThickness << endl;




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
	currentValue = filteredImageNew.at<uchar>(bottomStartRect, leftStart);

	/*test variables start-----------------------------------------------------*/
	int currentValue2 = 0;
	int horizontalMarginValue2 = 2; // margin that is direct the point left 
	int verticalMarginValue2 = 30; // margin that is direct the point top 

	int rectWidth2 = horizontalMarginValue2 * 2;
	int rectWidthAlt2 = rectWidth2 * 8;
	int rowThicknessWhenCollapsed = 0;
	int epsilon = 5;
	int startSeedlingPoint = 0;


	bool rowCheckIsDone = false;
	int heightSeedlingSum = 0, heightSeedling = 0, nextSeedlingStartPoint = 0, currentHeightAtWhitePoint = 0;

	int bottomStartRect2 = lastWhitePixelInLine - verticalMarginValue2;

	if (sumWhitePixelToTheLeft < sumWhitePixelToTheRight)
	{
		isLeftOriented = true;
		startSeedlingPoint = highestIntensityColumnIndex - horizontalMarginValue2;

	}

	else if (sumWhitePixelToTheRight < sumWhitePixelToTheLeft)
	{
		isRightOriented = true;
		startSeedlingPoint = highestIntensityColumnIndex + horizontalMarginValue2;
	}

	currentValue2 = filteredImageNew.at<uchar>(bottomStartRect2, startSeedlingPoint);
	circle(filteredImageNew_3D, Point(startSeedlingPoint, bottomStartRect2), 0, Scalar(255, 0, 0), -1);


	/*test variables end*/

	/*test body start */


	if (currentValue2 == 255 && isLeftOriented == true) //checks the entry point white or black, it must be black
	{
		startSeedlingPoint = startSeedlingPoint - horizontalMarginValue2;
		rectWidth2 = rectWidth2 * 2;
	}

	if (currentValue2 == 255 && isRightOriented == true) //checks the entry point white or black, it must be black
	{
		startSeedlingPoint = startSeedlingPoint + horizontalMarginValue2;
		rectWidth2 = rectWidth2 * 2;
	}

	while (rowThicknessWhenCollapsed <= (seedlingThickness + epsilon)) {
		for (int y = startSeedlingPoint; y < startSeedlingPoint + 1; y++)
		{
			for (int x = bottomStartRect2; x > 0; x--)
			{
				heightSeedlingSum++;
				determineThresholdSeedlingToLeaf(y, x, startSeedlingPoint, bottomStartRect2, heightSeedling, currentValue2, rowCheckIsDone,
					finalWhitePixelRight2, sumWhitePixelToTheRight2, filteredImageNew, filteredImageNew_3D,
					currentValueRight, currentValueLeft, finalWhitePixelLeft2, sumWhitePixelToTheLeft2,
					rowThicknessWhenCollapsed, currentHeightAtWhitePoint, seedlingThickness);

				if (rowCheckIsDone == true)
				{
					bottomStartRect2 = bottomStartRect2 - currentHeightAtWhitePoint;
					if (isLeftOriented == true) {
						startSeedlingPoint = startSeedlingPoint - horizontalMarginValue2;
						y = startSeedlingPoint;
					}
					else if(isRightOriented == true)
					{
						startSeedlingPoint = startSeedlingPoint + horizontalMarginValue2;
						y = startSeedlingPoint;
					}
					break;
				}
			}
		}
	}

	//cout << "value: " << highestIntensityColumnIndex - leftStart << endl;
	cv::Rect rectSeedling(leftStart + horizontalMarginValue2, bottomStartRect2, (highestIntensityColumnIndex - leftStart) + seedlingThickness, heightSeedlingSum);
	//cv::Rect rectComplateSeedling(y - 200, 0, rectWidthAlt, bottomStartRect);

	seedlingArea = filteredImageNew(rectSeedling);


	cout << "heightSeedlingSum: " << heightSeedlingSum << endl;
	/*test body end */
	//if (currentValue == 255)
	//{
	//	leftStart = leftStart - horizontalMarginValue;
	//	rectWidth = rectWidth * 2;
	//}

	//for (int y = leftStart; y < leftStart + 1; y++)
	//{
	//	for (int x = bottomStartRect; x > 0; x--)
	//	{
	//		countHeight++;
	//		//cout << "currenPoint: " << Point(x, y) << endl;
	//		//cout << "bottomStartRect: " << bottomStartRect << endl;
	//		//filteredImageNew.at<uchar>(x, y) = 255;
	//		//test.at<uchar>(y, x) = 255;
	//		currentValue = filteredImageNew.at<uchar>(x, y);
	//		if (currentValue == 255 && rectengleDetected == false)
	//		{
	//			//cout << "currentValue: " << currentValue << endl;
	//			//cout << "x: " << x << endl;
	//			//cout << "y: " << y << endl;

	//			cv::Rect rectSeedling(y, x, rectWidth, countHeight);
	//			cv::Rect rectComplateSeedling(y - 200, 0, rectWidthAlt, bottomStartRect);

	//			seedlingArea = filteredImageNew(rectSeedling);
	//			complateSeedlingArea = filteredImageNew(rectComplateSeedling);
	//			rectengleDetected = true;
	//			//rectangle(test, rect, Scalar(255, 255, 255));
	//		}
	//	}
	//}
	Mat seedlingAreaEroded, seedlingAreaDilated;
	morphologyEx(seedlingArea, seedlingAreaEroded, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 3);
	morphologyEx(seedlingAreaEroded, seedlingAreaDilated, MORPH_DILATE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 3);

	int startLeft = seedlingAreaDilated.cols;
	int startAlternate = seedlingAreaDilated.rows - 1;
	int tempNextPoint = 0, rowIsCounted = 0, averageWhitePixels = 0;
	bool rowIsCountedCheck = false;
	int whitePointsSuccesfulStreakAtCurrentRow = 0, whitePointsSuccesfulStreakAtCurrentRowNew = 0, whitePixelsCurrentRow = 0, reminder = 0;
	//cout << "startLeft: " << startLeft << endl;
	//cout << "startAlternate: " << startAlternate << endl;

	/*---------------calculate the center of the region starts-----------------------*/

	Moments m = moments(seedlingAreaDilated, false);
	Point p1(m.m10 / m.m00, m.m01 / m.m00);
	//cout << Mat(p1) << endl;
	//circle(seedlingAreaDilated, p1, 0, Scalar(0, 0, 0), -1);

	/*---------------calculate the center of the region ends-----------------------*/

	for (int x = p1.y; x < p1.y + 1; x++)
	{
		for (int y = 0; y < seedlingAreaDilated.cols; y++)
		{
			value = seedlingAreaDilated.at<uchar>(x, y);
			if (value == 255) {
				/*		cout << "point: " << Point(x, y) << endl;
						seedlingArea.at<uchar>(x, y) = 0;*/
				averageWhitePixels = averageWhitePixels + 1;
			}
		}
	}
	cout << "averageWhitePixels_CenterMode: " << averageWhitePixels << endl;

#pragma region previous build that calculates average whitePixel on seedling body
	///*---------------------------------*/
///*--------this block calculates average white pixel value by looking down to up and left to right by 10 pixel--------*/
//int startPoint = seedlingAreaDilated.rows - 1;
//int endPoint = startPoint / 2;
////cout << "startPoint: " << startPoint << endl;
//for (int x = startPoint; x > endPoint; x--)
//{
//	for (int y = 0; y < seedlingAreaDilated.cols; y++)
//	{
//		//cout << "point: " << Point(x, y) << endl;
//		//seedlingAreaDilated.at<uchar>(x, y) = 255;
//		value = seedlingAreaDilated.at<uchar>(x, y);
//		/*cout << "value: " << value << endl;*/
//		tempNextPoint = seedlingAreaDilated.at<uchar>(x, y + 1);

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
//			//cout << "sumWhitePixels: " << sumWhitePixels << endl;
//		}
//		tempyNew = x;
//	}
//}

//int averageWhitePixels = (sumWhitePixels + rowIsCounted) / rowIsCounted;  
#pragma endregion

	/*This blocks determine leaf artifact on the seedling*/
	int leafStartPixelRowAmountInBody = 0, bodyToLeafMargin = 5;
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
					//floodFill(filteredImageNew, Point2d(x, z), 127);
				}
				check = true;
				leafStartPixelRowAmountInBody = leafStartPixelRowAmountInBody + 1;
			}
			else if (tempyNewSecond != x)
			{
				whitePointsSuccesfulStreakAtCurrentRowNew = 0;
				check = false;
			}
			tempyNewSecond = x;
		}
	}
	//cout << "leafStartPixelRowAmountInBody: " << leafStartPixelRowAmountInBody << endl;
	//int seedlingHeight = (seedlingArea.rows + verticalMarginValue) - leafStartPixelRowAmountInBody;
	int seedlingHeight = heightSeedlingSum + verticalMarginValue;
	cout << "seedlingHeight: " << seedlingHeight << " pixel." << endl;

	return result;
}

void determineThresholdSeedlingToLeaf(int y, int x, int leftStart2, int bottomStartRect2, int& heightSeedling,
	int currentValue2,
	bool& rowCheckIsDone, bool finalWhitePixelRight2, int sumWhitePixelToTheRight2,
	Mat filteredImageNew, Mat filteredImageNew_3D, int currentValueRight,
	int currentValueLeft, bool finalWhitePixelLeft2, int sumWhitePixelToTheLeft2,
	int& rowThicknessWhenCollapsed, int& currentHeightAtWhitePoint,
	int seedlingThickness)
{

	//cout << "Point(x, y): " << Point(x, y) << endl;
	heightSeedling++;
	rowCheckIsDone = false;
	//cout << "heightSeedling: " << heightSeedling << endl;

	currentValue2 = filteredImageNew.at<uchar>(x, y);

	if (rowCheckIsDone == false) { // colors the test area to let developer track the code
		circle(filteredImageNew_3D, Point(y, x), 0, Scalar(255, 0, 0), -1);
	}

	if (currentValue2 == 255 && rowCheckIsDone == false)
	{
		//for right side of the center, calculates white pixels
		for (int j = x; j < x + 1; j++)
		{
			for (int k = y; k < filteredImageNew.cols; k++)
			{
				//cout << "pointCollapsed: " << Point(j, k) << endl;
				//filteredImageNew.at<uchar>(x, y) = 0;
				currentValueRight = filteredImageNew.at<uchar>(j, k);
				if (currentValueRight == 255 && finalWhitePixelRight2 == false) {
					//cout << "pointRightFirst: " << Point(j, k) << endl;
					sumWhitePixelToTheRight2 = sumWhitePixelToTheRight2 + 1;
				}
				else if (currentValueRight == 0)
				{
					finalWhitePixelRight2 = true;
				}
			}
		}
		for (int l = x; l < x + 1; l++)
		{
			for (int m = y; m > -1; m--)
			{
				//cout << "pointCollapsed: " << Point(l, m) << endl;
				//circle(filteredImageNew_3D, Point(l, m), 0, Scalar(0, 255, 0), -1);

				//cout << "pointLeft: " << Point(x, y) << endl;
				//filteredImageNew.at<uchar>(x, y) = 0;
				currentValueLeft = filteredImageNew.at<uchar>(l, m);
				if (currentValueLeft == 255 && finalWhitePixelLeft2 == false) {
					sumWhitePixelToTheLeft2 = sumWhitePixelToTheLeft2 + 1;
				}
				else if (currentValueLeft == 0)
				{
					finalWhitePixelLeft2 = true;
				}
			}
		}
		rowThicknessWhenCollapsed = sumWhitePixelToTheLeft2 + sumWhitePixelToTheRight2;
		currentHeightAtWhitePoint = heightSeedling;
		rowCheckIsDone = true;
		heightSeedling = 0;
	}
}
