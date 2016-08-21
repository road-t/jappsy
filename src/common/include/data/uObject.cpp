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

const wchar_t TypeNull[] = L"Undefined::";
const wchar_t TypeObject[] = L"JObject::";
const wchar_t TypeNumber[] = L"Number::";
const wchar_t TypeBoolean[] = L"Number::Boolean::";
const wchar_t TypeByte[] = L"Number::Byte::";
const wchar_t TypeShort[] = L"Number::Short::";
const wchar_t TypeInt[] = L"Number::Int::";
const wchar_t TypeLong[] = L"Number::Long::";
const wchar_t TypeFloat[] = L"Number::Float::";
const wchar_t TypeDouble[] = L"Number::Double::";
const wchar_t TypeString[] = L"JString::";
const wchar_t TypeJSONObject[] = L"JSONObject::";
const wchar_t TypeJSONArray[] = L"JSONArray::";
const wchar_t TypeAtomicObject[] = L"Atomic::";
const wchar_t TypeStack[] = L"JStack::";
const wchar_t TypeIterator[] = L"JIterator::";
const wchar_t TypeListIterator[] = L"JListIterator::";
const wchar_t TypeCollection[] = L"JCollection::";
const wchar_t TypeList[] = L"List::";
const wchar_t TypeSet[] = L"Set::";
const wchar_t TypeHashSet[] = L"HashSet::";
const wchar_t TypeLinkedHashSet[] = L"LinkedHashSet::";
const wchar_t TypeMap[] = L"Map::";
const wchar_t TypeHashMap[] = L"HashMap::";
const wchar_t TypeLinkedHashMap[] = L"LinkedHashMap::";
const wchar_t TypeSparseArray[] = L"JSparseArray::";

void CObject::_threadLock() const {
	void* thread = CurrentThreadId();
	
	do {
		_spinLock();
		if ((AtomicCompareExchangePtr((void**)&_thread, thread, NULL) == NULL) ||
			(AtomicCompareExchangePtr((void**)&_thread, thread, thread) == thread)) {
			AtomicIncrement(&(((CObject*)this)->_lockCount));
			_spinUnlock();
			break;
		}
		_spinUnlock();
		sleep(0);
	} while (true);
}

bool CObject::_threadLockTry() const {
	void* thread = CurrentThreadId();
	
	_spinLock();
	if ((AtomicCompareExchangePtr((void**)&_thread, thread, NULL) == NULL) ||
		(AtomicCompareExchangePtr((void**)&_thread, thread, thread) == thread)) {
		AtomicIncrement(&(((CObject*)this)->_lockCount));
		_spinUnlock();
		return true;
	}
	_spinUnlock();
	return false;
}

void CObject::_threadUnlock() const {
	void* thread = CurrentThreadId();
	
	_spinLock();
	if (AtomicDecrement((volatile int32_t*)&_lockCount) == 1) {
		AtomicCompareExchangePtr((void**)&_thread, NULL, thread);
	}
	_spinUnlock();
}

void JRefObject::_threadLock() const {
	void* thread = CurrentThreadId();

	do {
		_spinLock();
		if ((AtomicCompareExchangePtr((void**)&_thread, thread, NULL) == NULL) ||
			(AtomicCompareExchangePtr((void**)&_thread, thread, thread) == thread)) {
			AtomicIncrement(&(((JRefObject*)this)->_lockCount));
			_spinUnlock();
			break;
		}
		_spinUnlock();
		sleep(0);
	} while (true);
}

bool JRefObject::_threadLockTry() const {
	void* thread = CurrentThreadId();
	
	_spinLock();
	if ((AtomicCompareExchangePtr((void**)&_thread, thread, NULL) == NULL) ||
		(AtomicCompareExchangePtr((void**)&_thread, thread, thread) == thread)) {
		AtomicIncrement(&(((JRefObject*)this)->_lockCount));
		_spinUnlock();
		return true;
	}
	_spinUnlock();
	return false;
}

