// seedlingDetectAndSegment.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <seedlingDetector.hpp>
using namespace cv;
using namespace std;


int main()
{
    std::cout << "Seedling Classifier\n";

	string image_path = "C:/test_image_seedling.png";

	Mat src = imread(image_path, IMREAD_COLOR);
	Mat dst;

    seedlingDetectorPreferences prefs;
    const seedlingDetectorResult res = seedlingDetector(src, dst, prefs);
}


