#pragma once
#define _USE_MATH_DEFINES
#include "GaussianBlur.h"

#include <cmath>
#include "ImageBuffer.h"
#include "filter/Convolution.h"

using namespace std;
using namespace ip;

class GaussianBlur : Convolution {
private:
	GaussianBlur() = delete;
	static void getLutKernel(int kSize, double sigma, int bShift = 10);

public:
	static ImageBuffer apply(const ImageBuffer& target, ImageBuffer& output, int kSize, double sigma);
};