void JRefObject::_threadUnlock() const {
	void* thread = CurrentThreadId();
	
	_spinLock();
	if (AtomicDecrement((volatile int32_t*)&_lockCount) == 1) {
		AtomicCompareExchangePtr((void**)&_thread, NULL, thread);
	}
	_spinUnlock();
}

bool JRefObject::equals(const JObject& object) const {
	return ((void*)this == (void*)(object._object));
}

bool JRefObject::equals(const void* object) const {
	return (this == object);
}

JString JRefObject::toString() const {
	return JString::format(L"0x%08X", (int)(uint64_t)this);
}

JString JRefObject::toJSON() const {
	return L"{}";
}

JString JRefObject::getClass() const {
	return TYPE;
}

bool JRefObject::wait(int milis, int nanos) const {
	int usec = 0;
	int64_t start = 0;
	
	do {
		if (_threadLockTry())
			return true;
		
		if (start == 0) {
			usec = nanos / 1000;
			if (usec > 1000000) usec = 999999;
			start = currentTimeMillis();
		} else {
			int64_t interval = currentTimeMillis() - start;
			if (interval >= milis)
				return false;
		}
		
		usleep(usec);
	} while (true);
}

bool JRefObject::wait(int milis) const {
	int64_t start = 0;
	
	do {
		if (_threadLockTry())
			return true;
		
		if (start == 0) {
			start = currentTimeMillis();
		} else {
			int64_t interval = currentTimeMillis() - start;
			if (interval >= milis) {
				return false;
			}
		}
		
		usleep(0);
	} while (true);
}

JString JObject::getClass() const {
	if (_object == NULL)
		return TypeNull;

	return THIS.ref().getClass();
}

JString JObject::toString() const {
	if (_object == NULL)
		return NULL;
	
	return THIS.ref().toString();
}

JString JObject::toJSON() const {
	if (_object == NULL)
		return L"null";
	
	return THIS.ref().toJSON();
}


JObject::JObject(const void* object) throw(const char*) {
	initialize();
	JObject* o = (JObject*)object;
	JRefObject* newObject;
	if (o != NULL) {
		if (o->_object != NULL) {
			newObject = (JRefObject*)(o->_object);
		} else {
			try {
				newObject = new JRefObject();
				if (newObject == NULL)
					throw eOutOfMemory;
			} catch (...) {
				AtomicExchangePtr((void* volatile*)&_object, (void*)NULL);
				if (o != NULL) {
					delete o;
				}
				throw;
			}
		}
	} else {
		newObject = NULL;
	}
	AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
	if (newObject != NULL)
		AtomicIncrement(&(newObject->_retainCount));

	if (o != NULL) {
		delete o;
	}
}

void JObject::setRef(const void* object) {
	JRefObject* newObject = (JRefObject*)object;
	JRefObject* prevObject = (JRefObject*)AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
	if (prevObject != newObject) {
		if (prevObject != NULL) {
			if (AtomicDecrement(&(prevObject->_retainCount)) == 1) {
				prevObject->finalize();
				delete prevObject;
			}
		}
		if (newObject != NULL)
			AtomicIncrement(&(newObject->_retainCount));
	}
}

JObject& JObject::operator =(const void* object) throw(const char*) {
	JObject* o = (JObject*)object;
	JRefObject* newObject;
	if (o != NULL) {
		if (o->_object != NULL) {
			newObject = (JRefObject*)(o->_object);
		} else {
			try {
				newObject = new JRefObject();
				if (newObject == NULL)
					throw eOutOfMemory;
			} catch (...) {
				if (o != NULL) {
					delete o;
				}
				throw;
			}
		}
	} else {
		newObject = NULL;
	}
	JRefObject* prevObject = (JRefObject*)AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
	if (prevObject != newObject) {
		if (prevObject != NULL) {
			if (AtomicDecrement(&(prevObject->_retainCount)) == 1) {
				prevObject->finalize();
				delete prevObject;
			}
		}
		if (newObject != NULL)
			AtomicIncrement(&(newObject->_retainCount));
	}
	if (o != NULL) {
		delete o;
	}
	return *this;
}

