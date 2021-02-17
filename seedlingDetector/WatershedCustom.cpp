#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <array>
#include "Helpers.hpp"
#include "WatershedCustom.hpp"
#include <iostream>


using namespace cv;
using namespace std;

const int SINGLE_POINTS = 0;
const int SEGMENTED = 2;

const uchar MAXIMUM = 1; // marks local maxima (irrespective of noise tolerance)
const uchar LISTED = 2; // marks points currently in the list
const uchar PROCESSED = 4; // marks points processed previously
const uchar MAX_AREA = 8; // marks areas near a maximum, within the tolerance
const uchar EQUAL = 16; // marks contigous maximum points of equal level
const uchar MAX_POINT = 32; // marks a single point standing for a maximum
const uchar ELIMINATED = 64; // marks maxima that have been eliminated before watershed

const int ONE = 41;
const int SQRT2 = 58;
const int SQRT5 = 92;

array<int, 8> dirOffset;
array<int, 8> dirXoffset = {0, 1, 1, 1, 0, -1, -1, -1};
array<int, 8> dirYoffset = {-1, -1, 0, 1, 1, 1, 0, -1};

class MaxPoint
{
public:
	short x;
	short y;
	float value;

	MaxPoint(const short x, const short y, const float value) : x(x), y(y), value(value) {}

	bool operator<(const MaxPoint other) const { return value < other.value; }
};

int trueEdmHeight(const int x, const int y, Mat& ip)
{
	const int xmax = ip.cols - 1;
	const int ymax = ip.rows - 1;
	const int v = ip.at<short>(y, x);
	if (x == 0 || y == 0 || x == xmax || y == ymax || v == 0) {
		return v; //don't recalculate for edge pixels or background
	}
	else {
		const int one = ONE;
		const int sqrt2 = SQRT2;
		int trueH = v + sqrt2 / 2; //true height can never by higher than this
		bool ridgeOrMax = false;
		for (int d = 0; d < 4; d++) {
			//for all directions halfway around:
			const int d2 = (d + 4) % 8; //get the opposite direction and neighbors
			int v1 = ip.at<short>(y + dirYoffset[d], x + dirXoffset[d]);
			int v2 = ip.at<short>(y + dirYoffset[d2], x + dirXoffset[d2]);
			int h;
			if (v >= v1 && v >= v2) {
				ridgeOrMax = true;
				h = (v1 + v2) / 2;
			}
			else {
				h = min(v1, v2);
			}
			h += (d % 2 == 0) ? one : sqrt2; //in diagonal directions, distance is sqrt2
			if (trueH > h) trueH = h;
		}
		if (!ridgeOrMax) trueH = v;
		return trueH;
	}
}

bool isWithin(Mat& ip, const int x, const int y, const int direction)
{
	const int width = ip.cols;
	const int height = ip.rows;
	const int xmax = width - 1;
	const int ymax = height - 1;
	switch (direction) {
	case 0:
		return (y > 0);
	case 1:
		return (x < xmax && y > 0);
	case 2:
		return (x < xmax);
	case 3:
		return (x < xmax && y < ymax);
	case 4:
		return (y < ymax);
	case 5:
		return (x > 0 && y < ymax);
	case 6:
		return (x > 0);
	case 7:
		return (x > 0 && y > 0);
	default:
		return false;
	}
}

