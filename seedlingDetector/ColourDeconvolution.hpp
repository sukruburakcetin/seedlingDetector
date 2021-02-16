#pragma once
#ifndef COLOURDECONVOLUTION_HPP
#define COLOURDECONVOLUTION_HPP

#include <opencv2/core.hpp>
#include <cstring>

class ColourDeconvolution
{
public:
	ColourDeconvolution();
	cv::Mat process(cv::Mat I, const std::string stainingType, int vector01 = 650, int vector02 = 704, int vector03 = 286, int vector11 = 268, int vector12 = 570, int vector13 = 776) const;
	void setCustomMODs(const std::vector<double> customMODx, const std::vector<double> customMODy, const std::vector<double> customMODz)
	{
		this->customMODx = customMODx;
		this->customMODy = customMODy;
		this->customMODz = customMODz;
	}

private:
	cv::Mat normalizeStaining(const std::string stainingType, int vector01 = 650, int vector02 = 704, int vector03 = 286, int vector11 = 268, int vector12 = 570, int vector13 = 776) const;
	std::vector<double> customMODx;
	std::vector<double> customMODy;
	std::vector<double> customMODz;
};

#endif // COLOURDECONVOLUTION_HPP
