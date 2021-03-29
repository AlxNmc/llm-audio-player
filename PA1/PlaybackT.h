#pragma once
#include "CircularQueue.h"
#include "WaveBufferT.h"

#ifndef PlaybackT_H
#define PlaybackT_H

class PlaybackT
{
public:
	std::thread playback_thread;

	// main thread exit coordination
	ThreadCounter*				thread_counter;
	std::future<void>*			kill_future;

	// wb data structures
	WaveBufferT*				wave_buffers[20];
	std::shared_future<void>	wb_futures[20];
	CircularQueue*				empty_queue;
	CircularQueue*				full_queue;

	// wb coordination
	std::mutex					wb_mutex;
	std::condition_variable		cv_wb;
	std::promise<void>			wb_kill_promise;
	ThreadCounter*				wb_counter;
	ThreadCounter*				play_counter;

	// waveout
	WAVEFORMATEX				wfx;
	MMRESULT					result;
	HWAVEOUT					hWaveOut;

	PlaybackT() = delete;
	PlaybackT(ThreadCounter* counter, std::future<void>* kill_future);
	PlaybackT(const PlaybackT& o) = delete;
	PlaybackT& operator=(const PlaybackT& o) = delete;
	~PlaybackT();

	void operator () ();

	void loadEmpty();
	void dumpFull();
	void closeWBthreads();
	void prepareWaveOut();
	void closePlayback();
	static void CALLBACK waveOutCallBack(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

};

#endif
