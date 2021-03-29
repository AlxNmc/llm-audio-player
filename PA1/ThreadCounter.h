#pragma once

#ifndef ThreadCounter_H
#define ThreadCounter_H

struct ThreadCounter
{
	ThreadCounter();
	void increment();
	void decrement();

	std::condition_variable cv_count;
	int count;
	std::mutex mtx;
};

#endif
