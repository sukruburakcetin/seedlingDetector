#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <seedlingDetector.hpp>
#include <boost/filesystem.hpp>

using namespace cv;
using namespace std;
namespace fs = boost::filesystem;

float specimenRealBodyThickness = 0.69;
float specimenRealBodyHeight = 30.00;
float specimenRealLeafLength = 20.00;

float specimenPxBodyThickness = 24.00;
float specimenPxBodyHeight = 437.00;
float specimenPxLeafLength = 238.824;

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


			if (continueTocalculateScore == true && res.roiResults[index].leafLength >= 1 && res.roiResults[index].bodyThickness >= 1 && res.roiResults[index].bodyHeight >= 1 && res.roiResults[index].seedlingCount >= 1) {
				finalResult.totalSeedlingCount += res.roiResults[index].seedlingCount;
				finalResult.totalBodyHeight += res.roiResults[index].bodyHeight;
				finalResult.totalBodyThickness += res.roiResults[index].bodyThickness;
				finalResult.totalLeafLength += res.roiResults[index].leafLength;
			}
			else
			{
				cout << "Skipping file because there is bad argument for the analysis! " << endl;
				cout << "" << endl;
			}
		}
	}
	
	finalResult.averageTotalBodyHeight = (finalResult.totalBodyHeight) / (finalResult.totalSeedlingCount);
	finalResult.averageTotalBodyThickness = (finalResult.totalBodyThickness) / (finalResult.totalSeedlingCount);
	finalResult.averageTotalLeafLength = (finalResult.totalLeafLength) / (finalResult.totalSeedlingCount);

	cout << "All files are examined, calculating the score..." << endl;
	cout << "" << endl;
	cout << "********************Score Screen******************" << endl;
	cout << "" << endl;
	
	cout << "totalSeedlingCount: " << finalResult.totalSeedlingCount << endl;
	cout << "totalBodyHeight: " << finalResult.totalBodyHeight << endl;
	cout << "totalBodyThickness: " << finalResult.totalBodyThickness << endl;
	cout << "totalLeafLength: " << finalResult.totalLeafLength << endl;
	
	cout << "********************_Specimen Real Results_(mm)******************" << endl;
	cout << "specimenRealBodyThickness: " << specimenRealBodyThickness << endl;
	cout << "specimenRealBodyHeight: " << specimenRealBodyHeight << endl;
	cout << "specimenRealLeafLength: " << specimenRealLeafLength << endl;

	cout << "********************_Specimen Real Results_(mm)******************" << endl;
	cout << "specimenPxBodyThickness: " << specimenPxBodyThickness << endl;
	cout << "specimenPxBodyHeight: " << specimenPxBodyHeight << endl;
	cout << "specimenPxLeafLength: " << specimenPxLeafLength << endl;
	
	cout << "****************_Average Values_(px)******************" << endl;
	cout << "averageTotalBodyHeight: " << finalResult.averageTotalBodyHeight << endl;
	cout << "averageTotalBodyThickness: " << finalResult.averageTotalBodyThickness << endl;
	cout << "averageTotalLeafLength: " << finalResult.averageTotalLeafLength << endl;

	float calibrationValueLL = ((specimenRealLeafLength * 1000) / (specimenPxLeafLength * 1000));
	float calibrationValueBH = ((specimenRealBodyHeight * 1000) / (specimenPxBodyHeight * 1000));
	float calibrationValueBT = ((specimenRealBodyThickness * 1000) / (specimenPxBodyThickness * 1000));
	
	cout << "****************_Calibration Values_(mm)******************" << endl;
	cout << "calibrationValueLeafLength: " << (ceil((calibrationValueLL * 100)) / 100) << endl;
	cout << "calibrationValueBodyHeight: " << (ceil((calibrationValueBH * 100)) / 100) << endl;
	cout << "calibrationValueBodyThickness: " << (ceil((calibrationValueBT * 100)) / 100) << endl;

	cout << "****************_Calibrated End Results_(mm)******************" << endl;
	cout << "EndResult_LeafLength: " << (((float)finalResult.averageTotalLeafLength) * (ceil((calibrationValueLL * 100)) / 100)) << endl;
	cout << "EndResult_TotalBodyHeight: " << (((float)finalResult.averageTotalBodyHeight) * (ceil((calibrationValueBH * 100)) / 100)) << endl;
	cout << "EndResult_TotalBodyThickness: " << (((float)finalResult.averageTotalBodyThickness) * (ceil((calibrationValueBT * 100)) / 100)) << endl;

	//string image_path = "C:/IMG_3891.png";
	//string image_path = "C:/4.png";
	//Mat src = imread(image_path, IMREAD_COLOR);
}


