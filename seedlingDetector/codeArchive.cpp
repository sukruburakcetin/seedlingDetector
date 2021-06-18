
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





/*This blocks determine leaf artifact on the seedling starts*/
//int leafStartPixelRowAmountInBody = 0, bodyToLeafMargin = 5;
//for (int x = 0; x < seedlingAreaDilated.rows; x++)
//{
//	for (int y = 0; y < seedlingAreaDilated.cols; y++)
//	{
//		//cout << "x: " << x << endl;
//		//cout << "y: " << y << endl;
//		value = seedlingAreaDilated.at<uchar>(x, y);
//		//cout << "value: " << value << endl;
//		tempNextPoint = seedlingAreaDilated.at<uchar>(x, y + 1);
//		if (value == 255 && tempNextPoint == 255) {
//			whitePointsSuccesfulStreakAtCurrentRowNew = whitePointsSuccesfulStreakAtCurrentRowNew + 1;
//			/*seedlingAreaDilated.at<uchar>(x, y) = 0;*/
//			//cout << "point: " << Point(x, y) << endl;
//			//seedlingArea.at<uchar>(x, y) = 0;
//			//cout << "whitePointsSuccesfulStreakAtCurrentRow: " << whitePointsSuccesfulStreakAtCurrentRowNew << endl;
//
//			//cout << "pointbefore: " << Point(x, y) << endl;
//			//cout << "whitePointsSuccesfulStreakAtCurrentRowNew: " << whitePointsSuccesfulStreakAtCurrentRowNew << endl;
//		}
//		else if (whitePointsSuccesfulStreakAtCurrentRowNew >= averageWhitePixels + bodyToLeafMargin && check == false)
//		{
//			//cout << "whitePointsSuccesfulStreakAtCurrentRow: " << whitePointsSuccesfulStreakAtCurrentRowNew << endl;
//			for (int z = 0; z < seedlingArea.rows; z++)
//			{
//				seedlingArea.at<uchar>(x, z) = 0;
//				//floodFill(filteredImageNew, Point2d(x, z), 127);
//			}
//			check = true;
//			leafStartPixelRowAmountInBody = leafStartPixelRowAmountInBody + 1;
//		}
//		else if (tempyNewSecond != x)
//		{
//			whitePointsSuccesfulStreakAtCurrentRowNew = 0;
//			check = false;
//		}
//		tempyNewSecond = x;
//	}
//}
////cout << "leafStartPixelRowAmountInBody: " << leafStartPixelRowAmountInBody << endl;
////int seedlingHeight = (seedlingArea.rows + verticalMarginValue) - leafStartPixelRowAmountInBody;
///
/*This blocks determine leaf artifact on the seedling ends*/
///
///
///
///
///
///
///
///
///
///