void getSortedMaxPoints(Mat& ip, Mat& typeP, vector<MaxPoint>& maxPoints, const bool excludeEdgesNow, const bool isEDM, const float globalMin)
{
	const int width = ip.cols;
	const int height = ip.rows;

	int nMax = 0; //counts local maxima
	for (int y = 0; y < height; y++) {
		// find local maxima now
		for (int x = 0; x < width; x++) {
			const float v = ip.at<short>(y, x);
			const float vTrue = isEDM ? trueEdmHeight(x, y, ip) : v; // for 16-bit EDMs, use interpolated ridge height
			if (v == globalMin) continue;
			if (excludeEdgesNow && (x == 0 || x == width - 1 || y == 0 || y == height - 1)) continue;
			bool isMax = true;
			/* check wheter we have a local maximum.
			Note: For a 16-bit EDM, we need all maxima: those of the EDM-corrected values
			(needed by findMaxima) and those of the raw values (needed by cleanupMaxima) */
			for (int d = 0; d < 8; d++) {
				// compare with the 8 neighbor pixels
				if (isWithin(ip, x, y, d)) {
					const float vNeighbor = ip.at<short>(y + dirYoffset[d], x + dirXoffset[d]);
					const float vNeighborTrue = isEDM ? trueEdmHeight(x + dirXoffset[d], y + dirYoffset[d], ip) : vNeighbor;
					if (vNeighbor > v && vNeighborTrue > vTrue) {
						isMax = false;
						break;
					}
				}
			}
			if (isMax) {
				typeP.at<uchar>(y, x) = MAXIMUM;
				nMax++;
			}
		} // for x
	} // for y
	maxPoints.reserve(nMax);
	for (short y = 0; y < height; y++) {
		for (short x = 0; x < width; x++) {
			if (typeP.at<uchar>(y, x) == MAXIMUM) {
				maxPoints.push_back(MaxPoint(x, y, isEDM ? trueEdmHeight(x, y, ip) : ip.at<short>(x, y)));
			}
		}
	}
	std::sort(maxPoints.begin(), maxPoints.end());
}

void analyzeAndMarkMaxima(Mat& ip, Mat& typeP, vector<MaxPoint>& maxPoints, const bool excludeEdgesNow, const bool isEDM, const float globalMin, const double tolerance)
{
	const int width = ip.cols;
	const int height = ip.rows;
	const int nMax = maxPoints.size();
	uchar* types = typeP.ptr<uchar>();
	short* xList = new short[width * height](); //here we enter points starting from a maximum
	short* yList = new short[width * height]();
	//bool displayOrCount = false;
	for (int iMax = nMax - 1; iMax >= 0; iMax--) {
		//process all maxima now, starting from the highest
		const float v = maxPoints[iMax].value;
		if (v == globalMin) break;
		int offset = maxPoints[iMax].x + width * maxPoints[iMax].y;
		if ((types[offset] & PROCESSED) != 0) //this maximum has been reached from another one, skip it
			continue;
		xList[0] = maxPoints[iMax].x; //we create a list of connected points and start the list
		yList[0] = maxPoints[iMax].y; //  at the current maximum
		types[offset] |= (EQUAL | LISTED); //mark first point as equal height (to itself) and listed
		int listLen = 1; //number of elements in the list
		int listI = 0; //index of current element in the list
		//bool isEdgeMaximum = (xList[0] == 0 || xList[0] == width - 1 || yList[0] == 0 || yList[0] == height - 1);
		bool maxPossible = true; //it may be a true maxmum
		double xEqual = xList[0]; //for creating a single point: determine average over the
		double yEqual = yList[0]; //  coordinates of contiguous equal-height points
		int nEqual = 1; //counts xEqual/yEqual points that we use for averaging
		do {
			offset = xList[listI] + width * yList[listI];
			for (int d = 0; d < 8; d++) {
				//analyze all neighbors (in 8 directions) at the same level
				const int offset2 = offset + dirOffset[d];
				if (isWithin(ip, xList[listI], yList[listI], d) && (types[offset2] & LISTED) == 0) {
					if ((types[offset2] & PROCESSED) != 0) {
						maxPossible = false; //we have reached a point processed previously, thus it is no maximum now
						break;
					}
					const int x2 = xList[listI] + dirXoffset[d];
					const int y2 = yList[listI] + dirYoffset[d];
					float v2 = ip.at<short>(y2, x2);
					if (isEDM && (v2 <= v - static_cast<float>(tolerance))) v2 = trueEdmHeight(x2, y2, ip); //correcting for EDM peaks may move the point up
					if (v2 > v) {
						maxPossible = false; //we have reached a higher point, thus it is no maximum
						break;
					}
					else if (v2 >= v - static_cast<float>(tolerance)) {
						xList[listLen] = static_cast<float>(x2);
						yList[listLen] = static_cast<float>(y2);
						listLen++; //we have found a new point within the tolerance
						types[offset2] |= LISTED;
						if (x2 == 0 || x2 == width - 1 || y2 == 0 || y2 == height - 1) {
							//isEdgeMaximum = true;
							if (excludeEdgesNow) {
								maxPossible = false;
								break; //we have an edge maximum;
							}
						}
						if (v2 == v) {
							//prepare finding center of equal points (in case single point needed)
							types[offset2] |= EQUAL;
							xEqual += x2;
							yEqual += y2;
							nEqual++;
						}
					}
				} // if isWithin & not LISTED
			} // for directions d
			listI++;
		}
		while (listI < listLen);
		const uchar resetMask = static_cast<float>(~(maxPossible ? LISTED : (LISTED | EQUAL)));
		xEqual /= nEqual;
		yEqual /= nEqual;
		double minDist2 = 1e20;
		int nearestI = 0;
		for (listI = 0; listI < listLen; listI++) {
			offset = xList[listI] + width * yList[listI];
			types[offset] &= resetMask; //reset attributes no longer needed
			types[offset] |= PROCESSED; //mark as processed
			if (maxPossible) {
				types[offset] |= MAX_AREA;
				if ((types[offset] & EQUAL) != 0) {
					const double dist2 = (xEqual - xList[listI]) * (xEqual - xList[listI]) + (yEqual - yList[listI]) * (yEqual - yList[listI]);
					if (dist2 < minDist2) {
						minDist2 = dist2; //this could be the best "single maximum" point
						nearestI = listI;
					}
				}
			}
		} // for listI
		if (maxPossible) {
			types[xList[nearestI] + width * yList[nearestI]] |= MAX_POINT;
		}
	}
	delete[] xList;
	delete[] yList;
}

