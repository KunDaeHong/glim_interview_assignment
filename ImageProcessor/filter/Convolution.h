#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include "utils/MultiThread.h"

using namespace std;

class Convolution {

protected:
	static vector<int> kernel; // Default Bit Shift 10
	static ImageBuffer linearConv(const ImageBuffer& target, const int kSize = 3, int padding = 1, int stride = 1, int ch = 3);
private:
	Convolution() = delete;

	//Caution! TargetPtr, width and height are including padding.
	static void mul_thread_apply_seperate_horizontal(const uint8_t* targetPtr, uint8_t* outputPtr, int width, int height, int sRow, int eRow, int padding, int stride);
	static void mul_thread_apply_seperate_vertical(const uint8_t* targetPtr, uint8_t* outputPtr, int width, int height, int sRow, int eRow, int padding, int stride);
};