#ifndef iterating on leaf section
					//for (int y = leftStartPixelPoint; y < leftStartPixelPoint + 1; y++)
	//{
	//	for (int x = bottomStartPoint; x > 0; x--)
	//	{
	//		currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(x, y);
	//		//rowCheckIsDone = false;

	//		// colors the test area to let developer track the code
	//		circle(filteredImageNew_3D, Point(y, x), 0, Scalar(255, 0, 255), -1);


	//		if (currentPixelValueAtCoordinate == 0)
	//		{
	//			currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(x, y + 1);

	//			if (currentPixelValueAtCoordinate == 0) {

	//				//for forwarding to the left from the current collapsed pixel
	//				for (int l = x; l < x + 1; l++)
	//				{
	//					for (int m = y; m > -1; m--)
	//					{
	//						/*					cout << "x: " << l << endl;
	//											cout << "y: " << m << endl;*/

	//						circle(filteredImageNew_3D, Point(m, l), 0, Scalar(0, 0, 255), -1);
	//						circle(filteredImageNew_clone, Point(m, l), 0, Scalar(255), -1);

	//						// this gets the next pixel at the current spesific pixel in order to calculate if it is collapsed the corner of the object to move forward, looks the next left pixel
	//						currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(l, m - 1);
	//						//countCurrent = countCurrent + 1;
	//						if (currentPixelValueAtCoordinate == 0)
	//						{
	//							//get the center pixel of the line in order to jump through vertically
	//							Moments z = moments(filteredImageNew_clone, false);
	//							Point p1(z.m10 / z.m00, z.m01 / z.m00);
	//							morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);

	//							//circle(filteredImageNew_clone, p1, 0, Scalar(0, 255, 0), -1);

	//							/*cout << "p1.y: " << p1.y << endl;
	//							cout << "p1.x: " << p1.x << endl;*/
	//							for (int o = p1.x; o < p1.x + 1; o++)
	//							{
	//								for (int p = p1.y; p > -1; p--)
	//								{
	//									//BGR is normal format when using scalar
	//									circle(filteredImageNew_3D, Point(o, p), 0, Scalar(255, 0, 0), -1);

	//									//cout << "points: " << Point(o, p) << endl;

	//									//p-1 looks the next top pixel at the spesific point
	//									currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(p - 1, o);

	//									//cout << "currentPixelValueAtCoordinate: " << currentPixelValueAtCoordinate << endl;

	//									if (currentPixelValueAtCoordinate == 0)
	//									{
	//										for (int a = p; a < p + 1; a++)
	//										{
	//											for (int b = o; b > -1; b--)
	//											{
	//												//cout << "points: " << Point(a, b) << endl;
	//												/*		countCurrent = countCurrent + 1;
	//														if (countCurrent == 2) {


	//															Rect ccomp;
	//															cout << "points: " << Point(a, b) << endl;
	//															floodFill(filteredImageNew_3D, Point(576, 169), Scalar(155, 255, 55), &ccomp, Scalar(20, 20, 20), Scalar(20, 20, 20));
	//														}*/

	//												circle(filteredImageNew_3D, Point(b, a), 0, Scalar(0, 0, 255), -1);
	//												circle(filteredImageNew_clone, Point(b, a), 0, Scalar(255), -1);
	//												currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(a, b - 1);

	//												//countCurrent = countCurrent + 1;
	//												if (currentPixelValueAtCoordinate == 0)
	//												{

	//													int lineLenght = countNonZero(filteredImageNew_clone);
	//													Moments z = moments(filteredImageNew_clone, false);
	//													Point p1(z.m10 / z.m00, z.m01 / z.m00);
	//													//p1.y is actually X point on the coordinate system
	//													//cout << "points: " << Point((p1.y) - 1, p1.x) << endl;

	//													currentPixelValueAtCoordinate = filteredImageNew.at<uchar>((p1.y) - 1, p1.x);
	//													//cout << "currentPixelValueAtCoordinate1: " << currentPixelValueAtCoordinate << endl;

	//													morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
	//													if (currentPixelValueAtCoordinate != 0) {
	//														for (int g = p1.x; g < p1.x + 1; g++)
	//														{
	//															for (int h = p1.y; h > -1; h--)
	//															{
	//																circle(filteredImageNew_3D, Point(g, h), 0, Scalar(0, 255, 0), -1);
	//																currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(h - 1, g);

	//																//cout << "points: " << Point(a, b) << endl;

	//																if (currentPixelValueAtCoordinate == 0) {

	//																	for (int t = h; t < h + 1; h++)
	//																	{
	//																		for (int r = g; r > -1; r--)
	//																		{
	//																			circle(filteredImageNew_3D, Point(r, t), 0, Scalar(0, 0, 255), -1);
	//																			circle(filteredImageNew_clone, Point(r, t), 0, Scalar(255), -1);
	//																			currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(t, r - 1);

	//																			if (currentPixelValueAtCoordinate == 0)
	//																			{
	//																				//cout << "points: " << Point(r, t) << endl;

	//																				int lineLenght = countNonZero(filteredImageNew_clone);
	//																				Moments z = moments(filteredImageNew_clone, false);
	//																				Point p1(z.m10 / z.m00, z.m01 / z.m00);
	//																				//p1.y is actually X point on the coordinate system
	//																				//cout << "points: " << Point((p1.y) - 1, p1.x) << endl;

	//																				//currentPixelValueAtCoordinate = filteredImageNew.at<uchar>((p1.y) - 1, p1.x);
	//																				//cout << "currentPixelValueAtCoordinate1: " << currentPixelValueAtCoordinate << endl;

	//																				morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);

	//																				if (currentPixelValueAtCoordinate != 0) {
	//																					//cout << "currentPixelValueAtCoordinate1: " << currentPixelValueAtCoordinate << endl;
	//																					for (int g = p1.x; g < p1.x + 1; g++)
	//																					{
	//																						for (int h = p1.y; h > -1; h--)
	//																						{
	//																							circle(filteredImageNew_3D, Point(g, h), 0, Scalar(0, 255, 0), -1);
	//																							currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(h - 1, g);


	//																							if (currentPixelValueAtCoordinate == 0) {
	//																								//cout << "points: " << Point(a, b) << endl;
	//																								for (int t = h; t < h + 1; h++)
	//																								{
	//																									for (int r = g; r > -1; r--)
	//																									{
	//																										circle(filteredImageNew_3D, Point(r, t), 0, Scalar(0, 0, 255), -1);
	//																										circle(filteredImageNew_clone, Point(r, t), 0, Scalar(255), -1);
	//																										currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(t, r - 1);
	//																										if (currentPixelValueAtCoordinate == 0)
	//																										{
	//																											int lineLenght = countNonZero(filteredImageNew_clone);
	//																											Moments z = moments(filteredImageNew_clone, false);
	//																											Point p1(z.m10 / z.m00, z.m01 / z.m00);
	//																											//p1.y is actually X point on the coordinate system
	//																											//cout << "points: " << Point((p1.y) - 1, p1.x) << endl;

	//																											currentPixelValueAtCoordinate = filteredImageNew.at<uchar>((p1.y) - 1, p1.x);
	//																											//cout << "currentPixelValueAtCoordinate1: " << currentPixelValueAtCoordinate << endl;

	//																											morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
	//																											if (currentPixelValueAtCoordinate != 0) {
	//																											}
	//																											else if (currentPixelValueAtCoordinate == 0) {
	//																												cout << "You are at peak point: " << endl;

	//																												int leafLenght = sqrt(((p1.x) - bottomStartPoint) * ((p1.x) - bottomStartPoint) + ((p1.y) - leftStartPixelPoint) * ((p1.y) - leftStartPixelPoint));
	//																												cout << "Leaf Lenght: " << leafLenght << endl;
	//																												cout << "You are at peak point: " << endl;
	//																											}
	//																										}
	//																									}
	//																								}

	//																							}
	//																						}
	//																					}
	//																				}

	//																			}

	//																		}
	//																	}

	//																}
	//															}
	//														}
	//													}
	//													else if (currentPixelValueAtCoordinate == 0) {

	//														for (int g = p1.x; g < p1.x + 1; g++)
	//														{
	//															for (int h = p1.y; h < filteredImageNew.rows; h++)
	//															{
	//																circle(filteredImageNewFilled, Point(g, h), 0, Scalar(0, 255, 0), -1);
	//																circle(filteredImageNew_clone, Point(g, h), 0, Scalar(255), -1);
	//																currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(h, g + 1);

	//																if (currentPixelValueAtCoordinate == 0)
	//																{

	//																	int lineLenght = countNonZero(filteredImageNew_clone);
	//																	Moments z = moments(filteredImageNew_clone, false);
	//																	Point p1(z.m10 / z.m00, z.m01 / z.m00);
	//																	//p1.y is actually X point on the coordinate system
	//																	//cout << "points: " << Point((p1.y) - 1, p1.x) << endl;
	//																	morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
	//																	for (int f = p1.y; f < p1.y + 1; f++)
	//																	{
	//																		for (int c = p1.x; c > -1; c--)
	//																		{

	//																			//BGR is normal format when using scalar
	//																			circle(filteredImageNewFilled, Point(c, f), 0, Scalar(255, 0, 0), -1);
	//																			currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(f, c - 1);


	//																			if (currentPixelValueAtCoordinate == 0) {
	//																				for (int t = c; t < c + 1; c++)
	//																				{
	//																					for (int s = f; s < filteredImageNew.rows; s++)
	//																					{
	//																						//cout << "points: " << Point(t, s) << endl;
	//																						circle(filteredImageNewFilled, Point(t, s), 0, Scalar(255, 0, 255), -1);
	//																						circle(filteredImageNew_clone, Point(t, s), 0, Scalar(255), -1);
	//																						currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(s + 1, t);

	//																						if (currentPixelValueAtCoordinate == 0) {
	//																							int lineLenght = countNonZero(filteredImageNew_clone);
	//																							Moments z = moments(filteredImageNew_clone, false);
	//																							Point p1(z.m10 / z.m00, z.m01 / z.m00);
	//																							//p1.y is actually X point on the coordinate system
	//																							morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
	//																							//cout << "points: " << Point((p1.y) - 1, p1.x) << endl;
	//																							for (int f = p1.y; f < p1.y + 1; f++)
	//																							{
	//																								for (int c = p1.x; c > -1; c--)
	//																								{
	//																									circle(filteredImageNewFilled, Point(c, f), 0, Scalar(255, 0, 0), -1);
	//																									currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(f, c - 1);

	//																									if (currentPixelValueAtCoordinate == 0) {

	//																										for (int t = c; t < c + 1; c++)
	//																										{
	//																											for (int s = f; s < filteredImageNew.rows; s++)
	//																											{
	//																												//cout << "points: " << Point(t, s) << endl;
	//																												circle(filteredImageNewFilled, Point(t, s), 0, Scalar(255, 0, 255), -1);
	//																												circle(filteredImageNew_clone, Point(t, s), 0, Scalar(255), -1);
	//																												currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(s + 1, t);

	//																												if (currentPixelValueAtCoordinate == 0) {
	//																													int lineLenght = countNonZero(filteredImageNew_clone);
	//																													Moments z = moments(filteredImageNew_clone, false);
	//																													Point p1(z.m10 / z.m00, z.m01 / z.m00);
	//																													//p1.y is actually X point on the coordinate system
	//																													//cout << "points: " << Point((p1.y) - 1, p1.x) << endl;
	//																													morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
	//																													for (int f = p1.y; f < p1.y + 1; f++)
	//																													{
	//																														for (int c = p1.x; c > -1; c--)
	//																														{
	//																															//BGR is normal format when using scalar
	//																															circle(filteredImageNewFilled, Point(c, f), 0, Scalar(255, 0, 0), -1);
	//																															currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(f, c - 1);

	//																															if (currentPixelValueAtCoordinate == 0) {
	//																																for (int t = c; t < c + 1; c++)
	//																																{
	//																																	for (int s = f; s < filteredImageNew.rows; s++)
	//																																	{
	//																																		//cout << "points: " << Point(t, s) << endl;
	//																																		circle(filteredImageNewFilled, Point(t, s), 0, Scalar(255, 0, 255), -1);
	//																																		circle(filteredImageNew_clone, Point(t, s), 0, Scalar(255), -1);
	//																																		currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(s + 1, t);

	//																																		if (currentPixelValueAtCoordinate == 0) {
	//																																			int lineLenght = countNonZero(filteredImageNew_clone);
	//																																			Moments z = moments(filteredImageNew_clone, false);
	//																																			Point p1(z.m10 / z.m00, z.m01 / z.m00);
	//																																			//p1.y is actually X point on the coordinate system
	//																																			//cout << "points: " << Point((p1.y) - 1, p1.x) << endl;
	//																																			morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
	//																																			for (int f = p1.y; f < p1.y + 1; f++)
	//																																			{
	//																																				for (int c = p1.x; c > -1; c--)
	//																																				{
	//																																					//BGR is normal format when using scalar
	//																																					circle(filteredImageNewFilled, Point(c, f), 0, Scalar(255, 0, 0), -1);
	//																																					currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(f, c - 1);


	//																																					if (currentPixelValueAtCoordinate == 0) {
	//																																						for (int t = c; t < c + 1; c++)
	//																																						{
	//																																							for (int s = f; s < filteredImageNew.rows; s++)
	//																																							{
	//																																								//cout << "points: " << Point(t, s) << endl;
	//																																								circle(filteredImageNewFilled, Point(t, s), 0, Scalar(255, 0, 255), -1);
	//																																								circle(filteredImageNew_clone, Point(t, s), 0, Scalar(255), -1);
	//																																								currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(s + 1, t);

	//																																								if (currentPixelValueAtCoordinate == 0) {
	//																																									int lineLenght = countNonZero(filteredImageNew_clone);
	//																																									Moments z = moments(filteredImageNew_clone, false);
	//																																									Point p1(z.m10 / z.m00, z.m01 / z.m00);
	//																																									//p1.y is actually X point on the coordinate system
	//																																									//cout << "points: " << Point((p1.y) - 1, p1.x) << endl;
	//																																									morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
	//																																									for (int f = p1.y; f < p1.y + 1; f++)
	//																																									{
	//																																										for (int c = p1.x; c > -1; c--)
	//																																										{
	//																																											circle(filteredImageNewFilled, Point(c, f), 0, Scalar(255, 0, 0), -1);
	//																																											currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(f, c - 1);
	//																																											Point2i test1(c, f);
	//																																											//cout << "test1: " << test1.x << endl;


	//																																											if (currentPixelValueAtCoordinate == 0) {
	//																																												for (int t = c; t < c + 1; c++)
	//																																												{
	//																																													for (int s = f; s < filteredImageNew.rows; s++)
	//																																													{
	//																																														//cout << "points: " << Point(t, s) << endl;
	//																																														circle(filteredImageNewFilled, Point(t, s), 0, Scalar(0, 0, 255), -1);
	//																																														circle(filteredImageNew_clone, Point(t, s), 0, Scalar(255), -1);
	//																																														currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(s + 1, t);
	//																																														Point2i test2(t, s);

	//																																														/*		cout << "test1.x: " << test1.x << endl;
	//																																																cout << "test2.x: " << test2.x << endl;
	//																																																cout << "test1.y: " << test1.y << endl;
	//																																																cout << "test2.y: " << test2.y << endl;*/

	//																																														if (currentPixelValueAtCoordinate == 0)
	//																																														{
	//																																															if (test1.x == test2.x)
	//																																															{
	//																																																int lineLenght = countNonZero(filteredImageNew_clone);
	//																																																Moments z = moments(filteredImageNew_clone, false);
	//																																																Point p1(z.m10 / z.m00, z.m01 / z.m00);
	//																																																//p1.y is actually X point on the coordinate system
	//																																																//cout << "points: " << Point((p1.y) - 1, p1.x) << endl;
	//																																																morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
	//																																																for (int f = p1.y; f < p1.y + 1; f++)
	//																																																{
	//																																																	for (int c = p1.x; c < filteredImageNew.cols; c++)
	//																																																	{
	//																																																		circle(filteredImageNewFilled, Point(c, f), 0, Scalar(255, 0, 0), -1);
	//																																																		circle(filteredImageNew_clone, Point(c, f), 0, Scalar(255), -1);

	//																																																		currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(f, c + 1);
	//																																																		Point2i test1(c, f);
	//																																																		//cout << "test1: " << test1.x << endl;


	//																																																		if (currentPixelValueAtCoordinate == 0) {
	//																																																			int lineLenght = countNonZero(filteredImageNew_clone);
	//																																																			Moments z = moments(filteredImageNew_clone, false);
	//																																																			Point p1(z.m10 / z.m00, z.m01 / z.m00);
	//																																																			//p1.y is actually X point on the coordinate system
	//																																																			//cout << "points: " << Point((p1.y) - 1, p1.x) << endl;
	//																																																			morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
	//																																																			for (int f = p1.x; f < p1.x + 1; f++)//hatalý
	//																																																			{
	//																																																				for (int c = p1.y; c < filteredImageNew.rows; c++)
	//																																																				{
	//																																																					circle(filteredImageNewFilled, Point(f, c), 0, Scalar(255, 255, 0), -1);
	//																																																					currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(c, f - 1);

	//																																																					if (currentPixelValueAtCoordinate == 0) {
	//																																																						for (int t = c; t < f + 1; f++)
	//																																																						{
	//																																																							for (int s = f; s < filteredImageNew.cols; s++)
	//																																																							{
	//																																																								//cout << "points: " << Point(t, s) << endl;
	//																																																								circle(filteredImageNewFilled, Point(s, t), 0, Scalar(0, 0, 255), -1);
	//																																																								circle(filteredImageNew_clone, Point(s, t), 0, Scalar(255), -1);
	//																																																								currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(t, s + 1);

	//																																																								if (currentPixelValueAtCoordinate == 0) {
	//																																																									int lineLenght = countNonZero(filteredImageNew_clone);
	//																																																									Moments z = moments(filteredImageNew_clone, false);
	//																																																									Point p1(z.m10 / z.m00, z.m01 / z.m00);
	//																																																									//p1.y is actually X point on the coordinate system
	//																																																									//cout << "points: " << Point((p1.y) - 1, p1.x) << endl;
	//																																																									morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
	//																																																									for (int f = p1.x; f < p1.x + 1; f++)
	//																																																									{
	//																																																										for (int c = p1.y; c < filteredImageNew.rows; c++)
	//																																																										{
	//																																																											circle(filteredImageNewFilled, Point(f, c), 0, Scalar(255, 255, 0), -1);
	//																																																											currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(c + 1, f);
	//																																																											if (currentPixelValueAtCoordinate == 0) {
	//																																																												for (int t = c; t < f + 1; f++)
	//																																																												{
	//																																																													for (int s = f; s < filteredImageNew.cols; s++)
	//																																																													{
	//																																																														//cout << "points: " << Point(t, s) << endl;
	//																																																														circle(filteredImageNewFilled, Point(s, t), 0, Scalar(0, 0, 255), -1);
	//																																																														circle(filteredImageNew_clone, Point(s, t), 0, Scalar(255), -1);
	//																																																														currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(t, s + 1);
	//																																																														if (currentPixelValueAtCoordinate == 0) {
	//																																																															int lineLenght = countNonZero(filteredImageNew_clone);
	//																																																															Moments z = moments(filteredImageNew_clone, false);
	//																																																															Point p1(z.m10 / z.m00, z.m01 / z.m00);
	//																																																															//p1.y is actually X point on the coordinate system
	//																																																															//cout << "points: " << Point((p1.y) - 1, p1.x) << endl;
	//																																																															morphologyEx(filteredImageNew_clone, filteredImageNew_clone, MORPH_ERODE, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
	//																																																															for (int f = p1.x; f < p1.x + 1; f++)
	//																																																															{
	//																																																																for (int c = p1.y; c < filteredImageNew.rows; c++)
	//																																																																{
	//																																																																	circle(filteredImageNewFilled, Point(f, c), 0, Scalar(0, 255, 255), -1);
	//																																																																	currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(c + 1, f);
	//																																																																	//cout << "points: " << Point(c, f) << endl;

	//																																																																	if (currentPixelValueAtCoordinate == 0) {

	//																																																																		float leafLength = sqrt((c - bottomStartPoint) * (c - bottomStartPoint) + (f - leftStartPixelPoint) * (f - leftStartPixelPoint));
	//																																																																		cout << "*****************_Calculated Results_(px)***************" << endl;
	//																																																																		cout << "bodyThickness: " << bodyThickness << endl;
	//																																																																		cout << "bodyHeight: " << bodyHeight << endl;
	//																																																																		cout << "leafLength: " << leafLength << endl;
	//																																																																		cout << "********************_Real Results_(mm)******************" << endl;
	//																																																																		cout << "realBodyThickness: " << realBodyThickness << endl;
	//																																																																		cout << "realBodyHeight: " << realBodyHeight << endl;
	//																																																																		cout << "realLeafLength: " << realLeafLength << endl;

	//																																																																		float calibrationValueLL = ((realLeafLength * 100) / (leafLength * 100));
	//																																																																		float calibrationValueBH = ((realBodyHeight * 100) / (bodyHeight * 100));
	//																																																																		float calibrationValueBT = ((realBodyThickness * 100) / (bodyThickness * 100));
	//																																																																		cout << "****************_Calibration Values_******************" << endl;
	//																																																																		cout << "calibrationValueLL: " << (ceil((calibrationValueLL * 1000)) / 1000) << endl;
	//																																																																		cout << "calibrationValueBH: " << (ceil((calibrationValueBH * 1000)) / 1000) << endl;
	//																																																																		cout << "calibrationValueBT: " << (ceil((calibrationValueBT * 1000)) / 1000) << endl;
	//																																																																		cout << "end for now: " << endl;
	//																																																																	}
	//																																																																}
	//																																																															}
	//																																																														}

	//																																																													}
	//																																																												}

	//																																																											}

	//																																																										}
	//																																																									}

	//																																																								}
	//																																																							}
	//																																																						}
	//																																																					}

	//																																																				}
	//																																																			}
	//																																																		}
	//																																																	}
	//																																																}
	//																																															}

	//																																														}

	//																																													}
	//																																												}
	//																																											}
	//																																										}
	//																																									}
	//																																								}
	//																																							}
	//																																						}

	//																																					}
	//																																				}
	//																																			}
	//																																		}
	//																																	}
	//																																}
	//																															}
	//																														}
	//																													}
	//																												}
	//																											}
	//																										}
	//																									}
	//																								}
	//																							}
	//																						}

	//																					}
	//																				}
	//																			}
	//																		}
	//																	}
	//																}
	//															}
	//														}

	//													}

	//												}

	//											}
	//										}
	//									}
	//								}
	//							}
	//						}
	//						//currentPixelValueAtCoordinate = filteredImageNew.at<uchar>(m, l);
	//					}
	//				}

	//				////for forwarding to the left from the current collapsed pixel
	//				//for (int j = x; j < x + 1; j++)
	//				//{
	//				//	for (int k = y; k < filteredImageNew.cols; k++)
	//				//	{

	//				//	}
	//				//}
	//			}
	//		}
	//	}

	//}  
#endif // !
