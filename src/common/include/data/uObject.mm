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

const wchar_t TypeNull[] = L"Undefined::";
const wchar_t TypeObject[] = L"Object::";
const wchar_t TypeAtomicObject[] = L"Atomic::";
const wchar_t TypeStack[] = L"Stack::";
const wchar_t TypeIterator[] = L"Iterator::";
const wchar_t TypeListIterator[] = L"ListIterator::";
const wchar_t TypeCollection[] = L"Collection::";
const wchar_t TypeList[] = L"List::";
const wchar_t TypeSet[] = L"Set::";
const wchar_t TypeHashSet[] = L"HashSet::";
const wchar_t TypeLinkedHashSet[] = L"LinkedHashSet::";

void RefObject::_threadLock() const {
#if defined(__IOS__)
	void* thread = (__bridge void *)([NSThread currentThread]);
#endif
	
	do {
		_spinLock();
		if ((AtomicCompareExchangePtr((void* volatile *)_thread, thread, NULL) == NULL) ||
			(AtomicCompareExchangePtr((void* volatile *)_thread, thread, thread) == thread)) {
			AtomicIncrement((volatile int32_t*)&_lockCount);
			_spinUnlock();
			break;
		}
		_spinUnlock();
		sleep(0);
	} while (true);
}

bool RefObject::_threadLockTry() const {
#if defined(__IOS__)
	void* thread = (__bridge void *)([NSThread currentThread]);
#endif
	
	_spinLock();
	if ((AtomicCompareExchangePtr((void* volatile *)_thread, thread, NULL) == NULL) ||
		(AtomicCompareExchangePtr((void* volatile *)_thread, thread, thread) == thread)) {
		AtomicIncrement((volatile int32_t*)&_lockCount);
		_spinUnlock();
		return true;
	}
	_spinUnlock();
	return false;
}

void RefObject::_threadUnlock() const {
#if defined(__IOS__)
	void* thread = (__bridge void *)([NSThread currentThread]);
#endif
	
	_spinLock();
	if (AtomicDecrement((volatile int32_t*)&_lockCount) == 1) {
		AtomicCompareExchangePtr((void* volatile *)_thread, NULL, thread);
	}
	_spinUnlock();
}

bool RefObject::equals(const Object& object) const {
	return (this == object._object);
}

bool RefObject::equals(const void* object) const {
	return (this == object);
}

String RefObject::toString() const {
	return String::format(L"0x%08X", (int)(uint64_t)this);
}

String RefObject::getClass() const {
	return TYPE;
}

bool RefObject::wait(int milis, int nanos) const {
#if defined(__IOS__)
	void* thread = (__bridge void *)([NSThread currentThread]);
#else
	#error Crossplatform Object not finished
#endif
	
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
#if defined(__IOS__)
	void* thread = (__bridge void *)([NSThread currentThread]);
#else
	#error Crossplatform Object not finished
#endif
	
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

	return THIS->getClass();
}

String Object::toString() const {
	if (_object == NULL)
		return NULL;
	
	return THIS->toString();
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
				newObject = memNew(newObject, RefObject());
				if (newObject == NULL)
					throw eOutOfMemory;
			} catch (...) {
				AtomicExchangePtr((void* volatile*)&_object, (void*)NULL);
				if (o != NULL) {
					memDelete(o);
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
		memDelete(o);
	}
}

void Object::setRef(const void* object) {
	RefObject* newObject = (RefObject*)object;
	RefObject* prevObject = (RefObject*)AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
	if (prevObject != newObject) {
		if (prevObject != NULL) {
			if (AtomicDecrement(&(prevObject->_retainCount)) == 1) {
				prevObject->finalize();
				memDelete(prevObject);
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
				newObject = memNew(newObject, RefObject());
				if (newObject == NULL)
					throw eOutOfMemory;
			} catch (...) {
				if (o != NULL) {
					memDelete(o);
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
				memDelete(prevObject);
			}
		}
		if (newObject != NULL)
			AtomicIncrement(&(newObject->_retainCount));
	}
	if (o != NULL) {
		memDelete(o);
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
				newObject = memNew(newObject, RefObject());
				if (newObject == NULL)
					throw eOutOfMemory;
			} catch (...) {
				AtomicExchangePtr((void* volatile*)&_object, (void*)NULL);
				if (o != NULL) {
					memDelete(o);
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
		memDelete(o);
	}
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
				newObject = memNew(newObject, RefObject());
				if (newObject == NULL)
					throw eOutOfMemory;
			} catch (...) {
				if (o != NULL) {
					memDelete(o);
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
				memDelete(prevObject);
			}
		}
		if (newObject != NULL) {
			AtomicIncrement(&(newObject->_retainCount));
		}
	}
	if (o != NULL) {
		memDelete(o);
	}
	return *this;
}

Object& Object::operator =(const Object& object) {
	RefObject* newObject = (RefObject*)(object._object);
	RefObject* prevObject = (RefObject*)AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
	if (prevObject != newObject) {
		if (prevObject != NULL) {
			if (AtomicDecrement(&(prevObject->_retainCount)) == 1) {
				prevObject->finalize();
				memDelete(prevObject);
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
			memDelete(prevObject);
		}
	}
}
