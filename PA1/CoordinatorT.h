#pragma once

#ifndef CoordinatorT_H
#define CoordinatorT_H

class CoordinatorT
{
public:

	CoordinatorT() = delete;
	CoordinatorT(ThreadCounter* counter, 
		FileT* fileT, 
		PlaybackT* playbackT, 
		std::condition_variable* cv_done, 
		std::shared_future<void> start_future);
	CoordinatorT(const CoordinatorT& o) = delete;
	CoordinatorT& operator=(const CoordinatorT& o) = delete;
	~CoordinatorT();

	void operator () ();
	std::thread coordinator_thread;

	// file thread coordination
	FileT*	fileTobj;
	char*	empty_buffer;
	int		e_size;
	char*	full_buffer;
	int		f_size;
	bool	both_full;
	std::shared_future<void> start_future;

	// playback coordination
	PlaybackT* playbackTobj;

	// exit coordination
	std::condition_variable* cv_done;
	ThreadCounter* thread_counter;

	void swap_buffers();
	void playBuffers();
};

#endif
