//----------------------------------------------------------------------------
// Copyright 2019, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------
#include <thread>
#include <future>

#include "ThreadHelper.h"
#include "File_Slow.h"
#include "FileT.h"
#include "ThreadCounter.h"
#include "PlaybackT.h"
#include "CoordinatorT.h"

int main()
{
	ThreadCounter*			t_counter = new ThreadCounter();
	std::promise<void>		kill_promise;
	std::future<void>		kill_future = kill_promise.get_future();
	std::condition_variable cv_CT_done;
	std::promise<void>		playback_start_promise;
	std::shared_future<void> playback_start_future(playback_start_promise.get_future());

	// Create thread objects
	FileT*			file_thread = new FileT(t_counter, std::move(playback_start_promise));
	PlaybackT*		playback_thread = new PlaybackT(t_counter, &kill_future);
	CoordinatorT*	coordinator_thread = new CoordinatorT(t_counter, file_thread, playback_thread, &cv_CT_done, playback_start_future);

	// Wait for coordinator thread to finish
	std::mutex mtx_f;
	std::unique_lock<std::mutex> fileT_lock(mtx_f);
	cv_CT_done.wait(fileT_lock, [&]() { return file_thread->done; });

	// Fulfill kill promise
	kill_promise.set_value();

	// Wait for thread count to reach 0
	std::mutex mtx_c;
	std::unique_lock<std::mutex> count_lock(mtx_c);
	t_counter->cv_count.wait(count_lock, [&]() { return t_counter->count == 0; });

	// Clean up
	delete coordinator_thread;
	delete playback_thread;
	delete file_thread;
	delete t_counter;
}
