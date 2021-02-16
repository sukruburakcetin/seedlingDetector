
// seedlingDetector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "ColourDeconvolution.hpp"
#include "seedlingDetector.hpp"
using namespace std;


using namespace cv;

seedlingDetectorResult seedlingDetector(cv::Mat& src, cv::Mat& dst, const seedlingDetectorPreferences& prefs)
{

	seedlingDetectorResult result;

	

	Mat channels[3]; //0: H, 1: DAB, 2: not used
	ColourDeconvolution cd;
	split(cd.process(src, "H DAB", prefs.vector01, prefs.vector02, prefs.vector03, prefs.vector11, prefs.vector12, prefs.vector13), channels);
	channels[2].release();
	channels[2] = NULL;
	dst = channels[1].clone();

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
