#include <torch/script.h> // One-stop header.
#include "fstream"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace std;
using namespace cv;


int scaleWidth = 256;
int scaleHeight = 256;
/// <summary>
/// GPU2CPU conversation is applied for now*
/// torch converted from gpu to cpu
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
int main(int argc, const char* argv[])
{
	Mat image;
	image = imread("train_458.png", CV_LOAD_IMAGE_COLOR);

	//#pragma region resizing if it is needed
//	Mat imageResized = Mat::zeros(image.size(), CV_8UC3);
//
//	cv::resize(image, imageResized, Size(scaleWidth, scaleHeight), INTER_LINEAR);
//
//	image = imageResized.clone();
//
//	imwrite("256x256.png", imageResized);
//#pragma endregion

	//we have to split the interleaved channels
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

	// Deserialize the ScriptModule from a file using torch::jit::load().
	torch::jit::script::Module module;
	module = torch::jit::load("seedling_segmentation.pt");
	module.to(torch::kCPU);

	torch::Tensor tensor_image = torch::from_blob(channelsConcatenatedFloat.data, at::IntList(dims), options);
	tensor_image = tensor_image.toType(torch::kFloat);

	std::ofstream file;
	file.open("tensor_image.txt");
	file << tensor_image;
	file.close();

	torch::Tensor result = module.forward({ tensor_image.to(torch::kCPU) }).toTensor();

	std::ofstream file2;
	file2.open("result.txt");

	file2 << result;

	file2.close();

	result = result.detach().squeeze().cpu();
	result = torch::sigmoid(result);

	cv::Mat img_out(image.rows, image.cols, CV_32F, result.data_ptr<float>());

	//img_out = img_out * 255.0;
	img_out = img_out > 0.5;

	cv::imwrite("_result.png", img_out);
}
