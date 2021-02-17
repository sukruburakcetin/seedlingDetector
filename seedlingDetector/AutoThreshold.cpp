#include "AutoThreshold.hpp"
#include "Helpers.hpp"
#include "opencv2/opencv.hpp"

using namespace cv;

int autoThreshold(Mat& src, const ThresholdMethod method, const bool darkBackground)
{
	CV_Assert(src.type() == CV_8UC1);
	const int thresholdValue = getThreshold(src, method);
	double lower, upper;
	if (darkBackground) {
		lower = thresholdValue + 1;
		upper = 255.0;
	}
	else {
		lower = 0.0;
		upper = thresholdValue;
	}
	if (lower > 255) lower = 255;
	inRange(src, lower, upper, src);
	return thresholdValue;
}

int getThreshold(cv::Mat& src, const ThresholdMethod method)
{
	CV_Assert(src.type() == CV_8UC1);
	Mat hist;
	getHistogramGrayscale(src, hist);
	int threshold = 0;
	switch (method) {
	case ThresholdMethod::Default:
		threshold = defaultIsoData(hist);
		break;
	case ThresholdMethod::Huang:
		threshold = Huang(hist);
		break;
	case ThresholdMethod::Shanbhag:
		threshold = Shanbhag(hist);
		break;
	case ThresholdMethod::Li:
		threshold = Li(hist);
		break;
	case ThresholdMethod::Minimum:
		threshold = Minimum(hist);
		break;
	case ThresholdMethod::MaxEntropy:
		threshold = MaxEntropy(hist);
		break;
	case ThresholdMethod::Otsu:
		threshold = Otsu(hist);
	}
	if (threshold == -1) threshold = 0;
	return threshold;
}

int defaultIsoData(cv::Mat& hist)
{
	const int n = 256;
	float* data = hist.ptr<float>(0);
	int mode = 0, maxCount = 0;
	for (int i = 0; i < n; i++) {
		if (data[i] > maxCount) {
			maxCount = static_cast<int>(data[i]);
			mode = i;
		}
	}
	int maxCount2 = 0;
	for (int i = 0; i < n; i++) {
		if ((data[i] > maxCount2) && (i != mode))
			maxCount2 = static_cast<int>(data[i]);
	}
	int hmax = maxCount;
	if ((hmax > (maxCount2 * 2)) && (maxCount2 != 0)) {
		hmax = static_cast<int>(maxCount2 * 1.5);
		data[mode] = static_cast<float>(hmax);
	}
	return IJIsoData(hist);
}

int IJIsoData(cv::Mat& hist)
{
	int level;
	const int maxValue = 255;
	double result, sum2, sum3, sum4;
	float* data = hist.ptr<float>(0);
	const int count0 = data[0];
	data[0] = 0; //set to zero so erased areas aren't included
	const int countMax = data[maxValue];
	data[maxValue] = 0;
	int minv = 0;
	while ((data[minv] == 0) && (minv < maxValue))
		minv++;
	int maxv = maxValue;
	while ((data[maxv] == 0) && (maxv > 0))
		maxv--;
	if (minv >= maxv) {
		data[0] = static_cast<float>(count0);
		data[maxValue] = static_cast<float>(countMax);
		level = 128;
		return level;
	}
	int movingIndex = minv;
	do {
		double sum1 = sum2 = sum3 = sum4 = 0.0;
		for (int i = minv; i <= movingIndex; i++) {
			sum1 += static_cast<double>(i) * data[i];
			sum2 += data[i];
		}
		for (int i = (movingIndex + 1); i <= maxv; i++) {
			sum3 += static_cast<double>(i) * data[i];
			sum4 += data[i];
		}
		result = (sum1 / sum2 + sum3 / sum4) / 2.0;
		movingIndex++;
	} while ((movingIndex + 1) <= result && movingIndex < maxv - 1);
	data[0] = static_cast<float>(count0);
	data[maxValue] = static_cast<float>(countMax);
	level = static_cast<int>(round(result));
	return level;
}

