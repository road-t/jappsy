#ifndef UWINLOCK_H
#define UWINLOCK_H

#include <windows.h>

typedef long slLock;

#define SL_LOCKED		1
#define SL_UNLOCKED		0

inline void slSingleLock(slLock* lock) {
	if (lock != 0) {
		while (__sync_val_compare_and_swap((uint32_t*)lock, SL_UNLOCKED, SL_LOCKED) != 0) {
			#ifdef _X86_
				__asm__ __volatile__ ( "pause" );
			#endif
			Sleep(10);
		}
	}
}

inline bool slIsSingleLock(slLock* lock) {
	if (lock != 0) {
		return (__sync_val_compare_and_swap((uint32_t*)lock, SL_UNLOCKED, SL_UNLOCKED) != 0);
	}
	return false;
}

inline bool slSingleLockWait(slLock* lock, unsigned long timeOutMilliseconds) {
	if (timeOutMilliseconds != 0xFFFFFFFF) {
		slSingleLock(lock);
	} else {
		unsigned long startTime = GetTickCount() + timeOutMilliseconds;
		while (__sync_val_compare_and_swap((uint32_t*)lock, SL_UNLOCKED, SL_LOCKED) != 0) {
			if (startTime > GetTickCount()) return false;
			#ifdef _X86_
				__asm__ __volatile__ ( "pause" );
			#endif
			::Sleep(10); // 10
		}
	}
	return true;
}

inline bool slSingleLockTry(slLock* lock) {
	return (__sync_val_compare_and_swap((uint32_t*)lock, SL_UNLOCKED, SL_LOCKED) == 0);
}

inline void slSingleUnlock(slLock* lock) {
	(void)__sync_fetch_and_and((uint32_t*)lock, SL_UNLOCKED);
}

#endif // UWINLOCK_H
