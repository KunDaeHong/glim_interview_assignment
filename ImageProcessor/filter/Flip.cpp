#include "Flip.h"

ImageBuffer Flip::cpyImg;

void Flip::apply_thread_horizontal(const ImageBuffer& target, ImageBuffer& output, int sRow, int eRow) {
	int rowChCnt = target.width();

	if (sRow < 0 || eRow >= target.height()) {
		throw std::out_of_range("The row scope does not match with the original image height.");
	}

	for (int i = sRow; i < eRow; ++i) {
		uint8_t* outPixel          = output.rowPtr(i);
		const uint8_t* originPixel = target.rowPtr(i);
		Buffer3Byte* output3Byte   = reinterpret_cast<Buffer3Byte*>(outPixel);
		Buffer3Byte* origin3Btyte  = reinterpret_cast<Buffer3Byte*>(const_cast<uint8_t*>(originPixel));

		reverse_copy(origin3Btyte, origin3Btyte + rowChCnt, output3Byte); //Reverse every 3 byte by Buffer3Byte
	}
}

void Flip::apply_thread_vertical(const ImageBuffer& target, ImageBuffer& output, int sRow, int eRow) {
	int rowChCnt = target.width();

	if (sRow < 0 || eRow >= target.height()) {
		throw std::out_of_range("The row scope does not match with the original image height.");
	}

	for (int row = sRow; row < eRow; ++row) {
		int pureIdx = (target.height() - 1) - row;

		if (pureIdx < 0 || pureIdx >= target.width()) {
			throw std::out_of_range("Original Image height and row length of thread is not match");
		}

		const uint8_t* originPixel = target.rowPtr(pureIdx);
		uint8_t* outPixel = output.rowPtr(row);

		for (int i = 0; i < rowChCnt * 3; i += 3) {
			outPixel[i + 0] = originPixel[i + 0];
			outPixel[i + 1] = originPixel[i + 1];
			outPixel[i + 2] = originPixel[i + 2];
		}
	}
}

ImageBuffer Flip::flip(FlipType type, const ImageBuffer& target, ImageBuffer& output) {
	cpyImg            = ImageBuffer(target.width(), target.height());

	MultiThread mulTh = MultiThread();
	int coreSize      = mulTh.coreCount();
	int rowPerThread  = target.height() / coreSize;

	for (int rowIdx = 0; rowIdx < coreSize; ++rowIdx) {
		int sRow = rowPerThread * rowIdx;
		int eRow = (rowIdx == coreSize - 1) ? target.height() : rowPerThread * (rowIdx + 1);

		if (type == FlipType::horizontal) {
			mulTh.addThread(apply_thread_horizontal, target, output, sRow, eRow);
		}

		if (type == FlipType::vertical) {
			mulTh.addThread(apply_thread_vertical, target, output, sRow, eRow);
		}
	}

	mulTh.join();
	cpyImg = output;

	return cpyImg;
}