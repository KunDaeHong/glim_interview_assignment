#include "GaussianBlur.h"

void GaussianBlur::getLutKernel(int kSize, double sigma, int bShift) {
	double sum     = 0.0;
	int radius     = kSize / 2;
	double sigmaSq = sigma * sigma;

	kernel.reserve(kSize);
	kernel.resize(kSize);

	vector<double> tempKernel(kSize);

	for (int i = 0; i < kSize; ++i) {
		int x         = i - radius;
		double factor = exp(-(x * x) / (2.0 * sigmaSq));
		tempKernel[i] = factor;
		sum           += factor;
	}

	int scale     = 1 << (bShift / 2);
	int currntSum = 0;

	for (int i = 0; i < kSize; ++i) {
		kernel[i] = (int)(round((tempKernel[i] / sum) * scale));
		currntSum += kernel[i];
	}

	kernel[radius] += (scale - currntSum);

	tempKernel.clear();
	tempKernel.shrink_to_fit();
}

ImageBuffer GaussianBlur::apply(const ImageBuffer& target, ImageBuffer& output, int kSize, double sigma) {
	getLutKernel(kSize, sigma, 10);
	ImageBuffer result = linearConv(target, kSize, 1, 1, 3);
	output             = result;
	return result;
}