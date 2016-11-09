#pragma once
#include <process.h>
#include <queue>
#include <Windows.h>

class SimpleLock
{
public:
	SimpleLock(){ ::InitializeCriticalSection(&cs_); }
	~SimpleLock(){ ::DeleteCriticalSection(&cs_); }

	void Lock(){ ::EnterCriticalSection(&cs_); }
	void Unlock(){ ::LeaveCriticalSection(&cs_); }

private:
	CRITICAL_SECTION cs_;
};

class SimpleAutoLock
{
public:
	SimpleAutoLock(SimpleLock* lock):lock_(lock){ lock_->Lock(); }
	~SimpleAutoLock(){ lock_->Unlock(); }

private:
	SimpleLock* lock_;
};

class SimpleThread {
	UINT32 m_iThreadID;
	uintptr_t hThread;
	
public:
	bool start()
	{ 
		hThread = _beginthreadex(NULL, 0, SimpleThread::run, (void*)this, 0, &m_iThreadID); 
		return hThread ? true : false;
	}

	virtual void run() = 0;
private:
	static unsigned int WINAPI run(PVOID pParam) 
	{
		reinterpret_cast<SimpleThread*>(pParam)->run();
		return 0;
	}
};

template <class T>
class SimpleQueue{
public:
	SimpleQueue()
	{
		InitializeConditionVariable(&m_cond);
		InitializeCriticalSection(&m_lock);
	}	

	~SimpleQueue()
	{
		::DeleteCriticalSection(&m_lock);
	}

	void pushWithSignal(T t) 
	{
		EnterCriticalSection(&m_lock);
		m_queue.push(t);
		LeaveCriticalSection(&m_lock);
		WakeConditionVariable(&m_cond);
	}

	T popWithCond(void)
	{
		EnterCriticalSection(&m_lock);
		while(m_queue.empty()) {
			SleepConditionVariableCS(&m_cond, &m_lock, INFINITE);
		}
		T t = m_queue.front();
		m_queue.pop();
		LeaveCriticalSection(&m_lock);
		return t;
	}

	void clear() 
	{
		EnterCriticalSection(&m_lock);
		//m_queue.clear();
		LeaveCriticalSection(&m_lock);
	}

private:
	std::queue<T> m_queue;
	CRITICAL_SECTION m_lock;
	CONDITION_VARIABLE m_cond;
};