#define _USE_MATH_DEFINES
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include "seedlingDetector.hpp"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "AutoThreshold.hpp"
#include "ParticleAnalyzer.hpp"
#include "RankFilters.hpp"
#include "FillHoles.hpp"

using namespace std;
using namespace cv;


auto determineThresholdSeedlingToLeaf(int y, int x, int& heightSeedling,
	int currentPixelValueAtCoordinate,
	bool& rowCheckIsDone, bool finalWhitePixelRight2, int sumWhitePixelToTheRight2,
	Mat filteredImageNew, Mat filteredImageNew_3D, int currentValueRight,
	int currentValueLeft, bool finalWhitePixelLeft2, int sumWhitePixelToTheLeft2,
	int& rowThicknessWhenCollapsed, int& currentHeightAtWhitePoint) -> void;

auto determineLeafStartToLeafPeakPoint(int& x, int& y, int currentPixelValueAtCoordinate,
	Mat filteredImageNew,
	Mat filteredImageNew_3D, Mat filteredImageNew_clone,
	bool isLeftOriented, bool isRightOriented, const Point2i& pointFirst, bool stageSecond,
	vector<float>& peakDistances, int bottomStartPoint, int leftStartPixelPoint, bool& finalPoint,
	float& leafLength, int& newX) -> void;

auto checkComputeDirection(bool& isLeftOriented, bool& isRightOriented, int x, int y, int currentPixelValueAtCoordinate, Mat filteredImageNew, bool& directionUpdated) -> void;

float realBodyThickness = 0.69;
float realBodyHeight = 30.00;
float realLeafLength = 20.00;