int Huang(cv::Mat& hist)
{
	int ih;
	double num_pix;
	double mu_x;
	float* data = hist.ptr<float>();

	/* Determine the first non-zero bin */
	int first_bin = 0;
	for (ih = 0; ih < 256; ih++) {
		if (data[ih] != 0) {
			first_bin = ih;
			break;
		}
	}

	/* Determine the last non-zero bin */
	int last_bin = 255;
	for (ih = 255; ih >= first_bin; ih--) {
		if (data[ih] != 0) {
			last_bin = ih;
			break;
		}
	}
	const double term = 1.0 / static_cast<double>(last_bin - first_bin);
	double mu_0[256];
	double sum_pix = num_pix = 0;
	for (ih = first_bin; ih < 256; ih++) {
		sum_pix += static_cast<double>(ih) * data[ih];
		num_pix += data[ih];
		/* NUM_PIX cannot be zero ! */
		mu_0[ih] = sum_pix / num_pix;
	}

	double mu_1[256];
	sum_pix = num_pix = 0;
	for (ih = last_bin; ih > 0; ih--) {
		sum_pix += static_cast<double>(ih) * data[ih];
		num_pix += data[ih];
		/* NUM_PIX cannot be zero ! */
		mu_1[ih - 1] = sum_pix / static_cast<double>(num_pix);
	}

	/* Determine the threshold that minimizes the fuzzy entropy */
	int threshold = -1;
	double min_ent = DBL_MAX;
	for (int it = 0; it < 256; it++) {
		double ent = 0.0;
		for (ih = 0; ih <= it; ih++) {
			/* Equation (4) in Ref. 1 */
			mu_x = 1.0 / (1.0 + term * abs(ih - mu_0[it]));
			if (!((mu_x < 1e-06) || (mu_x > 0.999999))) {
				/* Equation (6) & (8) in Ref. 1 */
				ent += data[ih] * (-mu_x * log(mu_x) - (1.0 - mu_x) * log(1.0 - mu_x));
			}
		}

		for (ih = it + 1; ih < 256; ih++) {
			/* Equation (4) in Ref. 1 */
			mu_x = 1.0 / (1.0 + term * abs(ih - mu_1[it]));
			if (!((mu_x < 1e-06) || (mu_x > 0.999999))) {
				/* Equation (6) & (8) in Ref. 1 */
				ent += data[ih] * (-mu_x * log(mu_x) - (1.0 - mu_x) * log(1.0 - mu_x));
			}
		}
		/* No need to divide by NUM_ROWS * NUM_COLS * LOG(2) ! */
		if (ent < min_ent) {
			min_ent = ent;
			threshold = it;
		}
	}
	return threshold;
}

int Shanbhag(cv::Mat& hist)
{
	int ih;
	double norm_histo[256]; /* normalized histogram */
	double p1[256]; /* cumulative normalized histogram */
	double p2[256];
	float* data = hist.ptr<float>();

	double total = 0;
	for (ih = 0; ih < 256; ih++)
		total += data[ih];

	for (ih = 0; ih < 256; ih++)
		norm_histo[ih] = data[ih] / total;

	p1[0] = norm_histo[0];
	p2[0] = 1.0 - p1[0];
	for (ih = 1; ih < 256; ih++) {
		p1[ih] = p1[ih - 1] + norm_histo[ih];
		p2[ih] = 1.0 - p1[ih];
	}

	/* Determine the first non-zero bin */
	int first_bin = 0;
	for (ih = 0; ih < 256; ih++) {
		if (!(abs(p1[ih]) < 2.220446049250313E-16)) {
			first_bin = ih;
			break;
		}
	}

	/* Determine the last non-zero bin */
	int last_bin = 255;
	for (ih = 255; ih >= first_bin; ih--) {
		if (!(abs(p2[ih]) < 2.220446049250313E-16)) {
			last_bin = ih;
			break;
		}
	}

	// Calculate the total entropy each gray-level
	// and find the threshold that maximizes it 
	int threshold = -1;
	double min_ent = DBL_MAX;

	for (int it = first_bin; it <= last_bin; it++) {
		/* Entropy of the background pixels */
		double ent_back = 0.0;
		double term = 0.5 / p1[it];
		for (ih = 1; ih <= it; ih++) {
			//0+1?
			ent_back -= norm_histo[ih] * log(1.0 - term * p1[ih - 1]);
		}
		ent_back *= term;

		/* Entropy of the object pixels */
		double ent_obj = 0.0;
		term = 0.5 / p2[it];
		for (ih = it + 1; ih < 256; ih++) {
			ent_obj -= norm_histo[ih] * log(1.0 - term * p2[ih]);
		}
		ent_obj *= term;

		/* Total entropy */
		const double tot_ent = abs(ent_back - ent_obj);

		if (tot_ent < min_ent) {
			min_ent = tot_ent;
			threshold = it;
		}
	}
	return threshold;
}