void make8bit(Mat& ip, Mat& dst, Mat& typeP, const bool isEDM, const float globalMin, const float globalMax)
{
	const int width = ip.cols;
	const int height = ip.rows;
	uchar* types = typeP.ptr<uchar>();
	const double minValue = globalMin;
	const double offset = minValue - (globalMax - minValue) * (1. / 253 / 2 - 1e-6); //everything above minValue should become >(uchar)0
	double factor = 253 / (globalMax - minValue);

	if (isEDM && factor > 1. / ONE)
		factor = 1. / ONE; // in EDM, no better resolution is needed
	dst.create(height, width, CV_8U);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			const long v = static_cast<long>(round((ip.at<short>(y, x) - offset) * factor));
			if (v < 0) dst.at<uchar>(y, x) = 0;
			else if (v <= 255) dst.at<uchar>(y, x) = static_cast<uchar>(v & 255);
			else dst.at<uchar>(y, x) = 255;
		}
	}

	for (int y = 0, i = 0; y < height; y++) {
		for (int x = 0; x < width; x++, i++) {
			if ((dst.at<uchar>(y, x) & 255) == 255) //pixel value 255 is reserved
				dst.at<uchar>(y, x)--;
			if ((types[i] & MAX_AREA) != 0)
				dst.at<uchar>(y, x) = 255; //prepare watershed by setting "true" maxima+surroundings to 255
		}
	}
}