seedlingDetectorResult seedlingDetector(cv::Mat& src, cv::Mat& dst, const seedlingDetectorPreferences& prefs)
{
	seedlingDetectorResult result;

	
//#pragma region grid analysis for deeplearning start
//	Mat testpic = imread("C:/8.png");
//
//	int width = testpic.cols;
//	int height = testpic.rows;
//	int GRID_SIZE = 256, counter = -1;
//	Mat previousGrid, wholeImageConcat, verticalPartImage1, wholeImageVConcat;
//	vector<Rect> mCells;
//	vector<Mat> vCells;
//	for (int y = 0; y <= height - GRID_SIZE; y += GRID_SIZE) {
//		for (int x = 0; x <= width - GRID_SIZE; x += GRID_SIZE) {
//			int k = x * y + x;
//			Rect grid_rect(x, y, GRID_SIZE, GRID_SIZE);
//			cout << grid_rect << endl;
//			mCells.push_back(grid_rect);
//			//rectangle(testpic, grid_rect, Scalar(0, 255, 0), 0);
//			counter++;
//			cout << "x: " << x << endl;
//			cout << "y: " << y << endl;
//
//			if (x == 0) {
//				previousGrid = testpic(grid_rect);
//			}
//			else if (x != 0) {
//				hconcat(previousGrid, testpic(grid_rect), wholeImageConcat);
//				previousGrid = wholeImageConcat;
//				cout << "wholeImage:" << wholeImageConcat.cols << endl;
//				if (wholeImageConcat.cols == testpic.cols)
//				{
//					vCells.push_back(wholeImageConcat);
//				}
//			}
//			//imwrite("grid" + to_string(counter) + ".png", testpic(grid_rect));
//			//waitKey();
//		}
//	}
//	vconcat(vCells[0], vCells[1], wholeImageVConcat);
//	vconcat(wholeImageVConcat, vCells[2], wholeImageVConcat);
//
//
//	//imwrite("emreSahin.png", wholeImageVConcat);  
//#pragma endregion grid analysis for deeplearning end

	
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
	autoThreshold(thresholded_dst, ThresholdMethod::Otsu);
	thresholded_dst = ~thresholded_dst;
	//watershedProcess(thresholded_dst, thresholded_dst, 41);
	fillHoles(thresholded_dst);
	Mat thresholded_labels, thresholded_stats, thresholded_centroids, thresholded_doubleStats;
	// | ParticleAnalyzer::EXCLUDE_EDGE_PARTICLES
	int count = analyzeParticles(thresholded_dst, thresholded_labels, thresholded_stats, thresholded_centroids, thresholded_doubleStats, ParticleAnalyzer::FOUR_CONNECTED, 20);

	thresholded_dst = thresholded_labels > 0;
	//crop
	//int topStart = 150, bottom_margin = 150;
	int topStart = 1, bottom_margin = 70; //70 value for new image template

	int bottomStart = thresholded_dst.rows - bottom_margin;
	for (int i = 0; i < thresholded_dst.cols; i++) {
		for (int j = topStart; j < bottomStart; j++) {
			// You can now access the pixel value with cv::Vec3b
			//std::cout << "value: " << thresholded_dst.at<uchar>(i, j) << std::endl;
			cv::circle(thresholded_dst_new, Point(i, j), 0, 255, -1);
		}
	}
	bitwise_and(thresholded_dst, thresholded_dst_new, filteredImage);
	Mat filteredImage_labels, filteredImage_stats, filteredImage_centroids, filteredImage_doubleStats, roiMask, filteredImageNewEroded, filteredImageNewDilated;
	//watershedProcess(filteredImage, filteredImageNew, 41);

	analyzeParticles(filteredImage, filteredImage_labels, filteredImage_stats, filteredImage_centroids, filteredImage_doubleStats, ParticleAnalyzer::FOUR_CONNECTED | ParticleAnalyzer::EXCLUDE_EDGE_PARTICLES, 10000);

	Mat filteredImageNew = filteredImage_labels > 0;
	Mat filteredImageLabelsClone = filteredImageNew.clone();

	morphologyEx(filteredImageNew, filteredImageNewEroded, MORPH_ERODE, getStructuringElement(CV_SHAPE_RECT, Size(3, 3)), Point(-1, -1), 2);
	morphologyEx(filteredImageNewEroded, filteredImageNewDilated, MORPH_DILATE, getStructuringElement(CV_SHAPE_RECT, Size(3, 3)), Point(-1, -1), 2);

	filteredImageNew = filteredImageNewDilated.clone();

	//Mat test = Mat::zeros(src.size(), CV_8UC1);

	/*****************testing pic for artifact cleared env start***************************/

	//filteredImageNew = imread("C:/Users/HTG_SOFTWARE/Desktop/asd.png");
	//cvtColor(filteredImageNew, filteredImageNew, COLOR_RGB2GRAY);

	/*****************testing pic for artifact cleared env end***************************/

	int value = 0;
	int whitePixelCounter = 0;
	int tempy = 0, tempyNewSecond = 0;
	int maxIntensity = 0;
	int highestIntensityColumnIndex = 0;
	int lastWhitePixelInHighestIntensityLine = 0;

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
				lastWhitePixelInHighestIntensityLine = x;
				//cout << "whitePixelCounter: " << whitePixelCounter << endl;
			}
			//			test.at<uchar>(x, y) = 255;
			//cout << "point: " << Point(x, y) << endl;
		}
	}

	/*------lastWhitePixelInLine == en yo�un column(vertical line)daki taban obje pikseli------*/
	cout << "lastWhitePixelInLine(x): " << lastWhitePixelInHighestIntensityLine << endl;


	Mat filteredImageNew_3D = Mat::zeros(filteredImageNew.size(), CV_8UC3);
	Mat filteredImageNewFilled = Mat::zeros(filteredImageNew.size(), CV_8UC3);
	Mat filteredImageNew_clone = Mat::zeros(filteredImageNew.size(), CV_8UC1);

	cvtColor(filteredImageNew, filteredImageNew_3D, COLOR_GRAY2RGB);
	cvtColor(filteredImageNew, filteredImageNewFilled, COLOR_GRAY2RGB);

	// dye most intensive column's bottom point
	circle(filteredImageNew_3D, Point(highestIntensityColumnIndex, lastWhitePixelInHighestIntensityLine), 0, Scalar(0, 255, 0), -1);


	/*---------------calculates seedlingThickness by searching left and right whitePixels---------------------*/
	int currentValueLeft = 0, currentValueRight = 0, sumWhitePixelToTheLeft = -1, sumWhitePixelToTheRight = 0, sumWhitePixelToTheLeft2 = -1, sumWhitePixelToTheRight2 = 0;

	int horizontalMarginValueForBottomStart = 2; // margin that is direct the point left 
	int verticalMarginValueForBottomStartPoint = 30; // margin that is direct the point top 

	// move the point upward a little bit to avoid artifacts that are mostly located
	// bottomStartPoint(filteredPointOfHICI) = filtered Point of Highest Intensity Column Index
	int bottomStartPoint = lastWhitePixelInHighestIntensityLine - verticalMarginValueForBottomStartPoint;
	circle(filteredImageNew_3D, Point(highestIntensityColumnIndex, bottomStartPoint), 0, Scalar(255, 0, 0), -1);

	bool finalWhitePixelLeft = false, finalWhitePixelRight = false, finalWhitePixelLeft2 = false, finalWhitePixelRight2 = false, isLeftOriented = false, isRightOriented = false;

	for (int x = bottomStartPoint; x < bottomStartPoint + 1; x++)
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
	for (int x = bottomStartPoint; x < bottomStartPoint + 1; x++)
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

	/*calculates body thickness by adding left pixel count and right pixel count of the determined intensive point*/
	float bodyThickness = sumWhitePixelToTheLeft + sumWhitePixelToTheRight;
	//cout << "bodyThickness: " << bodyThickness << endl;


	int horizontalMarginValue = 30; // margin that is direct the point left 
	int leftStart = highestIntensityColumnIndex - horizontalMarginValue;


	bool check = false;
	Mat seedlingArea, complateSeedlingArea, complateSeedlingAreaBlanked;


	int definedRectWidth = horizontalMarginValueForBottomStart * 2;
	int rowThicknessWhenCollapsed = 0;
	int epsilon = 5;

	bool rowCheckIsDone = false;
	int heightSeedlingSum = 0, heightSeedling = 0, currentHeightAtWhitePoint = 0;

	int startSeedlingPoint = highestIntensityColumnIndex;

	int currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(bottomStartPoint, highestIntensityColumnIndex);

	if (sumWhitePixelToTheLeft < sumWhitePixelToTheRight)
	{
		isLeftOriented = true;
	}

	else if (sumWhitePixelToTheRight < sumWhitePixelToTheLeft)
	{
		isRightOriented = true;
	}

	do {

		if (isLeftOriented == true) //checks the entry point white or black, it must be black
		{
			startSeedlingPoint = startSeedlingPoint - horizontalMarginValueForBottomStart;
			//cout << "startSeedlingPoint: " << startSeedlingPoint << endl;
			currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(bottomStartPoint, startSeedlingPoint);
			//cout << "currentPixelValueAtCoordinate: " << currentPixelValueAtCoordinate << endl;
			circle(filteredImageNew_3D, Point(startSeedlingPoint, bottomStartPoint), 0, Scalar(0, 255, 0), -1);
		}

		if (isRightOriented == true) //checks the entry point white or black, it must be black
		{
			startSeedlingPoint = startSeedlingPoint + horizontalMarginValueForBottomStart;
			currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(bottomStartPoint, startSeedlingPoint);
			circle(filteredImageNew_3D, Point(startSeedlingPoint, bottomStartPoint), 0, Scalar(0, 255, 0), -1);
		}
	} while (currentPixelValueAtCoordinate != 0);

	circle(filteredImageNew_3D, Point(startSeedlingPoint, bottomStartPoint), 0, Scalar(255, 0, 255), -1);

	definedRectWidth = definedRectWidth * 2;

	while (rowThicknessWhenCollapsed <= (bodyThickness + epsilon)) {
		for (int y = startSeedlingPoint; y < startSeedlingPoint + 1; y++)
		{
			for (int x = bottomStartPoint; x > 0; x--)
			{
				heightSeedlingSum++;
				determineThresholdSeedlingToLeaf(y, x, heightSeedling, currentPixelValueAtCoordinate, rowCheckIsDone,
					finalWhitePixelRight2, sumWhitePixelToTheRight2, filteredImageNew, filteredImageNew_3D,
					currentValueRight, currentValueLeft, finalWhitePixelLeft2, sumWhitePixelToTheLeft2,
					rowThicknessWhenCollapsed, currentHeightAtWhitePoint);

				if (rowCheckIsDone == true)
				{
					bottomStartPoint = bottomStartPoint - currentHeightAtWhitePoint;
					if (isLeftOriented == true) {
						startSeedlingPoint = startSeedlingPoint - horizontalMarginValueForBottomStart; // if the seedling is aligned left
						y = startSeedlingPoint;
					}
					else if (isRightOriented == true)
					{
						startSeedlingPoint = startSeedlingPoint + horizontalMarginValueForBottomStart; // if the seedling is aligned right
						y = startSeedlingPoint;
					}
					break;
				}
			}
		}
	}
	/*calculates seedling Body Height by subtracting bottom margin value to determine the least bottom point point, then added vertical margin value with the road lenght to reach the top*/
	int bottomPartOfBodyHeight = (filteredImageNew.rows - bottom_margin) - lastWhitePixelInHighestIntensityLine + verticalMarginValueForBottomStartPoint;
	//cout << "bottomPartOfBodyHeight: " << bottomPartOfBodyHeight << endl;
	float bodyHeight = heightSeedlingSum + bottomPartOfBodyHeight;

	int leftStartPixelPoint = 0, currentPixelValueAtCoordinateRight = 0, currentPixelValueAtCoordinateLeft = 0;
	Mat filterRect;
	if (isLeftOriented == true) {
		leftStartPixelPoint = startSeedlingPoint + horizontalMarginValueForBottomStart;
		Rect filterRegion(leftStartPixelPoint - (100 + bodyThickness), bottomStartPoint - (100 + bodyThickness), 100 + bodyThickness, 100 + bodyThickness);
		filterRect = filteredImageNew(filterRegion);

	}
	else if (isRightOriented == true) {
		leftStartPixelPoint = startSeedlingPoint - horizontalMarginValueForBottomStart;
		Rect filterRegion(leftStartPixelPoint, bottomStartPoint - 100, 100, 100);
		filterRect = filteredImageNew(filterRegion);
	}

	cout << "statusRight: " << isRightOriented << endl;
	cout << "statusLeft: " << isLeftOriented << endl;
