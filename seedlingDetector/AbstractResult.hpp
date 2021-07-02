#pragma once



#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include "iostream"


class AbstractResult
{
public:
	virtual ~AbstractResult() = default;

	virtual rapidjson::Document getJsonDoc();
	double GetFloatPrecision(double value, double precision) const;
};

inline rapidjson::Document AbstractResult::getJsonDoc()
{
	std::cout << "Joined to parent" << std::endl;
	return NULL;
}

inline double AbstractResult::GetFloatPrecision(double value, double precision) const
{
	return (floor((value * pow(10, precision) + 0.5)) / pow(10, precision));
}
