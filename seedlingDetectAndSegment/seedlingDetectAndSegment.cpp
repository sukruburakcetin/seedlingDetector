#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <seedlingDetector.hpp>
#include <boost/filesystem.hpp>

using namespace cv;
using namespace std;
namespace fs = boost::filesystem;

int main()
{
    std::cout << "------------------------------- Seedling Classifier ----------------------------------\n";

	std::vector<cv::Mat> imageVec;
	fs::path p("C:\\Users\\HTG_SOFTWARE\\Desktop\\mixed\\");
	fs::directory_iterator end_itr;
	// cycle through the directory
	for (fs::directory_iterator itr(p); itr != end_itr; ++itr) {
		// If it's not a directory, list it. If you want to list directories too, just remove this check.
		if (fs::is_regular_file(itr->path())) {
			if (fs::is_regular_file(itr->path())) {
				cv::Mat img;
				img = cv::imread(itr->path().string());
				if (img.data) imageVec.push_back(img);
			}
			// assign current file name to current_file and echo it out to the console.
			std::string current_file = itr->path().string();
			std::cout << current_file << std::endl;
			Mat src = imread(current_file, IMREAD_COLOR);
			Mat dst;

			seedlingDetectorPreferences prefs;
			const seedlingDetectorResult res = seedlingDetector(src, dst, prefs);
		}
	}
	//string image_path = "C:/IMG_3891.png";
	//string image_path = "C:/4.png";
	//Mat src = imread(image_path, IMREAD_COLOR);
}


