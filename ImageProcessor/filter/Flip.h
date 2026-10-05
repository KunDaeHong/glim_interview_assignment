#pragma once

#include "Flip.h"

#include <stdexcept>
#include <algorithm>
#include "ImageBuffer.h"
#include "utils/MultiThread.h"

using namespace ip;
using namespace std;

class Flip {
private:
	static ImageBuffer cpyImg;

	struct Buffer3Byte {
		uint8_t data[3];
	};

	static void apply_thread_vertical(const ImageBuffer& target, ImageBuffer& output, int sRow, int eRow);
	static void apply_thread_horizontal(const ImageBuffer& target, ImageBuffer& output, int sRow, int eRow);

public:
	enum FlipType {
		vertical, horizontal
	};

	static ImageBuffer flip(const FlipType type, const ImageBuffer& target, ImageBuffer& output);

};