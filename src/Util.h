#pragma once
#include <windows.h>

void SetMainHWnd(HWND hWnd);
HWND GetMainHWnd();

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