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

//void determineThresholdSeedlingToLeaf(int leftStart2, int bottomStartRect2, int heightSeedling, int currentValue2, bool rowCheckIsDone = false, bool finalWhitePixelRight2 = false, int sumWhitePixelToTheRight2, cv::Mat filteredImageNew, cv::Mat filteredImageNew_3D, int currentValueRight, int currentValueLeft, bool finalWhitePixelLeft, int sumWhitePixelToTheLeft2 = 0, int rowThicknessWhenCollapsed, int nextSeedlingStartPoint, int epsilon, int seedlingTickness);
