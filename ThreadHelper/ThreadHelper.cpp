//----------------------------------------------------------------------------
// Copyright 2019, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------


#include "Framework.h"

#pragma warning( push )
#pragma warning( disable : 4625 )
#pragma warning( disable : 4626 )
#pragma warning( disable : 4774 )
#include <iostream>
#include <thread>
#include <sstream> 
#include <map>
#include <mutex>
#pragma warning( pop ) 


#include "WindowsWrapper.h"
#include "ThreadHelper.h"
#include "ThreadDictionary.h"

// ---------------------------------------------------------------
// Locally static section
//----------------------------------------------------------------
const DWORD MS_VC_EXCEPTION = 0x406D1388;


#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

ThreadHelper::~ThreadHelper()
{

}

ThreadHelper::ThreadHelper()
	: tDictionary(),
	helperMtx()
{

}

ThreadHelper * ThreadHelper::privThreadHelper()
{
	static ThreadHelper threadHelper;
	return &threadHelper;
}

// --------------------------------------------------------------
// Get the current thread ID
// --------------------------------------------------------------
unsigned int ThreadHelper::GetCurrentID()
{
	// Get the string ID in an Integer form
	std::stringstream ss;

	// extracts the id
	ss << std::this_thread::get_id();

	// convert from string to number
	unsigned int id = std::stoul(ss.str());

	return id;
}

// --------------------------------------------------------------
// Get the thread name at any time
// --------------------------------------------------------------
const char * ThreadHelper::GetThreadName()
{
	ThreadHelper *ptHelp = privThreadHelper();
	assert(ptHelp);

	// Get id of the current thread
	unsigned int threadId = ::GetThreadId(GetCurrentThread());

	return ptHelp->tDictionary.GetName(threadId);
}

// --------------------------------------------------------------
// Set the Thread Name in the Debugger
// Warning: Crazy shit... 
// --------------------------------------------------------------
void ThreadHelper::SetThreadName(const std::thread &t0, const char* threadName)
{
	ThreadHelper *ptHelp = privThreadHelper();
	assert(ptHelp);

	// Get the id

	// OMG - a const cast?
	// reason... if you have a thread reference... I want no way of modifying the data
	// but to get the name for the debugger, I need to call native_handle which is mutable

	DWORD threadId = ::GetThreadId(
		static_cast<HANDLE>(
							(const_cast<std::thread &>(t0)).native_handle()
							)
					);

	// register the name
	ptHelp->tDictionary.AddToMap(threadName, threadId);

	// Fill in the thread info for the debug
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = threadId;
	info.dwFlags = 0;

	// Throw an exception on purpose to jam the thread info into debugger
	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

// --------------------------------------------------------------
// Set the Thread Name in the Debugger
// Warning: Crazy shit... 
// --------------------------------------------------------------
void ThreadHelper::SetThreadName(DWORD threadId, const char* threadName)
{
	ThreadHelper *ptHelp = privThreadHelper();
	assert(ptHelp);

	// register the name
	ptHelp->tDictionary.AddToMap(threadName, threadId);

	// Fill in the thread info for the debug
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = threadId;
	info.dwFlags = 0;

	// Throw an exception on purpose to jam the thread info into debugger
	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}
// --------------------------------------------------------------
// Set the Thread Name in the Debugger
// Different for current thread (we can get the id without passing it)
// --------------------------------------------------------------
void ThreadHelper::SetCurrentThreadName(const char* threadName)
{
	ThreadHelper *ptHelp = privThreadHelper();
	assert(ptHelp);

	// Get id of the current thread
	unsigned int threadId = ::GetThreadId( GetCurrentThread() );

	// register the name
	ptHelp->tDictionary.AddToMap(threadName, threadId);

	// Fill in the thread info for the debug
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = threadId;
	info.dwFlags = 0;

	// Throw an exception on purpose to jam the thread info into debugger
	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

void ThreadHelper::DebugBegin(int numTabs)
{
	std::lock_guard< std::mutex >  lock(privThreadHelper()->helperMtx);

	switch (numTabs)
	{
	case 0:
		Trace::out("thread(%5d) %s: begin()\n", ThreadHelper::GetCurrentID(), ThreadHelper::GetThreadName());
		break;
	case 1:
		Trace::out("\tthread(%5d) %s: begin()\n", ThreadHelper::GetCurrentID(), ThreadHelper::GetThreadName());
		break;
	case 2:
		Trace::out("\t\tthread(%5d) %s: begin()\n", ThreadHelper::GetCurrentID(), ThreadHelper::GetThreadName());
		break;
	case 3:
		Trace::out("\t\t\tthread(%5d) %s: begin()\n", ThreadHelper::GetCurrentID(), ThreadHelper::GetThreadName());
		break;
	case 4:
		Trace::out("\t\t\t\tthread(%5d) %s: begin()\n", ThreadHelper::GetCurrentID(), ThreadHelper::GetThreadName());
		break;
	default:
		for (int i = 0; i < numTabs; i++)
		{
			Trace::out("\t");
		}
		Trace::out("thread(%d) %s: begin()\n", ThreadHelper::GetCurrentID(), ThreadHelper::GetThreadName());
		break;
	}
}

void ThreadHelper::TabMe(int numTabs)
{
	std::lock_guard< std::mutex >  lock(privThreadHelper()->helperMtx);

	for (int i = 0; i < numTabs; i++)
	{
		Trace::out("\t");
	}
}

void ThreadHelper::DebugEnd(int numTabs)
{
	std::lock_guard< std::mutex >  lock(privThreadHelper()->helperMtx);


	switch (numTabs)
	{
	case 0:
		Trace::out("thread(%5d) %s: end()\n", ThreadHelper::GetCurrentID(), ThreadHelper::GetThreadName());
		break;
	case 1:
		Trace::out("\tthread(%5d) %s: end()\n", ThreadHelper::GetCurrentID(), ThreadHelper::GetThreadName());
		break;
	case 2:
		Trace::out("\t\tthread(%5d) %s: end()\n", ThreadHelper::GetCurrentID(), ThreadHelper::GetThreadName());
		break;
	case 3:
		Trace::out("\t\t\tthread(%5d) %s: end()\n", ThreadHelper::GetCurrentID(), ThreadHelper::GetThreadName());
		break;
	case 4:
		Trace::out("\t\t\t\tthread(%5d) %s: end()\n", ThreadHelper::GetCurrentID(), ThreadHelper::GetThreadName());
		break;
	default:
		for (int i = 0; i < numTabs; i++)
		{
			Trace::out("\t");
		}
		Trace::out("thread(%d) %s: end()\n", ThreadHelper::GetCurrentID(), ThreadHelper::GetThreadName());
		break;
	}
}


// End of File