void cleanupMaxima(Mat& outIp, Mat& typeP, vector<MaxPoint>& maxPoints)
{
	uchar* pixels = outIp.ptr<uchar>();
	uchar* types = typeP.ptr<uchar>();
	const int width = outIp.cols;
	const int height = outIp.rows;
	const int nMax = maxPoints.size();
	short* xList = new short[width * height]();
	short* yList = new short[width * height]();
	for (int iMax = nMax - 1; iMax >= 0; iMax--) {
		int offset = maxPoints[iMax].x + width * maxPoints[iMax].y;
		if ((types[offset] & (MAX_AREA | ELIMINATED)) != 0) continue;
		const int level = pixels[offset] & 255;
		int loLevel = level + 1;
		xList[0] = maxPoints[iMax].x; //we start the list at the current maximum
		yList[0] = maxPoints[iMax].y;
		types[offset] |= LISTED; //mark first point as listed
		int listLen = 1; //number of elements in the list
		int lastLen = 1;
		int listI; //index of current element in the list
		bool saddleFound = false;
		while (!saddleFound && loLevel > 0) {
			loLevel--;
			lastLen = listLen; //remember end of list for previous level
			listI = 0; //in each level, start analyzing the neighbors of all pixels
			do {
				//for all pixels listed so far
				offset = xList[listI] + width * yList[listI];
				for (int d = 0; d < 8; d++) {
					//analyze all neighbors (in 8 directions) at the same level
					const int offset2 = offset + dirOffset[d];
					if (isWithin(typeP, xList[listI], yList[listI], d) && (types[offset2] & LISTED) == 0) {
						if ((types[offset2] & MAX_AREA) != 0 || (((types[offset2] & ELIMINATED) != 0) && (pixels[offset2] & 255) >= loLevel)) {
							saddleFound = true; //we have reached a point touching a "true" maximum...
							break; //...or a level not lower, but touching a "true" maximum
						}
						else if ((pixels[offset2] & 255) >= loLevel && (types[offset2] & ELIMINATED) == 0) {
							xList[listLen] = static_cast<short>(xList[listI] + dirXoffset[d]);
							yList[listLen] = static_cast<short>(yList[listI] + dirYoffset[d]);
							listLen++; //we have found a new point to be processed
							types[offset2] |= LISTED;
						}
					} // if isWithin & not LISTED
				} // for directions d
				if (saddleFound) break; //no reason to search any further
				listI++;
			}
			while (listI < listLen);
		} // while !levelFound && loLevel>=0
		for (listI = 0; listI < listLen; listI++) //reset attribute since we may come to this place again
			types[xList[listI] + width * yList[listI]] &= ~LISTED;
		for (listI = 0; listI < lastLen; listI++) {
			//for all points higher than the level of the saddle point
			offset = xList[listI] + width * yList[listI];
			pixels[offset] = loLevel; //set pixel value to the level of the saddle point
			types[offset] |= ELIMINATED; //mark as processed: there can't be a local maximum in this area
		}
	} // for all maxima iMax
	delete[] xList;
	delete[] yList;
}

void makeFateTable(vector<int>& table)
{
	bool* isSet = new bool[8]();
	for (int item = 0; item < 256; item++) {
		//dissect into pixels
		for (int i = 0, mask = 1; i < 8; i++) {
			isSet[i] = (item & mask) == mask;
			mask *= 2;
		}
		for (int i = 0, mask = 1; i < 8; i++) {
			//we dilate in the direction opposite to the direction of the existing neighbors
			if (isSet[(i + 4) % 8]) table[item] |= mask;
			mask *= 2;
		}
		for (int i = 0; i < 8; i += 2) //if side pixels are set, for counting transitions it is as good as if the adjacent edges were also set
			if (isSet[i]) {
				isSet[(i + 1) % 8] = true;
				isSet[(i + 7) % 8] = true;
			}
		int transitions = 0;
		for (int i = 0; i < 8; i++) {
			if (isSet[i] != isSet[(i + 1) % 8])
				transitions++;
		}
		if (transitions >= 4) {
			//if neighbors contain more than one region, dilation ito this pixel is forbidden
			table[item] = 0;
		}
	}
}

bool processLevel(const int level, const int pass, Mat& ip1, Mat& ip2, vector<int>& table, int* histogram, int* levelStart, short* xCoordinate, short* yCoordinate)
{
	const int rowSize = ip1.cols;
	const int height = ip1.rows;
	const int xmax = rowSize - 1;
	const int ymax = height - 1;
	uchar* pixels1 = ip1.ptr<uchar>();
	uchar* pixels2 = ip2.ptr<uchar>();
	bool changed = false;
	for (int i = 0; i < histogram[level]; i++) {
		const int coordOffset = levelStart[level] + i;
		const int x = xCoordinate[coordOffset];
		const int y = yCoordinate[coordOffset];
		const int offset = x + y * rowSize;
		if ((pixels2[offset] & 255) != 255) {
			int index = 0;
			if (y > 0 && (pixels2[offset - rowSize] & 255) == 255)
				index ^= 1;
			if (x < xmax && y > 0 && (pixels2[offset - rowSize + 1] & 255) == 255)
				index ^= 2;
			if (x < xmax && (pixels2[offset + 1] & 255) == 255)
				index ^= 4;
			if (x < xmax && y < ymax && (pixels2[offset + rowSize + 1] & 255) == 255)
				index ^= 8;
			if (y < ymax && (pixels2[offset + rowSize] & 255) == 255)
				index ^= 16;
			if (x > 0 && y < ymax && (pixels2[offset + rowSize - 1] & 255) == 255)
				index ^= 32;
			if (x > 0 && (pixels2[offset - 1] & 255) == 255)
				index ^= 64;
			if (x > 0 && y > 0 && (pixels2[offset - rowSize - 1] & 255) == 255)
				index ^= 128;
			switch (pass) {
			case 0: if ((table[index] & 1) == 1) {
					pixels1[offset] = 255;
					changed = true;
				}
				break;
			case 1: if ((table[index] & 2) == 2) {
					pixels1[offset] = 255;
					changed = true;
				}
				break;
			case 2: if ((table[index] & 4) == 4) {
					pixels1[offset] = 255;
					changed = true;
				}
				break;
			case 3: if ((table[index] & 8) == 8) {
					pixels1[offset] = 255;
					changed = true;
				}
				break;
			case 4: if ((table[index] & 16) == 16) {
					pixels1[offset] = 255;
					changed = true;
				}
				break;
			case 5: if ((table[index] & 32) == 32) {
					pixels1[offset] = 255;
					changed = true;
				}
				break;
			case 6: if ((table[index] & 64) == 64) {
					pixels1[offset] = 255;
					changed = true;
				}
				break;
			case 7: if ((table[index] & 128) == 128) {
					pixels1[offset] = 255;
					changed = true;
				}
				break;
			default:
				printf("Invalid call to processLevel!");
			} // switch
		} // if .. !=255
	} // for pixel i
	if (changed) //make sure that ip2 is updated for the next time
		std::copy_n(pixels1, rowSize * height, pixels2);
	return changed;
}

