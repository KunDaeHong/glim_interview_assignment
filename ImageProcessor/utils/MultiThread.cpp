#include "MultiThread.h"

void MultiThread::addThread(
	function<void(const ImageBuffer& target, ImageBuffer& output, int sRow, int eRow)> workFunc,
	const ImageBuffer& target,
	ImageBuffer& output, 
	int sRow, 
	int eRow) 
{
	int threadCnt = coreCount();

	if (first_work) {
		m_workers.reserve(threadCnt);
	}

	if (m_workers.size() >= threadCnt) {
		join();
		first_work = false;
	}
	else {
		m_workers.emplace_back(workFunc, cref(target), ref(output), sRow, eRow);
	}
}

void MultiThread::add_thread_convolution(
	function<void(const uint8_t* targetPtr, uint8_t* outputPtr, int width, int height, int sRow, int eRow, int padding, int stride)> workFunc,
	const uint8_t* targetPtr, 
	uint8_t* outputPtr,
	int width,
	int height, 
	int sRow,
	int eRow, 
	int padding,
	int stride) 
{
	int threadCnt = coreCount();

	if (first_work) {
		m_workers.reserve(threadCnt);
	}

	if (m_workers.size() >= threadCnt) {
		join();
		first_work = false;
	}
	else {
		m_workers.emplace_back(workFunc, cref(targetPtr), ref(outputPtr), width, height, sRow, eRow, padding, stride);
	}
}

void MultiThread::join() {
	for (thread& th : m_workers) {
		if (th.joinable()) {
			th.join();
		}
	}
	m_workers.clear();
}
