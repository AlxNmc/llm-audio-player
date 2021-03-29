//----------------------------------------------------------------------------
// Copyright 2019, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------

#ifndef THREAD_DICTIONARY
#define THREAD_DICTIONARY

#pragma warning( push )
#pragma warning( disable : 4625 )
#pragma warning( disable : 4626 )
#include <mutex>
#include <map>
#pragma warning( pop )

class ThreadDictionary
{
public:
	ThreadDictionary();
	ThreadDictionary(const ThreadDictionary &) = delete;
	ThreadDictionary & operator = (const ThreadDictionary &) = delete;

	~ThreadDictionary();
	char *GetName(unsigned int threadId);
	void AddToMap(const char * threadName, unsigned int threadId);

private:
	std::map<unsigned int, char *> poThreadNames;
	std::mutex mtx;
};


#endif