#pragma region find contours and draw circle around
	vector<vector<cv::Point>> contours;
	vector<Vec4i> hierarchy;
	//filteredImageNew = imread("C:/Users/HTG_SOFTWARE/Desktop/yatay.png");
	//cvtColor(filteredImageNew, filterRect, COLOR_RGB2GRAY);
	findContours(filterRect, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, cv::Point());

	Mat f5 = filterRect.clone();

	vector<vector<Point> > contours_poly(contours.size());
	vector<Rect> boundRect(contours.size());
	vector<Point2f>centers(contours.size());
	vector<float>radius(contours.size());
	for (size_t i = 0; i < contours.size(); i++)
	{
		approxPolyDP(contours[i], contours_poly[i], 3, true);
		boundRect[i] = boundingRect(contours_poly[i]);
		minEnclosingCircle(contours_poly[i], centers[i], radius[i]);
	}
	Mat drawing = Mat::zeros(f5.size(), CV_8UC1);
	Mat drawingEroded;
	for (size_t i = 0; i < contours.size(); i++)
	{
		//drawContours(drawing, contours_poly, (int)i, 255);
		//rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), 255, 1);
		circle(drawing, centers[i], (int)radius[i], 255, 1);
	}

	contours.clear(); hierarchy.clear();

	morphologyEx(drawing, drawingEroded, MORPH_DILATE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(2, 2)), Point(-1, -1), 1);
	int tempMaxArea = 0, index = 0;
	Mat contours_labels, contours_stats, contours_centroids, contours_doubleStats;
	int countParticles = analyzeParticles(drawingEroded, contours_labels, contours_stats, contours_centroids, contours_doubleStats, ParticleAnalyzer::FOUR_CONNECTED);

	for (int j = 1; j < countParticles; j++) {
		int areaCurrent = contours_stats.at<int>(j, CC_STAT_AREA);
		if (areaCurrent > tempMaxArea) {
			tempMaxArea = areaCurrent;
			index = j;
		}
	}

	Mat cellRoi = getRoiMask(contours_labels, contours_stats, index);
	cv::Mat output, channelsConcatenated, channelsConcatenatedDilated;
	flip(cellRoi, output, 0);
	vconcat(cellRoi, output, channelsConcatenated);
	morphologyEx(channelsConcatenated, channelsConcatenatedDilated, MORPH_DILATE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 5);
	Mat contours_labels1, contours_stats1, contours_centroids1, contours_doubleStats1;

	vector<vector<Point>> objectContours;
	vector<Vec4i> objectHierarchy;
	findContours(channelsConcatenatedDilated, objectContours, objectHierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE); // alt. CV_RETR_TREE for outer contour

	//Mat test1 = Mat::zeros(channelsConcatenatedDilated.size(), CV_8UC1);
	//drawContours(test1, objectContours, (int)1, 255, 1, LINE_8, hierarchy, 0);
	vector<vector<Point>> objectContoursPoly(objectContours.size());
	approxPolyDP(Mat(objectContours[0]), objectContoursPoly[0], 3, true);
	double areaCurrent = contourArea(Mat(objectContoursPoly[0]), false);
	double perimeterOfDirection = arcLength(Mat(objectContoursPoly[0]), true);
	double circularityOfDirection = (4 * M_PI * areaCurrent) / pow(perimeterOfDirection, 2.0);
	cout << "circularityOfDirection: " << circularityOfDirection << endl;
	double circularityLimit = 0.500000;
	bool upward = false, downward = false;
	if (circularityOfDirection > circularityLimit) {
		cout << "direction is downward! " << endl;
		downward = true;
	}
	if (circularityOfDirection <= circularityLimit)
	{
		cout << "direction is upward! " << endl;
		upward = true;
	}
