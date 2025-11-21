#pragma once
#include "Types.h"

/*=============
* Read-Write Spin Lock
* [상위 16비트][하위 16비트]
*  상위 16비트 : 현재 쓰기 락을 건 스레드의 ID
*  하위 16비트 : 현재 읽기 락을 건 스레드의 수

* Write Lock 을 잡은 상태에서 Read Lock 획득은 가능
*  단, Read Lock 을 잡은 상태에서 Write Lock 획득은 불가능
*  Read Lock은 여러 스레드가 동시에 획득 하고 있을 수도 있기 때문
==============*/

class Lock
{
	enum : uint32 {
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000,
		READ_COUNT_MASK = 0x0000'FFFF,
		EMPTY_FLAG = 0x0000'0000
	};

public:
	void WirteLock(const char* name);
	void WriteUnlock(const char* name);
	void ReadLock(const char* name);
	void ReadUnlock(const char* name);

private:
	Atomic<uint32> _lockFlag = EMPTY_FLAG;
	uint16 _writeCount = 0; // 락의 재귀적 사용을 위한 카운트 -> 어차피 동일 스레드만 접근 가능 하기 때문에 Atomic 불필요
};

/*=============
* Lock Guards
==============*/

class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock, const char* name)
		: _lock(lock), _name(name)
	{
		_lock.ReadLock(_name);
	}
	~ReadLockGuard()
	{
		_lock.ReadUnlock(_name);
	}
private:
	Lock& _lock;
	const char* _name;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock, const char* name)
		: _lock(lock), _name(name)
	{
		_lock.WirteLock(_name);
	}
	~WriteLockGuard()
	{
		_lock.WriteUnlock(_name);
	}
private:
	Lock& _lock;
	const char* _name;
};
