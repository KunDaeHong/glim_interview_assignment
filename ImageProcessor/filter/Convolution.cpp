#include "Convolution.h"

vector<int> Convolution::kernel;

ImageBuffer Convolution::linearConv(const ImageBuffer& target, const int kSize, int padding, int stride, int ch) {
	//Multi-Thread
	MultiThread mulTh	    = MultiThread();
	int coreSize		    = mulTh.coreCount();
	int rowPerThread	    = target.height() / coreSize;

	// Image Metadata
	int width			    = target.width();
	int height			    = target.height();
	int paddedWidth         = width + padding * 2;
	int paddedHeight        = height + padding * 2;

	// Image Data
	uint8_t* tempVertical = new uint8_t[width * height * ch];
	uint8_t* tempHorizontal = new uint8_t[width * ch * paddedHeight];
	ImageBuffer output      = ImageBuffer(target.width(), target.height());
	uint8_t* paddedTarget   = new uint8_t[paddedWidth * paddedHeight * ch]();

	for (int y = 0; y < height; ++y) {
		int eIdx = (y + padding) * paddedWidth * ch + padding * ch;
		memcpy(paddedTarget + eIdx, target.rowPtr(y), static_cast<size_t>(width) * ch);
	}

	for (int p = 0; p < padding; ++p) {
		memcpy(paddedTarget + p * paddedWidth * ch + padding * ch, target.rowPtr(0), static_cast<size_t>(width) * ch);
		memcpy(paddedTarget + (height + padding + p) * paddedWidth * ch + padding * ch, target.rowPtr(height - 1), static_cast<size_t>(width) * ch);
	}

	for (int y = 0; y < paddedHeight; ++y) {
		int rowStart = y * paddedWidth * ch;
		memcpy(paddedTarget + rowStart, paddedTarget + rowStart + padding * ch, static_cast<size_t>(padding) * ch); 

		memcpy(paddedTarget + rowStart + (width + padding) * ch,
			paddedTarget + rowStart + (width + padding - 1) * ch, padding * ch);
	}


	//Horizontal Workload
	for (int rowIdx = 0; rowIdx < coreSize; ++rowIdx) {
		int sRow = rowPerThread * rowIdx;
		int eRow = (rowIdx == coreSize - 1) ? paddedHeight : rowPerThread * (rowIdx + 1);

		if (eRow > paddedHeight) {
			throw std::out_of_range("The row scope does not match with the original image height.");
		}

		//CAUTION!!
		//Don't make a temp pointer data in for sentences. Although you send the pointer with ref, it will be revoke by for-sentences.
		//Whatever you push back a pointer data in vecto. This pointer address will be revoke as soon as possible.

		mulTh.add_thread_convolution(
			mul_thread_apply_seperate_horizontal,
			cref(paddedTarget),
			ref(tempHorizontal),
			paddedWidth,
			paddedHeight,
			sRow,
			eRow,
			padding,
			stride
		);
	}

	mulTh.join();

	//Horizontal Test
	//for (int y = 0; y < 512; ++y) {
	//	uint8_t* outputRow = output.rowPtr(y);

	//	for (int chIdx = 0; chIdx < width * ch; chIdx += ch) {
	//		int bufferIdx = y * width * ch + chIdx;
	//		outputRow[chIdx] = tempHorizontal[bufferIdx];
	//		outputRow[chIdx + 1] = tempHorizontal[bufferIdx + 1];
	//		outputRow[chIdx + 2] = tempHorizontal[bufferIdx + 2];
	//	}
	//}

	//Vertical Workload
	for (int rowIdx = 0; rowIdx < coreSize; ++rowIdx) {
		int sRow = rowPerThread * rowIdx;
		int eRow = (rowIdx == coreSize - 1) ? height : rowPerThread * (rowIdx + 1);

		if (eRow > height) {
			throw std::out_of_range("The row scope does not match with the original image height.");
		}

		//CAUTION!!
		//Don't make a temp pointer data in for sentences. Although you send the pointer with ref, it will be revoke by for-sentences.
		//Whatever you push back a pointer data in vecto. This pointer address will be revoke as soon as possible.
		mulTh.add_thread_convolution(
			mul_thread_apply_seperate_vertical,
			cref(tempHorizontal),
			tempVertical,
			width,
			height,
			sRow,
			eRow,
			padding,
			stride
		);
	}

	mulTh.join();

	for (int y = 0; y < height; ++y) {
		uint8_t* outputRow = output.rowPtr(y);

		for (int chIdx = 0; chIdx < width * ch; chIdx += ch) {
			int bufferIdx        = y * width * ch + chIdx;
			//I already initialized that (new uint8_t[width * height * ch]();)
			outputRow[chIdx]     = tempVertical[bufferIdx];
			outputRow[chIdx + 1] = tempVertical[bufferIdx + 1];
			outputRow[chIdx + 2] = tempVertical[bufferIdx + 2];
		}
	}

	delete[] paddedTarget;
	delete[] tempHorizontal;
	delete[] tempVertical;

	return output;
}

