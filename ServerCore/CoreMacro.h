#pragma once

// OUT 매그로 뒤의 변수가 변경 될 수 있음을 나타냄
#define OUT

/*---------------
	Lock Macro
---------------*/
#define USE_MANY_LOCK(count)	Lock _lock[count];
#define USE_LOCK				USE_MANY_LOCK(1);
#define READ_LOCK_IDX(idx)		ReadLockGuard _readLockGuard_##idx(_lock[idx], typeid(this).name());
#define READ_LOCK				READ_LOCK_IDX(0)
#define WRITE_LOCK_IDX(idx)		WriteLockGuard _writeLockGuard_##idx(_lock[idx], typeid(this).name());
#define WRITE_LOCK				WRITE_LOCK_IDX(0)


#ifdef _DEBUG
#define xxalloc(size)		StompAllocator::Alloc(size)
#define xxrelease(ptr)		StompAllocator::Release(ptr)
#else
#define xxalloc(size)		BaseAllocator::Alloc(size)
#define xxrelease(ptr)		BaseAllocator::Release(ptr)
#endif

/*---------------
	Crash Macro
---------------*/

#define CRASH(cause){									\
	uint32* crash = nullptr;							\
	__analysis_assume(crash != nullptr);				\
	*crash = 0xDEADBEEF;								\
}

#define ASSERT_CRASH(expr){								\
	if(!(expr)) {										\
		CRASH("ASSERT_CRASH");							\
		__analysis_assume(expr);						\
	}													\
}