#include "ThreadCounter.h"

ThreadCounter::ThreadCounter()
	:count(0)
{
}

void ThreadCounter::increment()
{
	std::lock_guard<std::mutex> lock(mtx);
	count++;
}

void ThreadCounter::decrement()
{
	std::lock_guard<std::mutex> lock(mtx);
	count--;
	if (count <= 0)
	{
		cv_count.notify_all();
	}
}