#pragma endregion

	//start points for leaf height calculation are here
	int countCurrent = 0;
	int BX = bottomStartPoint;
	int BY = leftStartPixelPoint;

	int controlCurrentPixelValueAtCoordinate = 0;
	bool continueToIterate = false;

	//cout << "points1: " << Point(BX, BY) << endl;

	int newX = 0;
	bool stageSecond = false, finalPoint = false, directionUpdated = false;
	vector<float> peakDistances;
	float leafLength;

	for (int x = BX; x > 0; x--)
	{
		currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(x - 1, BY);
		//cout << "points-guncel: " << Point(x - 1, BY) << endl;
		// colors the test area to let developer track the code
		circle(filteredImageNew_3D, Point(BY, x), 0, Scalar(255, 0, 255), -1);
		//cout << "currentPixelValueAtCoordinate: " << currentPixelValueAtCoordinate << endl;
		//cout << "currentPixelValueAtCoordinate: " << currentPixelValueAtCoordinate << endl;
		const Point2i pointFirst(BY, x);
		if (currentPixelValueAtCoordinate == 0) {
			if (directionUpdated == false) {
				checkComputeDirection(isLeftOriented, isRightOriented, x, BY, currentPixelValueAtCoordinate, filteredImageNew, directionUpdated);
			}

			determineLeafStartToLeafPeakPoint(x, BY, controlCurrentPixelValueAtCoordinate, filteredImageNew,
				filteredImageNew_3D, filteredImageNew_clone, isLeftOriented,
				isRightOriented, pointFirst, stageSecond, peakDistances, bottomStartPoint,
				leftStartPixelPoint, finalPoint, leafLength, newX);
			currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(x - 1, BY);
			if (currentPixelValueAtCoordinate == 0) {
				cout << "x: " << x << endl;
				cout << "BY: " << BY << endl;
				leafLength = sqrt((x - bottomStartPoint) * (x - bottomStartPoint) + (BY - leftStartPixelPoint) * (BY - leftStartPixelPoint));
				cout << "leafLength_at_firstPeak: " << leafLength << endl;
				newX = x;
				break;
			}
		}
	}
	if (downward == true)
	{
		stageSecond = true;
		//cout << "PointCurrent: " << Point(newX, BY) << endl;
		for (int x = newX; x < filteredImageNew.rows; x++) {
			currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(x + 1, BY);
			// colors the test area to let developer track the code
			circle(filteredImageNew_3D, Point(BY, x), 0, Scalar(255, 0, 255), -1);
			circle(filteredImageNew_clone, Point(BY, x), 0, Scalar(255), -1);
			const Point2i pointFirst(BY, x);
			//cout << "alo x: " << x << endl;
			if (currentPixelValueAtCoordinate == 0) {
				const Point2i pointSecond(BY, x);
				//get the center pixel of the line in order to jump through vertically
				const Moments z = moments(filteredImageNew_clone, false);
				const Point p1(z.m10 / z.m00, z.m01 / z.m00);
				morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
				x = p1.y;
				BY = p1.x;
				determineLeafStartToLeafPeakPoint(x, BY, controlCurrentPixelValueAtCoordinate, filteredImageNew,
					filteredImageNew_3D, filteredImageNew_clone, isLeftOriented,
					isRightOriented, pointFirst, stageSecond, peakDistances,
					bottomStartPoint, leftStartPixelPoint, finalPoint, leafLength, newX);
				if (finalPoint == true)
				{
					break;
				}
			}
		}
	}

	cout << "*****************_Calculated Results_(px)***************" << endl;
	cout << "bodyThickness: " << bodyThickness << endl;
	cout << "bodyHeight: " << bodyHeight << endl;
	cout << "leafLength: " << leafLength << endl;
	cout << "********************_Real Results_(mm)******************" << endl;
	cout << "realBodyThickness: " << realBodyThickness << endl;
	cout << "realBodyHeight: " << realBodyHeight << endl;
	cout << "realLeafLength: " << realLeafLength << endl;

	float calibrationValueLL = ((realLeafLength * 100) / (leafLength * 100));
	float calibrationValueBH = ((realBodyHeight * 100) / (bodyHeight * 100));
	float calibrationValueBT = ((realBodyThickness * 100) / (bodyThickness * 100));
	cout << "****************_Calibration Values_******************" << endl;
	cout << "calibrationValueLL: " << (ceil((calibrationValueLL * 1000)) / 1000) << endl;
	cout << "calibrationValueBH: " << (ceil((calibrationValueBH * 1000)) / 1000) << endl;
	cout << "calibrationValueBT: " << (ceil((calibrationValueBT * 1000)) / 1000) << endl;
	cout << "end for now: " << endl;
	
	return result;
}

