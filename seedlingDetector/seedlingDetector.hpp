#include <opencv2/core/mat.hpp>

class seedlingDetectorResult
{

public:
	virtual ~seedlingDetectorResult() = default;

	int seedlingCount = 0;
};

class seedlingDetectorPreferences
{

public:
	virtual ~seedlingDetectorPreferences() = default;



	int vector01 = 650;
	int vector02 = 704;
	int vector03 = 286;
	int vector11 = 268;
	int vector12 = 570;
	int vector13 = 776;


};

seedlingDetectorResult seedlingDetector(cv::Mat& src, cv::Mat& dst, const seedlingDetectorPreferences& prefs);