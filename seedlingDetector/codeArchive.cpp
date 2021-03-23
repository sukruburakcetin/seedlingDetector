#include "codeArchive.h"
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