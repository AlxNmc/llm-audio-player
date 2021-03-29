#include "CircularQueue.h"

CircularQueue::CircularQueue()
	:front_index(0), back_index(0), empty(true), full(false), size(0), mask(CIRCULAR_QUE_SIZE - 1)
{
	queue = new int[CIRCULAR_QUE_SIZE];
}

CircularQueue::~CircularQueue()
{
	delete[] queue;
}

bool CircularQueue::pushBack(int t_id)
{
	std::lock_guard<std::mutex> lock(m);
	bool success = false;
	if (front_index != back_index || empty)
	{
		queue[back_index] = t_id;
		back_index = (back_index + 1) & mask;

		if (back_index == front_index)
		{
			full = true;
		}

		success = true;
		empty = false;
	}
	return success;
}

int CircularQueue::popFront() // returns -1 on fail
{
	std::lock_guard<std::mutex> lock(m);
	int t_id = -1;
	if (!empty)
	{
		t_id = queue[front_index];
		front_index = (front_index + 1) & mask;

		if (front_index == back_index)
		{
			empty = true;
		}

		full = false;
	}
	return t_id;
}

bool CircularQueue::isEmpty()
{
	return empty;
}