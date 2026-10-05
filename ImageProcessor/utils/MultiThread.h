#pragma once

#include <thread>
#include <vector>
#include <functional>
#include "ImageBuffer.h"

using namespace ip;
using namespace std;

class ThreadPolicy
{
public:
	virtual ~ThreadPolicy() {}
	virtual int coreCount() final {
		unsigned int cores = thread::hardware_concurrency();
		return cores;
	}
	virtual void join() {};
};

class MultiThread : public ThreadPolicy {
private:
	bool first_work = false;
	vector<thread> m_workers;
public:

	virtual ~MultiThread() {
		join();
	}

	void addThread(function<void(const ImageBuffer& target, ImageBuffer& output, int sRow, int eRow)> workFunc, const ImageBuffer& target, ImageBuffer& output, int sRow, int eRow);

	void add_thread_convolution(function<void(const uint8_t* targetPtr, uint8_t* outputPtr, int width, int height, int sRow, int eRow, int padding, int stride)> workFunc, const uint8_t* targetPtr, uint8_t* outputPtr, int width, int height, int sRow, int eRow, int padding = 1, int stride = 1);

	void join() override;

};