void watershedSegment(Mat& ip)
{
	const int width = ip.cols;
	const int height = ip.rows;

	uchar* pixels = ip.ptr<uchar>();
	// create arrays with the coordinates of all points between value 1 and 254
	//This method, suggested by Stein Roervik (stein_at_kjemi-dot-unit-dot-no),
	//greatly speeds up the watershed segmentation routine.
	Mat hist;
	getHistogramGrayscale(ip, hist);

	int* histogram = hist.ptr<int>();
	const int arraySize = width * height - histogram[0] - histogram[255];
	short* xCoordinate = new short[arraySize]();
	short* yCoordinate = new short[arraySize]();
	int highestValue = 0;
	int offset = 0;
	int* levelStart = new int[256]();
	for (int v = 1; v < 255; v++) {
		levelStart[v] = offset;
		offset += histogram[v];
		if (histogram[v] > 0) highestValue = v;
	}
	int* levelOffset = new int[highestValue + 1]();
	for (int y = 0, i = 0; y < height; y++) {
		for (int x = 0; x < width; x++, i++) {
			const int v = pixels[i] & 255;
			if (v > 0 && v < 255) {
				offset = levelStart[v] + levelOffset[v];
				xCoordinate[offset] = static_cast<short>(x);
				yCoordinate[offset] = static_cast<short>(y);
				levelOffset[v] ++;
			}
		} //for x
	} //for y
	// now do the segmentation, starting at the highest level and working down.
	// At each level, dilate the particle, constrained to pixels whose values are
	// at that level and also constrained to prevent features from merging.
	vector<int> table(256);
	makeFateTable(table);
	Mat ip2 = ip.clone();
	for (int level = highestValue; level >= 1; level--) {
		int idle = 1;
		while (true) {
			// break the loop at any point after 8 idle processLevel calls
			if (processLevel(level, 7, ip, ip2, table, histogram, levelStart, xCoordinate, yCoordinate))
				idle = 0;
			if (++idle >= 8) break;
			if (processLevel(level, 3, ip, ip2, table, histogram, levelStart, xCoordinate, yCoordinate))
				idle = 0;
			if (idle++ >= 8) break;
			if (processLevel(level, 1, ip, ip2, table, histogram, levelStart, xCoordinate, yCoordinate))
				idle = 0;
			if (idle++ >= 8) break;
			if (processLevel(level, 5, ip, ip2, table, histogram, levelStart, xCoordinate, yCoordinate))
				idle = 0;
			if (idle++ >= 8) break;
			if (processLevel(level, 0, ip, ip2, table, histogram, levelStart, xCoordinate, yCoordinate))
				idle = 0;
			if (idle++ >= 8) break;
			if (processLevel(level, 4, ip, ip2, table, histogram, levelStart, xCoordinate, yCoordinate))
				idle = 0;
			if (idle++ >= 8) break;
			if (processLevel(level, 2, ip, ip2, table, histogram, levelStart, xCoordinate, yCoordinate))
				idle = 0;
			if (idle++ >= 8) break;
			if (processLevel(level, 6, ip, ip2, table, histogram, levelStart, xCoordinate, yCoordinate))
				idle = 0;
			if (idle++ >= 8) break;
		}
	}
	delete[] xCoordinate;
	delete[] yCoordinate;
	delete[] levelStart;
	delete[] levelOffset;
}

