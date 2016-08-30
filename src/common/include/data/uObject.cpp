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

#include "uObject.h"
#include <core/uSystem.h>
#include <data/uString.h>
#include <data/uJSON.h>

void CObject::threadLock() const {
	void* thread = CurrentThreadId();
	
	do {
		spinLock();
		if ((AtomicCompareExchangePtr((void**)&m_thread, thread, NULL) == NULL) ||
			(AtomicCompareExchangePtr((void**)&m_thread, thread, thread) == thread)) {
			AtomicIncrement((volatile int32_t*)&m_lockCount);
			spinUnlock();
			break;
		}
		spinUnlock();
		usleep(0);
	} while (true);
}

bool CObject::threadLockTry() const {
	void* thread = CurrentThreadId();
	
	spinLock();
	if ((AtomicCompareExchangePtr((void**)&m_thread, thread, NULL) == NULL) ||
		(AtomicCompareExchangePtr((void**)&m_thread, thread, thread) == thread)) {
		AtomicIncrement((volatile int32_t*)&m_lockCount);
		spinUnlock();
		return true;
	}
	spinUnlock();
	return false;
}

void CObject::threadUnlock() const {
	void* thread = CurrentThreadId();
	
	spinLock();
	if (AtomicDecrement((volatile int32_t*)&m_lockCount) == 1) {
		AtomicCompareExchangePtr((void**)&m_thread, NULL, thread);
	}
	spinUnlock();
}

CString CObject::toString() const {
	return CString::format(L"0x%08X", (int)(uint64_t)this);
}

CString CObject::toJSON() const {
	return L"{}";
}

void CObject::log() const {
	/*
	CString json = JSON::stringify(*this);
	wchar_t* wstr = (wchar_t*)json;
	if (wstr == NULL)
		return;
	
	uint32_t size = wcs_toutf8_size(wstr);
	if (size == 0)
		return;
	
	char* str = memAlloc(char, str, size);
	if (str == NULL)
		return;

	wcs_toutf8(wstr, str, size);
	
	LOG("%s", str);
	
	memFree(str);
	 */
}

