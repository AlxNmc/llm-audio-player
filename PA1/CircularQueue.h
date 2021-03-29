#pragma once

#ifndef CircularQueue_H
#define CircularQueue_H

class CircularQueue
{
public:
	static const int CIRCULAR_QUE_SIZE = 32; // must be a power of 2

	CircularQueue();
	CircularQueue(const CircularQueue& o) = delete;
	CircularQueue& operator=(const CircularQueue& o) = delete;
	~CircularQueue();

	bool pushBack(int t_id);
	int popFront();
	bool isEmpty();

	int front_index;
	int back_index;
	bool empty;
	std::mutex m;

private:

	int* queue;
	bool full;
	int size;
	int mask;
};

#endif // !CircularQueue_H