void findMaxima(Mat& src, Mat& dst, const double tolerance, const int outputType, const bool excludeOnEdges, const bool isEDM)
{
	const int width = src.cols;
	const int height = src.rows;
	Mat typeP(height, width, CV_8UC1, Scalar(0));
	//uchar *types = typeP.ptr<uchar>();
	dirOffset = {-width, -width + 1, +1, +width + 1, +width, +width - 1, -1, -width - 1};

	float globalMin = FLT_MAX;
	float globalMax = -FLT_MAX;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			const float v = src.at<short>(y, x);
			if (globalMin > v) globalMin = v;
			if (globalMax < v) globalMax = v;
		}
	}

	vector<MaxPoint> maxPoints;
	getSortedMaxPoints(src, typeP, maxPoints, false, isEDM, globalMin);
	analyzeAndMarkMaxima(src, typeP, maxPoints, false, isEDM, globalMin, tolerance);

	if (outputType == SEGMENTED) {
		//create 8-bit image from src with background 0 and maximum areas 255
		make8bit(src, dst, typeP, isEDM, globalMin, globalMax);
		cleanupMaxima(dst, typeP, maxPoints); //eliminate all the small maxima (i.e. those outside MAX_AREA)
		watershedSegment(dst); //do watershed segmentation
		dst = dst == 255;; //levels to binary image
	}
	else if (outputType == SINGLE_POINTS) {
		dst = typeP & MAX_POINT;
	}
}


void setValue(const int offset, const int rowsize, short* image16)
{
	const int r1 = offset - rowsize - rowsize - 2;
	const int r2 = r1 + rowsize;
	const int r3 = r2 + rowsize;
	const int r4 = r3 + rowsize;
	const int r5 = r4 + rowsize;
	int min = 32767;

	int v = image16[r2 + 2] + ONE;
	if (v < min)
		min = v;
	v = image16[r3 + 1] + ONE;
	if (v < min)
		min = v;
	v = image16[r3 + 3] + ONE;
	if (v < min)
		min = v;
	v = image16[r4 + 2] + ONE;
	if (v < min)
		min = v;

	v = image16[r2 + 1] + SQRT2;
	if (v < min)
		min = v;
	v = image16[r2 + 3] + SQRT2;
	if (v < min)
		min = v;
	v = image16[r4 + 1] + SQRT2;
	if (v < min)
		min = v;
	v = image16[r4 + 3] + SQRT2;
	if (v < min)
		min = v;

	v = image16[r1 + 1] + SQRT5;
	if (v < min)
		min = v;
	v = image16[r1 + 3] + SQRT5;
	if (v < min)
		min = v;
	v = image16[r2 + 4] + SQRT5;
	if (v < min)
		min = v;
	v = image16[r4 + 4] + SQRT5;
	if (v < min)
		min = v;
	v = image16[r5 + 3] + SQRT5;
	if (v < min)
		min = v;
	v = image16[r5 + 1] + SQRT5;
	if (v < min)
		min = v;
	v = image16[r4] + SQRT5;
	if (v < min)
		min = v;
	v = image16[r2] + SQRT5;
	if (v < min)
		min = v;

	image16[offset] = static_cast<short>(min);
}