int Li(cv::Mat& hist)
{
	// Implements Li's Minimum Cross Entropy thresholding method
	// This implementation is based on the iterative version (Ref. 2) of the algorithm.
	// 1) Li C.H. and Lee C.K. (1993) "Minimum Cross Entropy Thresholding" 
	//    Pattern Recognition, 26(4): 617-625
	// 2) Li C.H. and Tam P.K.S. (1998) "An Iterative Algorithm for Minimum 
	//    Cross Entropy Thresholding"Pattern Recognition Letters, 18(8): 771-776
	// 3) Sezgin M. and Sankur B. (2004) "Survey over Image Thresholding 
	//    Techniques and Quantitative Performance Evaluation" Journal of 
	//    Electronic Imaging, 13(1): 146-165 
	//    http://citeseer.ist.psu.edu/sezgin04survey.html
	// Ported to ImageJ plugin by G.Landini from E Celebi's fourier_0.8 routines
	int threshold;
	double old_thresh;
	float* data = hist.ptr<float>();

	const double tolerance = 0.5;
	double num_pixels = 0;
	for (int ih = 0; ih < 256; ih++)
		num_pixels += data[ih];

	/* Calculate the mean gray-level */
	double mean = 0.0;
	for (int ih = 0 + 1; ih < 256; ih++) //0 + 1?
		mean += static_cast<double>(ih) * data[ih];
	mean /= num_pixels;
	/* Initial estimate */
	double new_thresh = mean;

	do {
		old_thresh = new_thresh;
		threshold = static_cast<int>(old_thresh + 0.5); /* range */
		/* Calculate the means of background and object pixels */
		/* Background */
		double sum_back = 0;
		double num_back = 0;
		for (int ih = 0; ih <= threshold; ih++) {
			sum_back += static_cast<double>(ih) * data[ih];
			num_back += data[ih];
		}
		const double mean_back = (num_back == 0 ? 0.0 : (sum_back / static_cast<double>(num_back)));
		/* Object */
		double sum_obj = 0;
		double num_obj = 0;
		for (int ih = threshold + 1; ih < 256; ih++) {
			sum_obj += static_cast<double>(ih) * data[ih];
			num_obj += data[ih];
		}
		const double mean_obj = (num_obj == 0 ? 0.0 : (sum_obj / static_cast<double>(num_obj)));

		/* Calculate the new threshold: Equation (7) in Ref. 2 */
		//new_thresh = simple_round ( ( mean_back - mean_obj ) / ( Math.log ( mean_back ) - Math.log ( mean_obj ) ) );
		//simple_round ( double x ) {
		// return ( int ) ( IS_NEG ( x ) ? x - .5 : x + .5 );
		//}
		//
		//#define IS_NEG( x ) ( ( x ) < -DBL_EPSILON ) 
		//DBL_EPSILON = 2.220446049250313E-16
		const double temp = (mean_back - mean_obj) / (log(mean_back) - log(mean_obj));

		if (temp < -2.220446049250313E-16)
			new_thresh = static_cast<int>(temp - 0.5);
		else
			new_thresh = static_cast<int>(temp + 0.5);
		/*  Stop the iterations when the difference between the
		new and old threshold values is less than the tolerance */
	} while (abs(new_thresh - old_thresh) > tolerance);
	return threshold;
}


bool bimodalTest(double* y)
{
	const int len = 256;
	bool b = false;
	int modes = 0;

	for (int k = 1; k < len - 1; k++) {
		if (y[k - 1] < y[k] && y[k + 1] < y[k]) {
			modes++;
			if (modes > 2)
				return false;
		}
	}
	if (modes == 2)
		b = true;
	return b;
}

int Minimum(cv::Mat& hist)
{
	int* data = hist.ptr<int>();
	int iter = 0;
	int threshold = -1;
	int max = -1;
	double iHisto[256];

	for (int i = 0; i < 256; i++) {
		iHisto[i] = static_cast<double>(data[i]);
		if (data[i] > 0) max = i;
	}
	double tHisto[256];

	while (!bimodalTest(iHisto)) {
		//smooth with a 3 point running mean filter
		for (int i = 1; i < 255; i++)
			tHisto[i] = (iHisto[i - 1] + iHisto[i] + iHisto[i + 1]) / 3.;
		tHisto[0] = (iHisto[0] + iHisto[1]) / 3.; //0 outside
		tHisto[255] = (iHisto[254] + iHisto[255]) / 3.; //0 outside
		memcpy(iHisto, tHisto, 256 * sizeof(double));

		iter++;
		if (iter > 10000) {
			threshold = -1;
			// Minimum Threshold not found after 10000 iterations.
			//std::cout << "iter: " << iter << std::endl;
			return threshold;
		}
	}
	// The threshold is the minimum between the two peaks. modified for 16 bits
	for (int i = 1; i < max; i++) {
		if (iHisto[i - 1] > iHisto[i] && iHisto[i + 1] >= iHisto[i]) {
			threshold = i;
			break;
		}
	}
	//std::cout << "iter: " << iter << std::endl;
	return threshold;
}

