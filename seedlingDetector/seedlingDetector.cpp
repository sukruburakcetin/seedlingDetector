
// seedlingDetector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "ColourDeconvolution.hpp"
#include "seedlingDetector.hpp"
#include <opencv2/imgproc.hpp>
#include "AutoThreshold.hpp"
#include "ParticleAnalyzer.hpp"
#include "WatershedCustom.hpp"
using namespace std;


using namespace cv;

seedlingDetectorResult seedlingDetector(cv::Mat& src, cv::Mat& dst, const seedlingDetectorPreferences& prefs)
{
	Mat hsvColorSpace;
	Mat rgbColorSpace;
	Mat bgrColorSpace;
	Mat labColorSpace;

	seedlingDetectorResult result;

	cvtColor(src, rgbColorSpace, COLOR_RGBA2RGB);
	cvtColor(src, bgrColorSpace, COLOR_RGBA2BGR);
	cvtColor(rgbColorSpace, hsvColorSpace, COLOR_RGB2HSV);
	cvtColor(rgbColorSpace, labColorSpace, COLOR_RGB2Lab);

	Mat hsv[3];
	split(hsvColorSpace, hsv);
	Mat hsv_channel1 = hsv[0];
	Mat hsv_channel2 = hsv[1];
	Mat hsv_channel3 = hsv[2];

	Mat rgb[3];
	split(rgbColorSpace, rgb);
	Mat rgb_channel1 = rgb[0];
	Mat rgb_channel2 = rgb[1];
	Mat rgb_channel3 = rgb[2];

	Mat bgr[3];
	split(bgrColorSpace, bgr);
	Mat bgr_channel1 = bgr[0];
	Mat bgr_channel2 = bgr[1];
	Mat bgr_channel3 = bgr[2];

	Mat lab[3];
	split(labColorSpace, lab);
	Mat lab_channel1 = lab[0];
	Mat lab_channel2 = lab[1];
	Mat lab_channel3 = lab[2];


	Mat channels[3]; //0: H, 1: DAB, 2: not used
	ColourDeconvolution cd;
	split(cd.process(src, "H DAB", prefs.vector01, prefs.vector02, prefs.vector03, prefs.vector11, prefs.vector12, prefs.vector13), channels);
	channels[2].release();
	channels[2] = NULL;
	dst = channels[1].clone();

	Mat thresholded_dst = lab_channel1.clone();

	//autoThreshold(thresholded_dst, ThresholdMethod::Default);
	//adaptiveThreshold(dst, thresholded_dst, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 51, 0);
	//adaptiveThreshold(dst, thresholded_dst, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 171, 0);

	autoThreshold(thresholded_dst, ThresholdMethod::Default);
	thresholded_dst = ~thresholded_dst;
	watershedProcess(thresholded_dst, thresholded_dst, 41);

	Mat labels, stats, centroids, doubleStats;
	int count = analyzeParticles(thresholded_dst, labels, stats, centroids, doubleStats, ParticleAnalyzer::FOUR_CONNECTED | ParticleAnalyzer::EXCLUDE_EDGE_PARTICLES, 0);

	//imshow("Display window", dst);
	//int k1 = waitKey(0); // Wait for a keystroke in the window








  //imshow("Display window", img);
  //int k = waitKey(0); // Wait for a keystroke in the window
  //if (k == 's')
  //{
  //    imwrite("C:/test_image_seedling_saved.jpeg", img);
  //}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
