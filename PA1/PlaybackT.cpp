#include <future>
#include "ThreadHelper.h"
#include "ThreadCounter.h"
#include "PlaybackT.h"

PlaybackT::PlaybackT(ThreadCounter* counter, std::future<void>* kill_future)
	:thread_counter(counter),
	kill_future(kill_future),
	play_counter(new ThreadCounter()),
	empty_queue(new CircularQueue()),
	full_queue(new CircularQueue()),
	wb_counter(new ThreadCounter())
{
	//start thread
	playback_thread = std::thread(std::ref(*this));

}

void PlaybackT::operator()()
{
	thread_counter->increment();
	ThreadHelper::SetCurrentThreadName("--Playback Thread--");

	std::future<void> wb_kill_future = wb_kill_promise.get_future();

	//create wave buffer objects
	prepareWaveOut();
	for (int i = 0; i < 20; i++) 
	{
		wave_buffers[i] = new WaveBufferT(i, wb_counter, &wb_kill_future, &cv_wb, &hWaveOut);
		wb_futures[i] = wave_buffers[i]->resetPromise();
		empty_queue->pushBack(i);
	}

	do
	{
		// migrate any empty buffers to empty queue
		loadEmpty();
		// signal coordinator to fill buffers
		cv_wb.notify_one();
		// initiate playback for any buffers in full_queue
		dumpFull();
	} while (!kill_future->_Is_ready());

	closeWBthreads();
	closePlayback();
}

void PlaybackT::loadEmpty()
{
	for (int i = 0; i < 20; i++) 
	{
		if (wb_futures[i]._Is_ready()) 
		{
			wb_futures[i].wait();
			wb_futures[i] = wave_buffers[i]->resetPromise();
			empty_queue->pushBack(i);
		}
	}
}

void PlaybackT::dumpFull()
{

	for (int id = full_queue->popFront(); id >= 0; id = full_queue->popFront()) 
	{
		wave_buffers[id]->empty = false;
		wave_buffers[id]->cv_play.notify_one();
		play_counter->increment();
	}
}

void PlaybackT::closeWBthreads()
{
	// wait for all to stop playing
	std::mutex wbKill_mtx;
	std::unique_lock<std::mutex> wb_play_lock(wbKill_mtx);
	play_counter->cv_count.wait(wb_play_lock, [&]() {return play_counter->count == 0; });

	// nudge threads to exit wait state and check kill condition
	wb_kill_promise.set_value();
	for (int i = 0; i < 20; i++) 
	{
		wave_buffers[i]->empty = false;
		wave_buffers[i]->cv_play.notify_one();
	}

	// wait for all to finish
	wb_counter->cv_count.wait(wb_play_lock, [&]() { return wb_counter->count == 0; });
}

void PlaybackT::prepareWaveOut()
{

	wfx.nSamplesPerSec = 22050;
	wfx.wBitsPerSample = 16;
	wfx.nChannels = 2; 
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nBlockAlign = (wfx.wBitsPerSample >> 3)* wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
	wfx.cbSize = 0;

	result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)waveOutCallBack, (DWORD_PTR)this, CALLBACK_FUNCTION);
	assert(result == 0);

}

void PlaybackT::closePlayback()
{
	// wait for all threads to finish before exiting
	thread_counter->decrement();
	std::mutex count_mtx;
	std::unique_lock<std::mutex> count_lock(count_mtx);
	thread_counter->cv_count.wait(count_lock, [&]() { return thread_counter->count == 0; });
}

void PlaybackT::waveOutCallBack(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	UNUSED_VAR(hWaveOut);
	UNUSED_VAR(dwParam1);
	UNUSED_VAR(dwParam2);

	PlaybackT* pbThread = (PlaybackT*)dwInstance;
	WAVEHDR* waveHdr = 0;
	WaveBufferT* wbThread = 0;

	switch (uMsg)
	{
	case WOM_DONE:
		waveHdr = (WAVEHDR*)dwParam1;
		wbThread = (WaveBufferT*)waveHdr->dwUser;

		//signal that playback is done
		pbThread->play_counter->decrement();
		wbThread->wv_out_done_promise.set_value();
		wbThread->cv_wb_done->notify_one();
		break;

	case WOM_CLOSE:
		break;

	case WOM_OPEN:
		break;

	default:
		assert(false);
	}
}



PlaybackT::~PlaybackT()
{
	if (playback_thread.joinable()) 
	{
		playback_thread.join();
	}

	for (int i = 0; i < 20; i++) 
	{
		delete wave_buffers[i];
	}

	delete empty_queue;
	delete full_queue;
	delete wb_counter;
	delete play_counter;
}

