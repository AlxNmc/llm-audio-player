#include <thread>

#include "ThreadHelper.h"
#include "FileT.h"
#include "File_Slow.h"

FileT::FileT(ThreadCounter* counter, std::promise<void>&& start_promise)
	:thread_counter(counter),
	start_promise(std::move(start_promise)),
	file_buffer(new char[512 * 1024]),
	block_size(0),
	empty(true),
	done(false)

{
	file_thread = std::thread(std::ref(*this));
}

void FileT::operator()()
{
	thread_counter->increment();
	ThreadHelper::SetCurrentThreadName("--File Thread--");

	File_Slow::Handle fh;
	File_Slow::Error error;
	char file_name[20];
	std::unique_lock<std::mutex> file_lock(file_mtx, std::defer_lock);

	sprintf_s(file_name, 20, "wave_");
	for (int i = 0; i < 23; i++)
	{
		// adjust file name for current index
		sprintf_s(file_name + 5, 15, "%d.wav", i);

		// read from file
		file_lock.lock();
		error = File_Slow::open(fh, file_name, File_Slow::READ);
		assert(error == File_Slow::SUCCESS);
		error = File_Slow::seek(fh, File_Slow::Seek::END, 0);
		assert(error == File_Slow::SUCCESS);
		error = File_Slow::tell(fh, block_size);
		assert(error == File_Slow::SUCCESS);
		error = File_Slow::seek(fh, File_Slow::Seek::BEGIN, 0);
		assert(error == File_Slow::SUCCESS);
		error = File_Slow::read(fh, file_buffer, block_size);
		assert(error == File_Slow::SUCCESS);

		// mark flag that buffer is full
		empty = false;
		file_lock.unlock();

		// start playback after first file read
		if (i==0) 
		{
			start_promise.set_value();
		}

		// wait for buffer to be read
		while (!empty) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		// close file
		error = File_Slow::close(fh);
		//assert(error == File_Slow::SUCCESS);
	}

	// notify main that all files are read
	done = true;

	// wait for all threads to finish before exiting
	thread_counter->decrement();
	std::mutex count_mtx;
	std::unique_lock<std::mutex> count_lock(count_mtx);
	thread_counter->cv_count.wait(count_lock, [&]() { return thread_counter->count == 0; });
}

int FileT::loadBuffer(char* dest) // returns size of buffer
{
	//initiate lock guard on file mutex
	std::lock_guard<std::mutex> buff_guard(file_mtx);
	
	// load block into empty buffer
	memcpy(dest, file_buffer, block_size);

	// notify waiting thread to continue
	empty = true;

	return block_size;
}

FileT::~FileT()
{
	if (file_thread.joinable()) 
	{
		file_thread.join();
	}

	delete[] file_buffer;
}
