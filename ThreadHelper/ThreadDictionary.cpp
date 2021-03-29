//----------------------------------------------------------------------------
// Copyright 2019, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------

#include "ThreadDictionary.h"

ThreadDictionary::ThreadDictionary() 
	: poThreadNames(),
	 mtx()
{
}

ThreadDictionary::~ThreadDictionary()
{
	this->mtx.lock();

	// Since Map owns name... you need to release the memory
	for (auto it = poThreadNames.begin(); it != poThreadNames.end(); ++it)
	{
		delete it->second;
	}

	this->mtx.unlock();
}

char * ThreadDictionary::GetName(unsigned int threadId)
{
	this->mtx.lock();

	auto it = poThreadNames.find(threadId);
	char *s = nullptr;

	if (it == poThreadNames.end())
	{
		s = nullptr;
	}
	else
	{
		s = it->second;
	}

	this->mtx.unlock();

	return s;
}

void ThreadDictionary::AddToMap(const char * threadName, unsigned int threadId)
{
	this->mtx.lock();

	size_t N = strlen(threadName) + 1;
	char *p = new char[N];
	memset(p, 0x0, N);
	strcpy_s(p, N, threadName);

	// The name is now OWNED by the Map
	auto it = poThreadNames.emplace(threadId, p);

	if (it.second == false)
	{
		// duplicate entry... no transfer
		// undo allocation

		delete p;
	}

	this->mtx.unlock();
}

// End of file