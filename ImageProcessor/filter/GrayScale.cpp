#include "GrayScale.h"

void GrayScale::apply_thread(const ImageBuffer& target, ImageBuffer& output, int sRow, int eRow) {
	int rowChCnt = target.width() * 3;

	for (int i = sRow; i < eRow; ++i) {
		uint8_t* outRowPixel = output.rowPtr(i);
		const uint8_t* rowPixel = target.rowPtr(i);

		for (int chIdx = 0; chIdx < rowChCnt; chIdx += 3) {
			int gray_scalar = ((rowPixel[chIdx] * iB) + (rowPixel[chIdx + 1] * iG) + (rowPixel[chIdx + 2] * iR)) >> 8;
			outRowPixel[chIdx] = gray_scalar;
			outRowPixel[chIdx + 1] = gray_scalar;
			outRowPixel[chIdx + 2] = gray_scalar;
		}
	}
}


ImageBuffer GrayScale::apply(const ImageBuffer& target, ImageBuffer& output) {
	cpy_img = ImageBuffer(target.width(), target.height());

	MultiThread mulTh = MultiThread();

	int coreSize = mulTh.coreCount();
	int rowPerThread = target.height() / coreSize;

	for (int rowIdx = 0; rowIdx < coreSize; ++rowIdx) {
		int sRow = rowPerThread * rowIdx;
		int eRow = (rowIdx == coreSize - 1) ? target.height() : rowPerThread * (rowIdx + 1);

		mulTh.addThread(apply_thread, target, output, sRow, eRow);
	}

	mulTh.join();
	cpy_img = output;

	return cpy_img;
}