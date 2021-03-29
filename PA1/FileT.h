#pragma once
#include <future>
#include"ThreadCounter.h"

#ifndef FileT_H
#define FileT_H

class FileT
{
public:
	std::thread file_thread;

	char*	file_buffer;
	int		block_size;
	bool	empty;
	bool	done;

	// playback initializatioon
	std::promise<void> start_promise;

	// coordinator coordination
	std::mutex file_mtx;

	// main thread exit coordination
	ThreadCounter* thread_counter;

	FileT() = delete;
	FileT(ThreadCounter* counter, std::promise<void>&& start_promise);
	FileT(const FileT& o) = default;
	FileT& operator=(const FileT& o) = delete;
	~FileT();

	void operator() ();
	int loadBuffer(char* dest);
};

#endif