#include <torch/script.h> // One-stop header.
#include "fstream"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace std;
using namespace cv;


int scaleWidth = 256;
int scaleHeight = 256;

int main(int argc, const char* argv[])
{
	Mat image;
	image = imread("train_458.png", CV_LOAD_IMAGE_COLOR);


	cv::Mat bgr[3]; // destination array
	cv::split(image, bgr);
	cv::Mat channelsConcatenated;
	vconcat(bgr[2], bgr[1], channelsConcatenated);
	vconcat(channelsConcatenated, bgr[0], channelsConcatenated);

	cv::Mat channelsConcatenatedFloat;
	channelsConcatenated.convertTo(channelsConcatenatedFloat, CV_32FC3, 1 / 255.0);


	std::vector<int64_t> dims{ 1, static_cast<int64_t>(image.channels()),
		static_cast<int64_t>(image.rows),
		static_cast<int64_t>(image.cols) };

	torch::TensorOptions options(torch::kFloat);


	torch::Tensor tensor_image = torch::from_blob(channelsConcatenatedFloat.data, at::IntList(dims), options);
	tensor_image = tensor_image.toType(torch::kFloat);


	torch::jit::script::Module module;
	module = torch::jit::load("seedling_segmentation.pt");
	module.to(torch::kCUDA);
	
	torch::Tensor result = module.forward({ tensor_image.to(torch::kCUDA) }).toTensor();

	result = result.squeeze().detach().permute({ 1, 2, 0 });
	result = result.mul(255).clamp(0, 255).to(torch::kU8);
	result = result.to(torch::kCPU);
	cv::Mat img_out(image.rows, image.cols, CV_8UC1);
	std::memcpy((void*)img_out.data, result.data_ptr(), sizeof(torch::kU8) * result.numel());

	cout << "asd << " << endl;

	//222222222222222222222222222222222222222222222222222222222222
	
	//cv::Mat bgr[3]; // destination array
	//cv::split(image, bgr);
	//cv::Mat channelsConcatenated;
	//vconcat(bgr[2], bgr[1], channelsConcatenated);
	//vconcat(channelsConcatenated, bgr[0], channelsConcatenated);

	//cv::Mat channelsConcatenatedFloat;
	//channelsConcatenated.convertTo(channelsConcatenatedFloat, CV_32FC3, 1 / 255.0);

	//std::vector<int64_t> dims{ 1, static_cast<int64_t>(image.channels()),
	//	static_cast<int64_t>(image.rows),
	//	static_cast<int64_t>(image.cols) };

	//torch::TensorOptions options(torch::kFloat);

	//torch::Tensor tensor_image = torch::from_blob(channelsConcatenated.data, { channelsConcatenated.rows, channelsConcatenated.cols,3 }, torch::kByte).to(torch::kCUDA);//hwc
	//																																						//ToTensor
	//tensor_image = tensor_image.permute({ 2,0,1 });//chw
	//tensor_image = tensor_image.toType(torch::kFloat);
	//tensor_image = tensor_image.div(255.0);

	//////normalize

	////tensor_image[0] = tensor_image[0].sub_(0.5).div_(0.5);
	////tensor_image[1] = tensor_image[1].sub_(0.5).div_(0.5);
	////tensor_image[2] = tensor_image[2].sub_(0.5).div_(0.5);



	//torch::jit::script::Module module;
	//module = torch::jit::load("seedling_segmentation.pt");
	//module.to(torch::kCUDA);

	//tensor_image = tensor_image.unsqueeze(0);
	//std::vector<torch::jit::IValue> inputs;
	//inputs.push_back(tensor_image);

	//torch::Tensor result = module.forward({ inputs }).toTensor();

	////result = torch::softmax(result, 1);

	//result = result.detach().squeeze().cpu();

	//cv::Mat img_out(image.rows, image.cols, CV_32F, result.data_ptr<float>());

	//img_out = img_out > 0.5;

	//cv::imwrite("_result.png", img_out);


///3333333333333333333333333333333333333333333333333333333333
	//#pragma region resizing if it is needed
	//	Mat imageResized = Mat::zeros(image.size(), CV_8UC3);
	//
	//	cv::resize(image, imageResized, Size(scaleWidth, scaleHeight), INTER_LINEAR);
	//
	//	image = imageResized.clone();
	//
	//	imwrite("256x256.png", imageResized);
	//#pragma endregion

	//	//we have to split the interleaved channels
	//	cv::Mat bgr[3]; // destination array
	//	cv::split(image, bgr);
	//	cv::Mat channelsConcatenated;
	//	vconcat(bgr[2], bgr[1], channelsConcatenated);
	//	vconcat(channelsConcatenated, bgr[0], channelsConcatenated);

	//	cv::Mat channelsConcatenatedFloat;
	//	channelsConcatenated.convertTo(channelsConcatenatedFloat, CV_32FC3, 1 / 255.0);

	//	std::vector<int64_t> dims{ 1, static_cast<int64_t>(image.channels()),
	//		static_cast<int64_t>(image.rows),
	//		static_cast<int64_t>(image.cols) };

	//	torch::TensorOptions options(torch::kFloat);

	//	// Deserialize the ScriptModule from a file using torch::jit::load().
	//	torch::jit::script::Module module;
	//	module = torch::jit::load("seedling_segmentation.pt");
	//	module.to(torch::kCUDA);

	//	torch::Tensor tensor_image = torch::from_blob(channelsConcatenatedFloat.data, at::IntList(dims), options);
	//	tensor_image = tensor_image.toType(torch::kFloat);

	//	std::ofstream file;
	//	file.open("tensor_image.txt");
	//	file << tensor_image;
	//	file.close();

	//	torch::Tensor result = module.forward({ tensor_image.to(torch::kCUDA) }).toTensor();

	//	std::ofstream file2;
	//	file2.open("result.txt");

	//	file2 << result;

	//	file2.close();

	//	result = result.detach().squeeze().cpu();
	//	result = torch::sigmoid(result);

	//	cv::Mat img_out(image.rows, image.cols, CV_32F, result.data_ptr<float>());

	//	//img_out = img_out * 255.0;
	//	img_out = img_out > 0.5;

	//	cv::imwrite("_result.png", img_out);
}
