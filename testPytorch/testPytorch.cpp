// testPytorch.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <torch/script.h> // One-stop header.
#include "fstream"
#include <memory>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <filesystem>
using namespace std;


using namespace cv;
namespace fs = std::experimental::filesystem;
int main()
{
	//int scaleWidth = 224, scaleHeight = 224;

	int countTP = 0, countFN = 0, countFP = 0, countTN = 0;
	int mitosis_count = 0, not_mitosis_count = 0;
	//Deserialize the ScriptModule from a file using torch::jit::load().
	torch::jit::script::Module module = torch::jit::load("C:/speech_commands_model.pt");
	module.to(torch::kCUDA);
	std::string path = "C:/datasetPatlicanClassifiedTest/seed_1/";
	

	vector<String> filenames;

	// Get all jpg in the folder
	cv::glob("C:/datasetPatlicanClassifiedTest/seed_1/*.jpeg", filenames);


	for (size_t i = 0; i < filenames.size(); i++) {
		
		Mat image = imread(filenames[i], CV_LOAD_IMAGE_COLOR);

		//Mat imageResized = Mat::zeros(image.size(), CV_8UC3);

		//cv::resize(image, imageResized, Size(scaleWidth, scaleHeight), INTER_LINEAR);

		//image = imageResized.clone();

		//imwrite("100x100.png", imageResized);

		//we have to split the interleaved channels
		cv::Mat bgr[3]; // destination array
		cv::split(image, bgr);
		cv::Mat channelsConcatenated;
		vconcat(bgr[2], bgr[1], channelsConcatenated);
		vconcat(channelsConcatenated, bgr[0], channelsConcatenated);

		cv::Mat channelsConcatenatedFloat, temp;
		channelsConcatenated.convertTo(channelsConcatenatedFloat, CV_32FC3, 1 / 255.0);


		//pleomorp
		subtract(channelsConcatenatedFloat, Scalar(0.485, 0.456, 0.406), temp);


		divide(temp, Scalar(0.229, 0.224, 0.225), channelsConcatenatedFloat);


		//subtract(channelsConcatenatedFloat, Scalar(0.5, 0.5, 0.5), temp);


		//divide(temp, Scalar(0.5, 0.5, 0.5), channelsConcatenatedFloat);



		std::vector<int64_t> dims{ 1, static_cast<int64_t>(image.channels()),
			static_cast<int64_t>(image.rows),
			static_cast<int64_t>(image.cols) };

		torch::TensorOptions options(torch::kFloat);

		torch::Tensor tensor_image = torch::from_blob(channelsConcatenatedFloat.data, at::IntList(dims), options);
		tensor_image = tensor_image.toType(torch::kFloat);

		//std::ofstream outfile2;
		//outfile2.open("tensor_image_tensors_yeni_s_a.txt", std::ios_base::app);//std::ios_base::app
		//outfile2 << tensor_image;

		torch::Tensor result = module.forward({ tensor_image.to(torch::kCUDA) }).toTensor();

		auto results = result.sort(-1, true);
		auto softmaxs = std::get<0>(results)[0].softmax(0);
		auto indexs = std::get<1>(results)[0];

		auto idx = indexs[0].item<int>();
		

		//if (idx == 0){
		//	countTP++;
		//	//mitosis_count++;
		//}
		//else
		//{
		//	countFN++;
		//	//not_mitosis_count++;
		//}

		if (idx == 1)
			countTN++;
		else
			countFP++;

		//std::cout << "countTP: "  << countTP << std::endl;
		//std::cout << "countFN: "  << countFN << std::endl;

		//std::cout << "countTN: " << countTN << std::endl;
		//std::cout << "countFP: " << countFP << std::endl;

	}

	/*std::cout << "countTP: " << countTP << std::endl;
	std::cout << "countFN: " << countFN << std::endl;*/

	std::cout << "countTN: " << countTN << std::endl;
	std::cout << "countFP: " << countFP << std::endl;

	//std::cout << "mitosis_count: " << mitosis_count << std::endl;
	//std::cout << "not_mitosis_count: " << not_mitosis_count << std::endl;
}
