#include "WaveBufferT.h"
#include <ThreadHelper.h>

WaveBufferT::WaveBufferT(int id,
	ThreadCounter* wb_counter, 
	std::future<void>* wb_kill_future,
	std::condition_variable* cv_wb_done,
	HWAVEOUT* hWaveOut)

	:t_id(id),
	counter(wb_counter),
	buffer(new char[2048]),
	empty(true),
	cv_wb_done(cv_wb_done),
	kill_future(wb_kill_future),
	hWaveOut(hWaveOut)
{
	wb_thread = std::thread(std::ref(*this));
}

void WaveBufferT::operator()()
{
	counter->increment();
	char name[] = "--Wave Buffer Thread 0------";
	sprintf_s(name + 21, 5, "%d--", t_id);
	ThreadHelper::SetCurrentThreadName(name);

	std::mutex mtx_play;

	prepareWaveOutHdr();
	while (true) 
	{
		// wait for cv notify/!empty
		std::unique_lock<std::mutex> lock(mtx_play);
		cv_play.wait(lock, [&]() {return !empty; });
		// check kill future
		if (kill_future->_Is_ready())
		{
			break;
		}
		empty = true;
		// play from buffer
		waveOutWrite(*hWaveOut, &header, sizeof(WAVEHDR));
	}
	counter->decrement();
}

std::shared_future<void> WaveBufferT::resetPromise()
{
	wv_out_done_promise = std::promise<void>();
	return wv_out_done_promise.get_future();
}

void WaveBufferT::prepareWaveOutHdr()
{
	DWORD blockSize = 2 * 1024;/* holds the size of the block */


	memset(&header, 0x0, sizeof(WAVEHDR));
	header.dwBufferLength = blockSize;
	header.lpData = buffer;
	header.dwUser = (DWORD_PTR)this;

	waveOutPrepareHeader(*hWaveOut, &header, sizeof(WAVEHDR));
}

WaveBufferT::~WaveBufferT()
{
	if (wb_thread.joinable()) 
	{
		wb_thread.join();
	}

	delete[] buffer;

}

