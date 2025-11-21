#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

void Lock::WirteLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif
	// 동일한 스레드가 이미 쓰기락을 소유하고 있을때, 무조건 획득
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (lockThreadId == LThreadId) {
		_writeCount++;
		return;
	}
	// 아무도 소유 및 공유하고 있지 않을때, 경합해서 획득
	const int64 beginTick = ::GetTickCount64();

	// 누군가가 Read 락을 소유하고 있는 경우엔 쓰기 락 획득 불가
	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	while (true) {
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++) {
			uint32 expected = EMPTY_FLAG;
			// 락 플래그가 비어있다면 쓰기락을 현재 스레드가 획득
			if (_lockFlag.compare_exchange_weak(OUT expected, desired)) {
				_writeCount++;
				return;
			}
		}

		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK) {;
			CRASH("Lock::WirteLock Acquire Timeout");
		}

		this_thread::yield();
	}
}

void Lock::WriteUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif
	// Read Lock 을 다 풀기 전에는 Write Lock 을 풀 수 없음
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0) {
		CRASH("Lock::WriteUnlock Read Lock Still Acquired");
	}
	const int32 lockCount = _writeCount--;
	if (_writeCount == 0) {
		// 쓰기 락 해제
		_lockFlag.store(EMPTY_FLAG);
	}
}

void Lock::ReadLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif
	// 쓰기 락을 소유한 스레드가 동일 스레드라면, 무조건 획득
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (lockThreadId == LThreadId) {
		_lockFlag.fetch_add(1);
		return;
	}
	// 아무도 쓰기 락을 소유하고 있지 않을때, 경합해서 획득
	const int64 beginTick = ::GetTickCount64();
	while (true) {
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++) {
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			// 쓰기 락이 걸려있지 않다면 읽기 락 획득
			if (_lockFlag.compare_exchange_weak(OUT expected, expected + 1)) {
				return;
			}
		}

		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK) {
			CRASH("Lock::ReadLock Acquire Timeout");
		}
		this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif
	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0) {
		CRASH("Lock::ReadUnlock No Read Lock Acquired");
	}
}