JObject::JObject(const JObject* object) throw(const char*) {
	initialize();
	JObject* o = (JObject*)object;
	JRefObject* newObject;
	if (o != NULL) {
		if (o->_object != NULL) {
			newObject = (JRefObject*)(o->_object);
		} else {
			try {
				newObject = new JRefObject();
				if (newObject == NULL)
					throw eOutOfMemory;
			} catch (...) {
				AtomicExchangePtr((void* volatile*)&_object, (void*)NULL);
				if (o != NULL) {
					delete o;
				}
				throw;
			}
		}
	} else {
		newObject = NULL;
	}
	AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
	if (newObject != NULL)
		AtomicIncrement(&(newObject->_retainCount));

	if (o != NULL) {
		delete o;
	}
}

JObject::JObject(const JRefObject* object) throw(const char*) {
	initialize();
	JRefObject* newObject = (JRefObject*)object;
	AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
	if (newObject != NULL)
		AtomicIncrement(&(newObject->_retainCount));
}

JObject::JObject(const JObject& object) {
	initialize();
	JRefObject* newObject = (JRefObject*)(object._object);
	AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
	if (newObject != NULL) {
		AtomicIncrement(&(newObject->_retainCount));
	}
}

JObject& JObject::operator =(const JObject* object) throw(const char*) {
	JObject* o = (JObject*)object;
	JRefObject* newObject;
	if (o != NULL) {
		if (o->_object != NULL) {
			newObject = (JRefObject*)(o->_object);
		} else {
			try {
				newObject = new JRefObject();
				if (newObject == NULL)
					throw eOutOfMemory;
			} catch (...) {
				if (o != NULL) {
					delete o;
				}
				throw;
			}
		}
	} else {
		newObject = NULL;
	}
	JRefObject* prevObject = (JRefObject*)AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
	if (prevObject != newObject) {
		if (prevObject != NULL) {
			if (AtomicDecrement(&(prevObject->_retainCount)) == 1) {
				prevObject->finalize();
				delete prevObject;
			}
		}
		if (newObject != NULL) {
			AtomicIncrement(&(newObject->_retainCount));
		}
	}
	if (o != NULL) {
		delete o;
	}
	return *this;
}

JObject& JObject::operator =(const JRefObject* object) throw(const char*) {
	THIS.setRef(object);
	return *this;
}

JObject& JObject::operator =(const JObject& object) {
	JRefObject* newObject = (JRefObject*)(object._object);
	JRefObject* prevObject = (JRefObject*)AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
	if (prevObject != newObject) {
		if (prevObject != NULL) {
			if (AtomicDecrement(&(prevObject->_retainCount)) == 1) {
				prevObject->finalize();
				delete prevObject;
			}
		}
		if (newObject != NULL) {
			AtomicIncrement(&(newObject->_retainCount));
		}
	}
	return *this;
}

JObject::~JObject() {
	JRefObject* prevObject = (JRefObject*)AtomicExchangePtr((void* volatile*)&_object, (void*)NULL);
	if (prevObject != NULL) {
		if (AtomicDecrement(&(prevObject->_retainCount)) == 1) {
			prevObject->finalize();
			delete prevObject;
		}
	}
}

bool JObject::isNull(const CString& string) {
	return (wchar_t*)string == NULL;
}

bool JObject::isNull(const CString* string) {
	return (string == NULL) ? true : ((wchar_t*)(*string) == NULL);
}

uint32_t JObject::hashCode(const wchar_t* string) throw(const char*) {
	if (string == NULL)
		return 0;
	
	uint32_t size = 0;
	wcs_strlen(string, &size);
	return mmcrc32(0xFFFFFFFF, (void*)string, size);
}

uint32_t JObject::hashCode(const char* string) throw(const char*) {
	if (string == NULL)
		return 0;
	
	uint32_t size = strlen(string) + 1;
	return mmcrc32(0xFFFFFFFF, (void*)string, size);
}
	
void JObject::log() const {
	JString json = JSON::stringify(*this);
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
}

