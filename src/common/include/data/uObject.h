/*
 * Copyright (C) 2016 The Jappsy Open Source Project (http://jappsy.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef JAPPSY_UOBJECT_H
#define JAPPSY_UOBJECT_H

#include <platform.h>
#include <core/uMemory.h>
#include <core/uAtomic.h>

class CString;

class CObject {
protected:
	volatile void* m_thread = 0;
	volatile bool m_lock = false;
	volatile int32_t m_lockCount = 0;
	
	inline void spinLock() const { AtomicLock(&m_lock); }
	inline bool spinLockTry() const { return AtomicLockTry(&m_lock); }
	inline void spinUnlock() const { AtomicUnlock(&m_lock); }
	
	void threadLock() const;
	bool threadLockTry() const;
	void threadUnlock() const;
	
public:
	inline void* operator new(size_t size) throw(const char*) { void *p = memAlloc(void, p, size); if (!p) throw eOutOfMemory; return p; }
	inline void* operator new[](size_t size) throw(const char*) { void *p = memAlloc(void, p, size); if (!p) throw eOutOfMemory; return p; }
	inline void operator delete(void* p) { memFree(p); }
	inline void operator delete[](void* p) { memFree(p); }
	
	inline void lock() const { threadLock(); }
	inline bool lockTry() const { return threadLockTry(); }
	inline void unlock() const { threadUnlock(); }
	
	virtual inline uint32_t hashCode() const { return (uint32_t)((intptr_t)this); }
	virtual CString toString() const;
	virtual CString toJSON() const;

	void log() const;
};

#endif //JAPPSY_UOBJECT_H