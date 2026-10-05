#pragma once

#include "ImageBuffer.h"
#include "utils/MultiThread.h"

using namespace ip;
using namespace std;

class GrayScale {
private:
	//시프트연산으로 계산 하기 위해 각 0.299, 0.587, 0.114를 8비트 스케일링 함.
	//2^8 = 256이므로 각각의 가중치 * 2^8함.
	static const int iR = 77;
	static const int iG = 150;
	static const int iB = 29;

	ImageBuffer cpy_img;

	static void apply_thread(const ImageBuffer& target, ImageBuffer& output, int sRow, int eRow);
public:

	ImageBuffer apply(const ImageBuffer& target, ImageBuffer& output);
	~GrayScale() = default;
};