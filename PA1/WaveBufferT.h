#pragma once

#include <future>
#include "ThreadCounter.h"

#ifndef WaveBufferT_H
#define WaveBufferT_H

class WaveBufferT
{
public:

	std::thread wb_thread;
	int t_id;
	ThreadCounter* counter;
	char* buffer;
	bool empty;
	std::condition_variable cv_play;
	std::condition_variable* cv_wb_done;
	std::promise<void> wv_out_done_promise;
	HWAVEOUT* hWaveOut;
	WAVEHDR header;
	std::future<void>* kill_future;

	WaveBufferT() = default;
	WaveBufferT(const WaveBufferT& o) = delete;
	WaveBufferT& operator=(const WaveBufferT& o) = delete;
	~WaveBufferT();

	WaveBufferT(int id,
		ThreadCounter* wb_counter, 
		std::future<void>* wb_kill_future,
		std::condition_variable* cv_wb_done,
		HWAVEOUT* hWaveOut);

	void operator () ();

	std::shared_future<void> resetPromise();
	void prepareWaveOutHdr();
};

#endif 