int MaxEntropy(cv::Mat& hist)
{
	// Implements Kapur-Sahoo-Wong (Maximum Entropy) thresholding method
	// Kapur J.N., Sahoo P.K., and Wong A.K.C. (1985) "A New Method for
	// Gray-Level Picture Thresholding Using the Entropy of the Histogram"
	// Graphical Models and Image Processing, 29(3): 273-285
	// M. Emre Celebi
	// 06.15.2007
	// Ported to ImageJ plugin by G.Landini from E Celebi's fourier_0.8 routines
	int threshold = -1;
	int ih;
	float* data = hist.ptr<float>();
	double norm_histo[256]; /* normalized histogram */
	double p1[256]; /* cumulative normalized histogram */
	double p2[256];

	double total = 0;
	for (ih = 0; ih < 256; ih++)
		total += data[ih];

	for (ih = 0; ih < 256; ih++)
		norm_histo[ih] = data[ih] / total;

	p1[0] = norm_histo[0];
	p2[0] = 1.0 - p1[0];
	for (ih = 1; ih < 256; ih++) {
		p1[ih] = p1[ih - 1] + norm_histo[ih];
		p2[ih] = 1.0 - p1[ih];
	}

	/* Determine the first non-zero bin */
	int first_bin = 0;
	for (ih = 0; ih < 256; ih++) {
		if (!(abs(p1[ih]) < 2.220446049250313E-16)) {
			first_bin = ih;
			break;
		}
	}

	/* Determine the last non-zero bin */
	int last_bin = 255;
	for (ih = 255; ih >= first_bin; ih--) {
		if (!(abs(p2[ih]) < 2.220446049250313E-16)) {
			last_bin = ih;
			break;
		}
	}

	// Calculate the total entropy each gray-level
	// and find the threshold that maximizes it 
	double max_ent = DBL_MIN;

	for (int it = first_bin; it <= last_bin; it++) {
		/* Entropy of the background pixels */
		double ent_back = 0.0;
		for (ih = 0; ih <= it; ih++) {
			if (data[ih] != 0) {
				ent_back -= (norm_histo[ih] / p1[it]) * log(norm_histo[ih] / p1[it]);
			}
		}

		/* Entropy of the object pixels */
		double ent_obj = 0.0;
		for (ih = it + 1; ih < 256; ih++) {
			if (data[ih] != 0) {
				ent_obj -= (norm_histo[ih] / p2[it]) * log(norm_histo[ih] / p2[it]);
			}
		}

		/* Total entropy */
		const double tot_ent = ent_back + ent_obj;

		// IJ.log(""+max_ent+"  "+tot_ent);
		if (max_ent < tot_ent) {
			max_ent = tot_ent;
			threshold = it;
		}
	}
	return threshold;
}

int Otsu(cv::Mat& hist) {
	// Otsu's threshold algorithm
	// C++ code by Jordan Bevik <Jordan.Bevic@qtiworld.com>
	// ported to ImageJ plugin by G.Landini
	int k, kStar;  // k = the current threshold; kStar = optimal threshold
	double N1, N;    // N1 = # points with intensity <=k; N = total number of points
	double BCV, BCVmax; // The current Between Class Variance and maximum BCV
	double num, denom;  // temporary bookeeping
	double Sk;  // The total intensity for all histogram points <=k
	double S, L = 256; // The total intensity of the image
	float* data = hist.ptr<float>();

	// Initialize values:
	S = N = 0;
	for (k = 0; k < L; k++) {
		S += (double)k * data[k];   // Total histogram intensity
		N += data[k];       // Total number of data points
	}

	Sk = 0;
	N1 = data[0]; // The entry for zero intensity
	BCV = 0;
	BCVmax = 0;
	kStar = 0;

	// Look at each possible threshold value,
	// calculate the between-class variance, and decide if it's a max
	for (k = 1; k < L - 1; k++) { // No need to check endpoints k = 0 or k = L-1
		Sk += (double)k * data[k];
		N1 += data[k];

		// The float casting here is to avoid compiler warning about loss of precision and
		// will prevent overflow in the case of large saturated images
		denom = (double)(N1) * (N - N1); // Maximum value of denom is (N^2)/4 =  approx. 3E10

		if (denom != 0) {
			// Float here is to avoid loss of precision when dividing
			num = ((double)N1 / N) * S - Sk;  // Maximum value of num =  255*N = approx 8E7
			BCV = (num * num) / denom;
		}
		else
			BCV = 0;

		if (BCV >= BCVmax) { // Assign the best threshold found so far
			BCVmax = BCV;
			kStar = k;
		}
	}
	// kStar += 1;  // Use QTI convention that intensity -> 1 if intensity >= k
	// (the algorithm was developed for I-> 1 if I <= k.)
	return kStar;
}