//Caution! TargetPtr, width and height are including padding.
void Convolution::mul_thread_apply_seperate_horizontal(const uint8_t* targetPtr, uint8_t* tempHorizontal, int width, int height, int sRow, int eRow, int padding, int stride) {
	int kSize = (int)kernel.size();
	int pureWidth = width - (padding * 2);

	//Horizontal Calaculate
	try {
		for (int s = sRow; s < eRow; ++s) {
			int sHorizonIdx  = s * width * 3 + (padding * 3);
			int eHorizonIdx  = sHorizonIdx + (pureWidth * 3);
			int writeRowStart = s * pureWidth * 3;

			for (int col = sHorizonIdx; col < eHorizonIdx; col += stride * 3) { //Provide start point only
				vector<int> temp_ptr_value = { 0, 0, 0 }; //BGR

				for (int k = 0; k < kSize; ++k) {
					//The image channel currently is fixed to 3 channel (BGR)
					int factKernelIdx = col - (padding * 3) + (k * 3);
					temp_ptr_value[0] += targetPtr[factKernelIdx + 0] * kernel[k]; //B
					temp_ptr_value[1] += targetPtr[factKernelIdx + 1] * kernel[k]; //G
					temp_ptr_value[2] += targetPtr[factKernelIdx + 2] * kernel[k]; //R
				}

				int localIdx = writeRowStart + (col - sHorizonIdx);

				tempHorizontal[localIdx]     = temp_ptr_value[0] >> 5;
				tempHorizontal[localIdx + 1] = temp_ptr_value[1] >> 5;
				tempHorizontal[localIdx + 2] = temp_ptr_value[2] >> 5;

				temp_ptr_value = { 0, 0 ,0 };
			}
		}
	}
	catch (out_of_range& err) {
		printf("An error occured. Out Of range.");
		cout << err.what() << '\n';
		cout << "------------------------------" << "\n";
	}
}

void Convolution::mul_thread_apply_seperate_vertical(const uint8_t* targetPtr, uint8_t* outputPtr, int width, int height, int sRow, int eRow, int padding, int stride) {
	int kSize = kernel.size();

	//Verical Calculate
	try {
		int cntPerRow = width * 3;
		int sVerticalIdx = sRow * cntPerRow;
		int eVerticalIdx = eRow  * cntPerRow;

		for (int row = sVerticalIdx; row < eVerticalIdx; row += cntPerRow) {
			int rowIdx = row / cntPerRow;

			for (int col = 0; col < cntPerRow; col += stride * 3) {
				vector<int> temp_ptr_value = { 0, 0, 0 }; //BGR

				for (int k = 0; k < kSize; ++k) {
					//The image channel currently is fixed to 3 channel (BGR)
					int factRowIdx = (rowIdx + k) * cntPerRow + col;
					temp_ptr_value[0] += targetPtr[factRowIdx + 0] * kernel[k];
					temp_ptr_value[1] += targetPtr[factRowIdx + 1] * kernel[k];
					temp_ptr_value[2] += targetPtr[factRowIdx + 2] * kernel[k];
				}

				int isolatedThreadIdx            = rowIdx * cntPerRow + col;
				outputPtr[isolatedThreadIdx]     = temp_ptr_value[0] >> 5;
				outputPtr[isolatedThreadIdx + 1] = temp_ptr_value[1] >> 5;
				outputPtr[isolatedThreadIdx + 2] = temp_ptr_value[2] >> 5;
			}
		}
	}
	catch (out_of_range& err) {
		printf("An error occured. Out Of range.");
		cout << err.what() << '\n';
		cout << "------------------------------" << "\n";
	}
}