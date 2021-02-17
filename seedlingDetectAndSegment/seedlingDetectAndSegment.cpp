// seedlingDetectAndSegment.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <seedlingDetector.cpp>
using namespace cv;
using namespace std;


int main()
{
    std::cout << "Hello World!\n";

	string image_path = "C:/test_image_seedling.png";



	Mat src = imread(image_path, IMREAD_COLOR);
	Mat dst;
	


    seedlingDetectorPreferences prefs;

    const seedlingDetectorResult res = seedlingDetector(src, dst, prefs);

	//imshow("Display window", dst);
	//int k = waitKey(0); // Wait for a keystroke in the window


}


