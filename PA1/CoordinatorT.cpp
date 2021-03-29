#include "ThreadHelper.h"
#include "FileT.h"
#include "ThreadCounter.h"
#include "PlaybackT.h"
#include "CoordinatorT.h"



CoordinatorT::CoordinatorT(ThreadCounter* counter, 
	FileT* fileT, 
	PlaybackT* playbackT, 
	std::condition_variable* cv_done, 
	std::shared_future<void> start_future)

	:thread_counter(counter), 
	fileTobj(fileT),
	playbackTobj(playbackT),
	cv_done(cv_done),
	empty_buffer(new char[512 * 1024]), e_size(0),
	full_buffer(new char[512 * 1024]), f_size(0),
	both_full(false),
	start_future(start_future)
{
	coordinator_thread = std::thread(std::ref(*this));
}

void CoordinatorT::operator()()
{
	thread_counter->increment();
	ThreadHelper::SetCurrentThreadName("--Coordinator Thread--");

	// copy first file buffer when file thread is ready
	start_future.get();
	f_size = fileTobj->loadBuffer(full_buffer);

	do 
	{
		playBuffers();
		swap_buffers();
	} while (!fileTobj->done);

	// play from second buffer before exiting
	both_full = true;
	playBuffers();

	cv_done->notify_one();

	// wait for all threads to finish before exiting
	thread_counter->decrement();
	std::mutex count_mtx;
	std::unique_lock<std::mutex> count_lock(count_mtx);
	thread_counter->cv_count.wait(count_lock, [&]() { return thread_counter->count == 0; });
}

void CoordinatorT::swap_buffers()
{
	char* temp = empty_buffer;
	empty_buffer = full_buffer;
	full_buffer = temp;
	f_size = e_size;
	both_full = false;
}

void CoordinatorT::playBuffers()
{
	// get number of iterations from block size
	int n = f_size / (2048);
	int t_id = -1;
	int wb_size = 2048;
	char* next_block = full_buffer;
	WaveBufferT* wb;

	// account for non-2k remainder
	if (f_size % (2048) != 0) 
	{
		n++;
	}

	for (int i = 0; i < n; i++)
	{
		if (!both_full && !fileTobj->empty) 
		{
			e_size = fileTobj->loadBuffer(empty_buffer);
			both_full = true;
		}

		t_id = playbackTobj->empty_queue->popFront();
		// if no items in queue - try again
		if (t_id < 0) 
		{
			std::unique_lock<std::mutex> dry_lock(playbackTobj->wb_mutex);
			playbackTobj->cv_wb.wait(dry_lock, [&] {return !playbackTobj->empty_queue->empty; });
			i--;
		}
		else 
		{
			wb = playbackTobj->wave_buffers[t_id];

			// get correct block size
			wb_size = f_size > wb_size ? wb_size : f_size;

			// clear wave buffer
			memset(wb->buffer, 0x0, 2048);

			// load wave buffer
			memcpy(wb->buffer, next_block, wb_size);

			// push to full queue
			playbackTobj->full_queue->pushBack(t_id);

			// adjust remaining block size and next block pointer
			next_block += wb_size;
			f_size -= wb_size;
		}
	}
}

CoordinatorT::~CoordinatorT()
{
	if (coordinator_thread.joinable()) 
	{
		coordinator_thread.join();
	}

	delete[] empty_buffer;
	delete[] full_buffer;
}
