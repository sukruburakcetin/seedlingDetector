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
	seedlingDetectorResult finalResult;
	bool continueTocalculateScore = false;
	std::vector<cv::Mat> imageVec;
	int index = 0;
	fs::path p("C:\\Users\\\HTG_SOFTWARE\\Desktop\\mixed\\");
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
			const seedlingDetectorResult res = seedlingDetector(src, dst, prefs, continueTocalculateScore);


			if (continueTocalculateScore == true ) {
				finalResult.totalSeedlingCount += res.roiResults[index].seedlingCount;
				finalResult.totalBodyHeight += res.roiResults[index].bodyHeight;
				finalResult.totalBodyThickness += res.roiResults[index].bodyThickness;
				finalResult.totalLeafLength += res.roiResults[index].leafLength;
				finalResult.averageTotalBodyHeight = (finalResult.totalBodyHeight) / (finalResult.totalSeedlingCount);
				finalResult.averageTotalBodyThickness = (finalResult.totalBodyThickness) / (finalResult.totalSeedlingCount);
				finalResult.averageTotalLeafLength = (finalResult.totalLeafLength) / (finalResult.totalSeedlingCount);
			}
			else
			{
				cout << "Skipping file because there is no seedling for the analysis! " << endl;
			}
		}
	}
	cout << "totalSeedlingCount: " << finalResult.totalSeedlingCount << endl;
	cout << "totalBodyHeight: " << finalResult.totalBodyHeight << endl;
	cout << "totalBodyThickness: " << finalResult.totalBodyThickness << endl;
	cout << "totalLeafLength: " << finalResult.totalLeafLength << endl;

	cout << "averageTotalBodyHeight: " << finalResult.averageTotalBodyHeight << endl;
	cout << "averageTotalBodyThickness: " << finalResult.averageTotalBodyThickness << endl;
	cout << "averageTotalLeafLength: " << finalResult.averageTotalLeafLength << endl;
	//string image_path = "C:/IMG_3891.png";
	//string image_path = "C:/4.png";
	//Mat src = imread(image_path, IMREAD_COLOR);
}


