#include <rapidjson/document.h>
#include "AbstractResult.hpp"


struct seedlingRoiResult
{
	int bodyThickness;
	int bodyHeight;
	int leafLength;
	int seedlingCount;
	seedlingRoiResult(int bodyThickness = 0, int bodyHeight = 0, int leafLength = 0, int seedlingCount = 0) :
		bodyThickness(bodyThickness), bodyHeight(bodyHeight), leafLength(leafLength), seedlingCount(seedlingCount)
	{
		//std::cerr << "Roi " << " x=" << x << " y=" << y << " h=" << h << " w=" << w << std::endl;
	}
};


class seedlingDetectorResult : public AbstractResult
{
public:
	virtual ~seedlingDetectorResult() = default;

	int seedlingCount = 0;
	std::vector<seedlingRoiResult> roiResults;
	int totalBodyHeight;
	int totalBodyThickness;
	int totalLeafLength;
	int totalSeedlingCount;
	int averageTotalBodyHeight;
	int averageTotalBodyThickness;
	int averageTotalLeafLength;

	rapidjson::Document getJsonDoc() override
	{
		rapidjson::Document res;
		res.SetObject();
		rapidjson::Document::AllocatorType& allocator = res.GetAllocator();
		rapidjson::Value rowModelList_array(rapidjson::kArrayType);

		for (int i = 0; i < roiResults.size(); i++)
		{
			const seedlingRoiResult roiRes = roiResults[i];

			totalBodyHeight = roiRes.bodyHeight;
			totalBodyThickness = roiRes.bodyThickness;
			totalLeafLength = roiRes.leafLength;
			totalSeedlingCount = roiRes.seedlingCount;

			rapidjson::Value resultObject(rapidjson::kObjectType);

			resultObject.AddMember("bodyHeight", roiRes.bodyHeight, allocator);
			resultObject.AddMember("bodyThickness", roiRes.bodyThickness, allocator);
			resultObject.AddMember("leafLength", roiRes.leafLength, allocator);
			resultObject.AddMember("seedlingCount", roiRes.seedlingCount, allocator);

			rowModelList_array.PushBack(resultObject, allocator);
		}
		res.AddMember("rowModelList", rowModelList_array, allocator);

		averageTotalBodyHeight = double(totalBodyHeight) / double(totalSeedlingCount);
		averageTotalBodyThickness = double(totalBodyThickness) / double(totalSeedlingCount);
		averageTotalLeafLength = double(totalLeafLength) / double(totalSeedlingCount);

		res.AddMember("averageTotalBodyHeight", averageTotalBodyHeight, allocator);
		res.AddMember("averageTotalBodyThickness", averageTotalBodyThickness, allocator);
		res.AddMember("averageTotalLeafLength", averageTotalLeafLength, allocator);
		return res;
	}

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