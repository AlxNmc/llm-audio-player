//----------------------------------------------------------------------------
// Copyright 2019, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------

#ifndef THREAD_HELPER_H
#define THREAD_HELPER_H

#include "WindowsWrapper.h"
#include <thread>
#include "ThreadDictionary.h"

class ThreadHelper
{
public:
	ThreadHelper(const ThreadHelper&) = delete;
	ThreadHelper& operator = (const ThreadHelper &) = delete;

	static unsigned int GetCurrentID();
	static void SetThreadName(const std::thread &t0, const char * threadName);
	static void SetThreadName(DWORD threadId, const char* threadName);
	static void SetCurrentThreadName(const char * threadName);
	static const char * GetThreadName();

	static void DebugBegin(int numTabs);
	static void DebugEnd (int numTabs);

	static void TabMe(int numTabs);

	~ThreadHelper();
private:

	ThreadHelper();
	static ThreadHelper *privThreadHelper();

	ThreadDictionary tDictionary;
	std::mutex		 helperMtx;
};

#define UNUSED_VAR(x)  ((void )(x))

#endif