void setEdgeValue(const int offset, const int rowsize, short* image16, const int x, const int y, const int xmax, const int ymax)
{
	int v;
	const int r1 = offset - rowsize - rowsize - 2;
	const int r2 = r1 + rowsize;
	const int r3 = r2 + rowsize;
	const int r4 = r3 + rowsize;
	const int r5 = r4 + rowsize;
	int min = 32767;
	const int offimage = image16[r3 + 2];

	if (y < 1)
		v = offimage + ONE;
	else
		v = image16[r2 + 2] + ONE;
	if (v < min)
		min = v;

	if (x < 1)
		v = offimage + ONE;
	else
		v = image16[r3 + 1] + ONE;
	if (v < min)
		min = v;

	if (x > xmax)
		v = offimage + ONE;
	else
		v = image16[r3 + 3] + ONE;
	if (v < min)
		min = v;

	if (y > ymax)
		v = offimage + ONE;
	else
		v = image16[r4 + 2] + ONE;
	if (v < min)
		min = v;

	if ((x < 1) || (y < 1))
		v = offimage + SQRT2;
	else
		v = image16[r2 + 1] + SQRT2;
	if (v < min)
		min = v;

	if ((x > xmax) || (y < 1))
		v = offimage + SQRT2;
	else
		v = image16[r2 + 3] + SQRT2;
	if (v < min)
		min = v;

	if ((x < 1) || (y > ymax))
		v = offimage + SQRT2;
	else
		v = image16[r4 + 1] + SQRT2;
	if (v < min)
		min = v;

	if ((x > xmax) || (y > ymax))
		v = offimage + SQRT2;
	else
		v = image16[r4 + 3] + SQRT2;
	if (v < min)
		min = v;

	if ((x < 1) || (y <= 1))
		v = offimage + SQRT5;
	else
		v = image16[r1 + 1] + SQRT5;
	if (v < min)
		min = v;

	if ((x > xmax) || (y <= 1))
		v = offimage + SQRT5;
	else
		v = image16[r1 + 3] + SQRT5;
	if (v < min)
		min = v;

	if ((x >= xmax) || (y < 1))
		v = offimage + SQRT5;
	else
		v = image16[r2 + 4] + SQRT5;
	if (v < min)
		min = v;

	if ((x >= xmax) || (y > ymax))
		v = offimage + SQRT5;
	else
		v = image16[r4 + 4] + SQRT5;
	if (v < min)
		min = v;

	if ((x > xmax) || (y >= ymax))
		v = offimage + SQRT5;
	else
		v = image16[r5 + 3] + SQRT5;
	if (v < min)
		min = v;

	if ((x < 1) || (y >= ymax))
		v = offimage + SQRT5;
	else
		v = image16[r5 + 1] + SQRT5;
	if (v < min)
		min = v;

	if ((x <= 1) || (y > ymax))
		v = offimage + SQRT5;
	else
		v = image16[r4] + SQRT5;
	if (v < min)
		min = v;

	if ((x <= 1) || (y < 1))
		v = offimage + SQRT5;
	else
		v = image16[r2] + SQRT5;
	if (v < min)
		min = v;

	image16[offset] = static_cast<short>(min);
}

Mat make16bitEDM(Mat& ip)
{
	int offset;

	const int width = ip.cols;
	const int height = ip.rows;
	const int rowsize = width;
	const int xmax = width - 2;
	const int ymax = height - 2;
	Mat ip16;
	ip.convertTo(ip16, CV_16S);
	ip16 *= 128;
	short* image16 = ip16.ptr<short>();

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			offset = x + y * rowsize;
			if (image16[offset] > 0) {
				if ((x <= 1) || (x >= xmax) || (y <= 1) || (y >= ymax))
					setEdgeValue(offset, rowsize, image16, x, y, xmax, ymax);
				else
					setValue(offset, rowsize, image16);
			}
		} // for x
	} // for y

	for (int y = height - 1; y >= 0; y--) {
		for (int x = width - 1; x >= 0; x--) {
			offset = x + y * rowsize;
			if (image16[offset] > 0) {
				if ((x <= 1) || (x >= xmax) || (y <= 1) || (y >= ymax))
					setEdgeValue(offset, rowsize, image16, x, y, xmax, ymax);
				else
					setValue(offset, rowsize, image16);
			}
		} // for x
	} // for y

	return ip16;
}

void watershedProcess(Mat& src, Mat& dst, const double tolerance)
{
	CV_Assert(src.type() == CV_8UC1);

	Mat src16 = make16bitEDM(src);
	findMaxima(src16, dst, tolerance, SEGMENTED, false, true);
}

void findMaximumPoints(Mat& src, Mat& dst, const double tolerance)
{
	CV_Assert(src.type() == CV_8UC1);

	Mat src16 = make16bitEDM(src);
	findMaxima(src16, dst, tolerance, SINGLE_POINTS, false, true);
}