auto determineThresholdSeedlingToLeaf(int y, int x, int& heightSeedling,
	int currentPixelValueAtCoordinate,
	bool& rowCheckIsDone, bool finalWhitePixelRight2, int sumWhitePixelToTheRight2,
	Mat filteredImageNew, Mat filteredImageNew_3D, int currentValueRight,
	int currentValueLeft, bool finalWhitePixelLeft2, int sumWhitePixelToTheLeft2,
	int& rowThicknessWhenCollapsed, int& currentHeightAtWhitePoint) -> void
{

	//cout << "Point(x, y): " << Point(x, y) << endl;
	heightSeedling++;
	rowCheckIsDone = false;
	//cout << "heightSeedling: " << heightSeedling << endl;

	currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(x, y);

	if (rowCheckIsDone == false) { // colors the test area to let developer track the code
		circle(filteredImageNew_3D, Point(y, x), 0, Scalar(255, 0, 0), -1);
	}

	if (currentPixelValueAtCoordinate == 255 && rowCheckIsDone == false)
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

auto determineLeafStartToLeafPeakPoint(int& x, int& y, int currentPixelValueAtCoordinate, Mat filteredImageNew,
	Mat filteredImageNew_3D, Mat filteredImageNew_clone, bool isLeftOriented,
	bool isRightOriented, const Point2i& pointFirst, bool stageSecond, vector<float>& peakDistances,
	int bottomStartPoint, int leftStartPixelPoint, bool& finalPoint, float& leafLength, int& newX) -> void
{

	if (isLeftOriented == true) {
		//for forwarding to the left from the current collapsed pixel
		for (int m = y; m > -1; m--)
		{
			circle(filteredImageNew_3D, Point(m, x), 0, Scalar(0, 0, 255), -1);

			if (stageSecond != true) {
				circle(filteredImageNew_clone, Point(m, x), 0, Scalar(255), -1);
			}

			// this gets the next pixel at the current spesific pixel in order to calculate if it is collapsed the corner of the object to move forward, looks the next left pixel
			currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(x, m - 1);

			if (currentPixelValueAtCoordinate == 0 && stageSecond != true)
			{
				const Point2i pointSecond(m, x);
				//get the center pixel of the line in order to jump through vertically
				const Moments z = moments(filteredImageNew_clone, false);
				const Point p1(z.m10 / z.m00, z.m01 / z.m00);
				morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
				x = p1.y;
				y = p1.x;
				//cout << "endLeft1 " << endl;
				break;
			}
			else if (currentPixelValueAtCoordinate == 0 && stageSecond == true)
			{
				const Point2i pointSecond(m, x);
				y = m; // ?
				if (pointFirst.x == pointSecond.x && stageSecond == true)
				{
					leafLength = sqrt((x - bottomStartPoint) * (x - bottomStartPoint) + (y - leftStartPixelPoint) * (y - leftStartPixelPoint));
					//cout << "leafLength: " << leafLength << endl;
					peakDistances.push_back(leafLength);
					finalPoint = true;
					//cout << "endLeft2 " << endl;
				}
				break;
			}
		}

	}
	else if (isRightOriented == true)
	{
		for (int m = y; m < filteredImageNew.cols; m++)
		{
			circle(filteredImageNew_3D, Point(m, x), 0, Scalar(0, 0, 255), -1);
			if (stageSecond != true) {
				circle(filteredImageNew_clone, Point(m, x), 0, Scalar(255), -1);
			}
			// this gets the next pixel at the current specific pixel in order to calculate if it is collapsed the corner of the object to move forward, looks the next left pixel
			currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(x, m + 1);

			if (currentPixelValueAtCoordinate == 0 && stageSecond != true)
			{
				const Point2i pointSecond(m, x);
				//get the center pixel of the line in order to jump through vertically
				const Moments z = moments(filteredImageNew_clone, false);
				const Point p1(z.m10 / z.m00, z.m01 / z.m00);
				morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
				x = p1.y;
				y = p1.x;
				//cout << "endLeft2 " << endl;
				break;
			}
			else if (currentPixelValueAtCoordinate == 0 && stageSecond == true)
			{
				const Point2i pointSecond(m, x);
				y = m;
				newX = x;
				if (pointFirst.x == pointSecond.x && stageSecond == true)
				{
					leafLength = sqrt((x - bottomStartPoint) * (x - bottomStartPoint) + (y - leftStartPixelPoint) * (y - leftStartPixelPoint));
					//cout << "leafLength: " << leafLength << endl;
					peakDistances.push_back(leafLength);
					finalPoint = true;
					//cout << "endRight2 " << endl;
				}
				break;
			}
		}
	}
}
void checkComputeDirection(bool& isLeftOriented, bool& isRightOriented, int x, int y, int currentPixelValueAtCoordinate, Mat filteredImageNew, bool& directionUpdated)
{
	currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(x, y + 1);
	if (currentPixelValueAtCoordinate == 0)
	{
		isLeftOriented = true;
		isRightOriented = false;
	}
	currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(x, y - 1);
	if (currentPixelValueAtCoordinate == 0)
	{
		isLeftOriented = false;
		isRightOriented = true;
	}
	directionUpdated = true;
};
