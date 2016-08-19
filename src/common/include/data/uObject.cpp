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
const wchar_t TypeObject[] = L"Object::";
const wchar_t TypeNumber[] = L"Number::";
const wchar_t TypeBoolean[] = L"Number::Boolean::";
const wchar_t TypeByte[] = L"Number::Byte::";
const wchar_t TypeShort[] = L"Number::Short::";
const wchar_t TypeInt[] = L"Number::Int::";
const wchar_t TypeLong[] = L"Number::Long::";
const wchar_t TypeFloat[] = L"Number::Float::";
const wchar_t TypeDouble[] = L"Number::Double::";
const wchar_t TypeString[] = L"String::";
const wchar_t TypeJSONObject[] = L"JSONObject::";
const wchar_t TypeJSONArray[] = L"JSONArray::";
const wchar_t TypeAtomicObject[] = L"Atomic::";
const wchar_t TypeStack[] = L"Stack::";
const wchar_t TypeIterator[] = L"Iterator::";
const wchar_t TypeListIterator[] = L"ListIterator::";
const wchar_t TypeCollection[] = L"Collection::";
const wchar_t TypeList[] = L"List::";
const wchar_t TypeSet[] = L"Set::";
const wchar_t TypeHashSet[] = L"HashSet::";
const wchar_t TypeLinkedHashSet[] = L"LinkedHashSet::";
const wchar_t TypeMap[] = L"Map::";
const wchar_t TypeHashMap[] = L"HashMap::";
const wchar_t TypeLinkedHashMap[] = L"LinkedHashMap::";
const wchar_t TypeSparseArray[] = L"SparseArray::";

void RefObject::_threadLock() const {
	void* thread = CurrentThreadId();

	do {
		_spinLock();
		if ((AtomicCompareExchangePtr((void**)&_thread, thread, NULL) == NULL) ||
			(AtomicCompareExchangePtr((void**)&_thread, thread, thread) == thread)) {
			AtomicIncrement(&(((RefObject*)this)->_lockCount));
			_spinUnlock();
			break;
		}
		_spinUnlock();
		sleep(0);
	} while (true);
}

bool RefObject::_threadLockTry() const {
	void* thread = CurrentThreadId();
	
	_spinLock();
	if ((AtomicCompareExchangePtr((void**)&_thread, thread, NULL) == NULL) ||
		(AtomicCompareExchangePtr((void**)&_thread, thread, thread) == thread)) {
		AtomicIncrement(&(((RefObject*)this)->_lockCount));
		_spinUnlock();
		return true;
	}
	_spinUnlock();
	return false;
}

void RefObject::_threadUnlock() const {
	void* thread = CurrentThreadId();
	
	_spinLock();
	if (AtomicDecrement((volatile int32_t*)&_lockCount) == 1) {
		AtomicCompareExchangePtr((void**)&_thread, NULL, thread);
	}
	_spinUnlock();
}

bool RefObject::equals(const Object& object) const {
	return ((void*)this == (void*)(object._object));
}

bool RefObject::equals(const void* object) const {
	return (this == object);
}

String RefObject::toString() const {
	return String::format(L"0x%08X", (int)(uint64_t)this);
}

String RefObject::toJSON() const {
	return L"{}";
}

String RefObject::getClass() const {
	return TYPE;
}

bool RefObject::wait(int milis, int nanos) const {
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

bool RefObject::wait(int milis) const {
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

String Object::getClass() const {
	if (_object == NULL)
		return TypeNull;

	return THIS.ref().getClass();
}

String Object::toString() const {
	if (_object == NULL)
		return NULL;
	
	return THIS.ref().toString();
}

String Object::toJSON() const {
	if (_object == NULL)
		return L"null";
	
	return THIS.ref().toJSON();
}


Object::Object(const void* object) throw(const char*) {
	initialize();
	Object* o = (Object*)object;
	RefObject* newObject;
	if (o != NULL) {
		if (o->_object != NULL) {
			newObject = (RefObject*)(o->_object);
		} else {
			try {
				newObject = new RefObject();
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

void Object::setRef(const void* object) {
	RefObject* newObject = (RefObject*)object;
	RefObject* prevObject = (RefObject*)AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
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

Object& Object::operator =(const void* object) throw(const char*) {
	Object* o = (Object*)object;
	RefObject* newObject;
	if (o != NULL) {
		if (o->_object != NULL) {
			newObject = (RefObject*)(o->_object);
		} else {
			try {
				newObject = new RefObject();
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
	RefObject* prevObject = (RefObject*)AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
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

Object::Object(const Object* object) throw(const char*) {
	initialize();
	Object* o = (Object*)object;
	RefObject* newObject;
	if (o != NULL) {
		if (o->_object != NULL) {
			newObject = (RefObject*)(o->_object);
		} else {
			try {
				newObject = new RefObject();
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

Object::Object(const RefObject* object) throw(const char*) {
	initialize();
	RefObject* newObject = (RefObject*)object;
	AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
	if (newObject != NULL)
		AtomicIncrement(&(newObject->_retainCount));
}

Object::Object(const Object& object) {
	initialize();
	RefObject* newObject = (RefObject*)(object._object);
	AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
	if (newObject != NULL) {
		AtomicIncrement(&(newObject->_retainCount));
	}
}

Object& Object::operator =(const Object* object) throw(const char*) {
	Object* o = (Object*)object;
	RefObject* newObject;
	if (o != NULL) {
		if (o->_object != NULL) {
			newObject = (RefObject*)(o->_object);
		} else {
			try {
				newObject = new RefObject();
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
	RefObject* prevObject = (RefObject*)AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
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

Object& Object::operator =(const RefObject* object) throw(const char*) {
	THIS.setRef(object);
	return *this;
}

Object& Object::operator =(const Object& object) {
	RefObject* newObject = (RefObject*)(object._object);
	RefObject* prevObject = (RefObject*)AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
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

Object::~Object() {
	RefObject* prevObject = (RefObject*)AtomicExchangePtr((void* volatile*)&_object, (void*)NULL);
	if (prevObject != NULL) {
		if (AtomicDecrement(&(prevObject->_retainCount)) == 1) {
			prevObject->finalize();
			delete prevObject;
		}
	}
}

bool Object::isNull(const RefString& string) {
	return (wchar_t*)string == NULL;
}

bool Object::isNull(const RefString* string) {
	return (string == NULL) ? true : ((wchar_t*)(*string) == NULL);
}

uint32_t Object::hashCode(const wchar_t* string) throw(const char*) {
	if (string == NULL)
		return 0;
	
	uint32_t size = 0;
	wcs_strlen(string, &size);
	return mmcrc32(0xFFFFFFFF, (void*)string, size);
}

uint32_t Object::hashCode(const char* string) throw(const char*) {
	if (string == NULL)
		return 0;
	
	uint32_t size = strlen(string) + 1;
	return mmcrc32(0xFFFFFFFF, (void*)string, size);
}
	
void Object::log() const {
	String json = JSON::stringify(*this);
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

