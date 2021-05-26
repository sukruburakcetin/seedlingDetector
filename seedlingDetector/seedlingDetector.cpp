#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include "seedlingDetector.hpp"
#include <opencv2/imgproc.hpp>
#include "AutoThreshold.hpp"
#include "ParticleAnalyzer.hpp"
#include "RankFilters.hpp"
#include "FillHoles.hpp"
#include "WatershedCustom.hpp"

using namespace std;
using namespace cv;


auto determineThresholdSeedlingToLeaf(int y, int x, int& heightSeedling,
	int currentPixelValueAtCoordinate,
	bool& rowCheckIsDone, bool finalWhitePixelRight2, int sumWhitePixelToTheRight2,
	Mat filteredImageNew, Mat filteredImageNew_3D, int currentValueRight,
	int currentValueLeft, bool finalWhitePixelLeft2, int sumWhitePixelToTheLeft2,
	int& rowThicknessWhenCollapsed, int& currentHeightAtWhitePoint) -> void;

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
	/*-----highestIntensityColumnIndex == yukarýdan aþaðýya en yoðun column(vertical line)-----*/
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

	/*------lastWhitePixelInLine == en yoðun column(vertical line)daki taban obje pikseli------*/
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

	int seedlingThickness = sumWhitePixelToTheLeft + sumWhitePixelToTheRight;
	cout << "seedlingThickness(OguzhanHoca): " << seedlingThickness << endl;


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
	//cout << "currentPixelValueAtCoordinateBefore: " << currentPixelValueAtCoordinate << endl;

	//circle(filteredImageNew_3D, Point(startSeedlingPoint, bottomStartPoint), 0, Scalar(255, 0, 255), -1);

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

	while (rowThicknessWhenCollapsed <= (seedlingThickness + epsilon)) {
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
	int leftStartPixelPoint = 0, currentPixelValueAtCoordinateRight = 0, currentPixelValueAtCoordinateLeft = 0;
	Mat filterRect;
	if (isLeftOriented == true){
		leftStartPixelPoint = startSeedlingPoint + horizontalMarginValueForBottomStart;
		Rect filterRegion(leftStartPixelPoint-(100+seedlingThickness), bottomStartPoint-(100+seedlingThickness),100+seedlingThickness, 100+ seedlingThickness);
		filterRect = filteredImageNew(filterRegion);

	}
	else if (isRightOriented == true) {
		leftStartPixelPoint = startSeedlingPoint - horizontalMarginValueForBottomStart;
		Rect filterRegion(leftStartPixelPoint-100, bottomStartPoint-100, 100, 100);
		filterRect = filteredImageNew(filterRegion);
	}

	cout << "statusRight: " << isRightOriented << endl;
	cout << "statusLeft: " << isLeftOriented << endl;

//#pragma region find contours and draw circle around
//	vector<vector<cv::Point>> contours;
//	vector<Vec4i> hierarchy;
//	filteredImageNew = imread("C:/Users/HTG_SOFTWARE/Desktop/dikey.png");
//	cvtColor(filteredImageNew, filterRect, COLOR_RGB2GRAY);
//	findContours(filterRect, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, cv::Point());
//
//	Mat f5 = filterRect.clone();
//
//	
//	vector<vector<Point> > contours_poly(contours.size());
//	vector<Rect> boundRect(contours.size());
//	vector<Point2f>centers(contours.size());
//	vector<float>radius(contours.size());
//	for (size_t i = 0; i < contours.size(); i++)
//	{
//		approxPolyDP(contours[i], contours_poly[i], 3, true);
//		boundRect[i] = boundingRect(contours_poly[i]);
//		minEnclosingCircle(contours_poly[i], centers[i], radius[i]);
//	}
//	Mat drawing = Mat::zeros(f5.size(), CV_8UC1);
//	Mat drawingEroded;
//	for (size_t i = 0; i < contours.size(); i++)
//	{
//		//drawContours(drawing, contours_poly, (int)i, 255);
//		//rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), 255, 1);
//		circle(drawing, centers[i], (int)radius[i], 255, 1);
//	}
//
//	contours.clear(); hierarchy.clear();
//#pragma endregion
//
//	
//	morphologyEx(drawing, drawingEroded, MORPH_DILATE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(2, 2)), Point(-1, -1), 1);
//
//	Mat contours_labels, contours_stats, contours_centroids, contours_doubleStats;
//	//analyzeParticles(f5, contours_labels, contours_stats, contours_centroids, contours_doubleStats, ParticleAnalyzer::FOUR_CONNECTED, 20);
//	analyzeParticles(drawingEroded, contours_labels, contours_stats, contours_centroids, contours_doubleStats, ParticleAnalyzer::FOUR_CONNECTED, 20);
//	cout << "LeftPixel: " << contours_stats.at<int>(1, CC_STAT_LEFT) << endl;
//	cout << "TopPixel: " << contours_stats.at<int>(1, CC_STAT_TOP) << endl;


	//start points for leaf height calculation are here
	int countCurrent = 0;
	for (int y = leftStartPixelPoint; y < leftStartPixelPoint + 1; y++)
	{
		for (int x = bottomStartPoint; x > 0; x--)
		{
			currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(x, y);
			//rowCheckIsDone = false;

			// colors the test area to let developer track the code
			circle(filteredImageNew_3D, Point(y, x), 0, Scalar(255, 0, 255), -1);


			if (currentPixelValueAtCoordinate == 0)
			{
				currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(x, y + 1);

				if (currentPixelValueAtCoordinate == 0) {

					//for forwarding to the left from the current collapsed pixel
					for (int l = x; l < x + 1; l++)
					{
						for (int m = y; m > -1; m--)
						{
							/*					cout << "x: " << l << endl;
												cout << "y: " << m << endl;*/

							circle(filteredImageNew_3D, Point(m, l), 0, Scalar(0, 0, 255), -1);
							circle(filteredImageNew_clone, Point(m, l), 0, Scalar(255), -1);

							// this gets the next pixel at the current spesific pixel in order to calculate if it is collapsed the corner of the object to move forward, looks the next left pixel
							currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(l, m - 1);
							//countCurrent = countCurrent + 1;
							if (currentPixelValueAtCoordinate == 0)
							{
								//get the center pixel of the line in order to jump through vertically
								Moments z = moments(filteredImageNew_clone, false);
								Point p1(z.m10 / z.m00, z.m01 / z.m00);
								morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);

								//circle(filteredImageNew_clone, p1, 0, Scalar(0, 255, 0), -1);

								/*cout << "p1.y: " << p1.y << endl;
								cout << "p1.x: " << p1.x << endl;*/
								for (int o = p1.x; o < p1.x + 1; o++)
								{
									for (int p = p1.y; p > -1; p--)
									{
										//BGR is normal format when using scalar
										circle(filteredImageNew_3D, Point(o, p), 0, Scalar(255, 0, 0), -1);

										//cout << "points: " << Point(o, p) << endl;

										//p-1 looks the next top pixel at the spesific point
										currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(p - 1, o);

										//cout << "currentPixelValueAtCoordinate: " << currentPixelValueAtCoordinate << endl;

										if (currentPixelValueAtCoordinate == 0)
										{
											for (int a = p; a < p + 1; a++)
											{
												for (int b = o; b > -1; b--)
												{
													cout << "points: " << Point(a, b) << endl;
											/*		countCurrent = countCurrent + 1;
													if (countCurrent == 2) {


														Rect ccomp;
														cout << "points: " << Point(a, b) << endl;
														floodFill(filteredImageNew_3D, Point(576, 169), Scalar(155, 255, 55), &ccomp, Scalar(20, 20, 20), Scalar(20, 20, 20));
													}*/

													circle(filteredImageNew_3D, Point(b, a), 0, Scalar(0, 0, 255), -1);
													circle(filteredImageNew_clone, Point(b, a), 0, Scalar(255), -1);
													currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(a, b - 1);

													//countCurrent = countCurrent + 1;
													if (currentPixelValueAtCoordinate == 0)
													{

														int lineLenght = countNonZero(filteredImageNew_clone);
														Moments z = moments(filteredImageNew_clone, false);
														Point p1(z.m10 / z.m00, z.m01 / z.m00);
														//p1.y is actually X point on the coordinate system
														cout << "points: " << Point((p1.y) - 1, p1.x) << endl;

														currentPixelValueAtCoordinate = filteredImageNew.at<uchar>((p1.y) - 1, p1.x);
														cout << "currentPixelValueAtCoordinate1: " << currentPixelValueAtCoordinate << endl;

														morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
														if(currentPixelValueAtCoordinate !=0){
															for (int g = p1.x; g < p1.x + 1; g++)
															{
																for (int h = p1.y; h > -1; h--)
																{
																	circle(filteredImageNew_3D, Point(g, h), 0, Scalar(0, 255, 0), -1);
																	currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(h - 1, g);

																		cout << "points: " << Point(a, b) << endl;

																		if (currentPixelValueAtCoordinate == 0) {
																	
																			for (int t = h; t < h + 1; h++)
																			{
																				for (int r = g; r > -1; r--)
																				{
																					circle(filteredImageNew_3D, Point(r, t), 0, Scalar(0, 0, 255), -1);
																					circle(filteredImageNew_clone, Point(r, t), 0, Scalar(255), -1);
																					currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(t, r - 1);

																					if (currentPixelValueAtCoordinate == 0)
																					{
																						cout << "points: " << Point(r, t) << endl;

																						int lineLenght = countNonZero(filteredImageNew_clone);
																						Moments z = moments(filteredImageNew_clone, false);
																						Point p1(z.m10 / z.m00, z.m01 / z.m00);
																						//p1.y is actually X point on the coordinate system
																						cout << "points: " << Point((p1.y) - 1, p1.x) << endl;

																						currentPixelValueAtCoordinate = filteredImageNew.at<uchar>((p1.y) - 1, p1.x);
																						cout << "currentPixelValueAtCoordinate1: " << currentPixelValueAtCoordinate << endl;

																						morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);

																						if (currentPixelValueAtCoordinate != 0) {
																							cout << "currentPixelValueAtCoordinate1: " << currentPixelValueAtCoordinate << endl;
																							for (int g = p1.x; g < p1.x + 1; g++)
																							{
																								for (int h = p1.y; h > -1; h--)
																								{
																									circle(filteredImageNew_3D, Point(g, h), 0, Scalar(0, 255, 0), -1);
																									currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(h - 1, g);


																									if (currentPixelValueAtCoordinate == 0) {
																										cout << "points: " << Point(a, b) << endl;
																										for (int t = h; t < h + 1; h++)
																										{
																											for (int r = g; r > -1; r--)
																											{
																												circle(filteredImageNew_3D, Point(r, t), 0, Scalar(0, 0, 255), -1);
																												circle(filteredImageNew_clone, Point(r, t), 0, Scalar(255), -1);
																												currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(t, r - 1);
																												if (currentPixelValueAtCoordinate == 0)
																												{
																													int lineLenght = countNonZero(filteredImageNew_clone);
																													Moments z = moments(filteredImageNew_clone, false);
																													Point p1(z.m10 / z.m00, z.m01 / z.m00);
																													//p1.y is actually X point on the coordinate system
																													cout << "points: " << Point((p1.y) - 1, p1.x) << endl;

																													currentPixelValueAtCoordinate = filteredImageNew.at<uchar>((p1.y) - 1, p1.x);
																													cout << "currentPixelValueAtCoordinate1: " << currentPixelValueAtCoordinate << endl;

																													morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
																													if (currentPixelValueAtCoordinate != 0) {
																													}
																													else if (currentPixelValueAtCoordinate == 0) {
																														cout << "You are at peak point: " << endl;

																														int leafLenght = sqrt(((p1.x) - bottomStartPoint) * ((p1.x) - bottomStartPoint) + ((p1.y) - leftStartPixelPoint) * ((p1.y) - leftStartPixelPoint));
																														cout << "Leaf Lenght: " << leafLenght << endl;
																														cout << "You are at peak point: " << endl;
																													}
																												}
																											}
																										}

																									}
																								}
																							}
																						}

																					}

																				}
																			}

																		}
																}
															}
														}
														else if (currentPixelValueAtCoordinate == 0) {

															for (int g = p1.x; g < p1.x + 1; g++)
															{
																for (int h = p1.y; h < filteredImageNew.rows; h++)
																{
																	circle(filteredImageNewFilled, Point(g, h), 0, Scalar(0, 255, 0), -1);
																	circle(filteredImageNew_clone, Point(g, h), 0, Scalar(255), -1);
																	currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(h, g + 1);

																	if (currentPixelValueAtCoordinate == 0)
																	{

																		int lineLenght = countNonZero(filteredImageNew_clone);
																		Moments z = moments(filteredImageNew_clone, false);
																		Point p1(z.m10 / z.m00, z.m01 / z.m00);
																		//p1.y is actually X point on the coordinate system
																		cout << "points: " << Point((p1.y) - 1, p1.x) << endl;
																		morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
																		for (int f = p1.y; f < p1.y + 1; f++)
																		{
																			for (int c = p1.x; c > -1; c--)
																			{

																				//BGR is normal format when using scalar
																				circle(filteredImageNewFilled, Point(c, f), 0, Scalar(255, 0, 0), -1);
																				currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(f, c-1);


																				if (currentPixelValueAtCoordinate == 0) {
																					for (int t = c; t < c + 1; c++)
																					{
																						for (int s = f; s < filteredImageNew.rows; s++)
																						{
																							//cout << "points: " << Point(t, s) << endl;
																							circle(filteredImageNewFilled, Point(t, s), 0, Scalar(255, 0, 255), -1);
																							circle(filteredImageNew_clone, Point(t, s), 0, Scalar(255), -1);
																							currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(s+1, t);

																							if (currentPixelValueAtCoordinate == 0) {
																								int lineLenght = countNonZero(filteredImageNew_clone);
																								Moments z = moments(filteredImageNew_clone, false);
																								Point p1(z.m10 / z.m00, z.m01 / z.m00);
																								//p1.y is actually X point on the coordinate system
																								morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
																								//cout << "points: " << Point((p1.y) - 1, p1.x) << endl;
																								for (int f = p1.y; f < p1.y + 1; f++)
																								{
																									for (int c = p1.x; c > -1; c--)
																									{
																										circle(filteredImageNewFilled, Point(c, f), 0, Scalar(255, 0, 0), -1);
																										currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(f, c - 1);

																										if (currentPixelValueAtCoordinate == 0) {
																											cout << "pause: " << endl;
																										}
																									}
																								}
																							}

																						}
																					}
																				}
																			}
																		}
																	}
																}
															}

														}

													}

												}
											}
										}
									}
								}
							}
							//currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(m, l);
						}
					}

					////for forwarding to the left from the current collapsed pixel
					//for (int j = x; j < x + 1; j++)
					//{
					//	for (int k = y; k < filteredImageNew.cols; k++)
					//	{

					//	}
					//}
				}

			}
		}

	}


	cout << "startSeedlingPoint(y-end): " << leftStartPixelPoint << endl;
	cout << "bottomStartPoint(x-end): " << bottomStartPoint << endl;










	//determine and crop seedling_area in matrix
	cv::Rect rectSeedling(leftStart + horizontalMarginValueForBottomStart, bottomStartPoint, (highestIntensityColumnIndex - leftStart) + seedlingThickness * 2, heightSeedlingSum + verticalMarginValueForBottomStartPoint);

	seedlingArea = filteredImageNew(rectSeedling);

	cout << "heightSeedlingSum: " << heightSeedlingSum << endl;


	Mat seedlingAreaEroded, seedlingAreaDilated;
	morphologyEx(seedlingArea, seedlingAreaEroded, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 3);
	morphologyEx(seedlingAreaEroded, seedlingAreaDilated, MORPH_DILATE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 3);

	int tempNextPoint = 0, averageWhitePixels = 0;
	int whitePointsSuccesfulStreakAtCurrentRowNew = 0;

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

	int seedlingHeight = heightSeedlingSum + verticalMarginValueForBottomStartPoint;
	cout << "seedlingHeight: " << seedlingHeight << " pixel." << endl;

	return result;
}

void determineThresholdSeedlingToLeaf(int y, int x, int& heightSeedling,
	int currentPixelValueAtCoordinate,
	bool& rowCheckIsDone, bool finalWhitePixelRight2, int sumWhitePixelToTheRight2,
	Mat filteredImageNew, Mat filteredImageNew_3D, int currentValueRight,
	int currentValueLeft, bool finalWhitePixelLeft2, int sumWhitePixelToTheLeft2,
	int& rowThicknessWhenCollapsed, int& currentHeightAtWhitePoint)
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
