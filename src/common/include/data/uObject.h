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

#define synchronized(var)   (var)->wait();

#define THIS (*this)

#define ClassRef(base, class, ...) \
	virtual inline class ## Ref, ## __VA_ARGS__* newRef() const { return new class ## Ref, ## __VA_ARGS__(); } \
	inline base(const void* object) { \
		initialize(); \
		*((Object*)this) = object; \
	} \
	inline base(const Object& object) { \
		initialize(); \
		*((Object*)this) = object; \
	} \
	virtual inline class, ## __VA_ARGS__& operator =(const void* object) { \
		*((Object*)this) = object; \
		return *this; \
	} \
	virtual inline class, ## __VA_ARGS__& operator =(const Object& object) { \
		*((Object*)this) = object; \
		return *this; \
	} \
	inline base(const class ## Ref, ## __VA_ARGS__& object) { \
		initialize(); \
		this->setRef(&object); \
	} \
	virtual inline class, ## __VA_ARGS__& operator =(const class ## Ref, ## __VA_ARGS__& object) { \
		this->setRef(&object); \
		return *this; \
	} \
	inline base(const class, ## __VA_ARGS__* object) { \
		initialize(); \
		if (object != NULL) { \
			if (object->_object == NULL) { \
				this->setRef(object->newRef()); \
			} else { \
				this->setRef((void*)(object->_object)); \
			} \
			memDelete(object); \
		} \
	} \
	inline class, ## __VA_ARGS__& operator =(const class, ## __VA_ARGS__* object) { \
		if (object != NULL) { \
			if (object->_object == NULL) { \
				this->setRef(object->newRef()); \
			} else { \
				this->setRef((void*)(object->_object)); \
			} \
			memDelete(object); \
		} \
		return *this; \
	} \
	inline base() { initialize(); } \
	virtual inline class ## Ref, ## __VA_ARGS__* operator->() const { \
		return (class ## Ref, ## __VA_ARGS__*)(this->_object); \
	} \
	virtual inline class ## Ref, ## __VA_ARGS__& reference() const { \
		return *(class ## Ref, ## __VA_ARGS__*)(this->_object); \
	}

#define CallbackRef(base, class, ...) \
	virtual inline class ## Ref, ## __VA_ARGS__* newRef() const { return new class ## Ref, ## __VA_ARGS__(); } \
	inline base(const void* object) { \
		initialize(); \
		*((Object*)this) = object; \
	} \
	inline base(const Object& object) { \
		initialize(); \
		*((Object*)this) = object; \
	} \
	virtual inline class, ## __VA_ARGS__& operator =(const void* object) { \
		*((Object*)this) = object; \
		return *this; \
	} \
	virtual inline class, ## __VA_ARGS__& operator =(const Object& object) { \
		*((Object*)this) = object; \
		return *this; \
	} \
	inline base(const class ## Ref, ## __VA_ARGS__& object) { \
		initialize(); \
		this->setRef(&object); \
	} \
	virtual inline class, ## __VA_ARGS__& operator =(const class ## Ref, ## __VA_ARGS__& object) { \
		this->setRef(&object); \
		return *this; \
	} \
	inline base(const class, ## __VA_ARGS__* object) { \
		initialize(); \
		if (object != NULL) { \
			if (object->_object == NULL) { \
				this->setRef(object->newRef()); \
			} else { \
				this->setRef((void*)(object->_object)); \
			} \
			memDelete(object); \
		} \
	} \
	inline class, ## __VA_ARGS__& operator =(const class, ## __VA_ARGS__* object) { \
		if (object != NULL) { \
			if (object->_object == NULL) { \
				this->setRef(object->newRef()); \
			} else { \
				this->setRef((void*)(object->_object)); \
			} \
			memDelete(object); \
		} \
		return *this; \
	} \
	inline base() { initialize(); } \
	virtual inline class ## Ref, ## __VA_ARGS__* operator->() const { \
		return (class ## Ref, ## __VA_ARGS__*)(this->_object); \
	} \
	inline base(const class ## Ref, ## __VA_ARGS__* c) { initialize(); setRef(c); }

class String;
class Object;

extern const wchar_t TypeObject[];

class ObjectRef {
protected:
	const wchar_t* TYPE;
	
	volatile void* _thread = 0;
	volatile int32_t _lock = 0;
	volatile int32_t _lockCount = 0;

	virtual inline void _spinLock() const { AtomicLock((volatile int32_t*)&_lock); }
	virtual inline bool _spinLockTry() const { return AtomicLockTry((volatile int32_t*)&_lock); }
	virtual inline void _spinUnlock() const { AtomicUnlock((volatile int32_t*)&_lock); }

	virtual void _threadLock() const;
	virtual bool _threadLockTry() const;
	virtual void _threadUnlock() const;
	
public:
	volatile int32_t _retainCount = 0;
	
	inline ObjectRef() : TYPE(TypeObject) { }
	
	virtual inline ~ObjectRef() {}
	virtual inline void finalize() {}
	
	virtual bool equals(const Object& object) const;
	virtual bool equals(const void* object) const;
	
	virtual inline uint32_t hashCode() const { return (uint32_t)((intptr_t)this); }
	virtual String toString() const;
	
	inline void* operator new(size_t size) throw(const char*) { void *p = memAlloc(void, p, size); if (!p) throw eOutOfMemory; return p; }
	inline void* operator new[](size_t size) throw(const char*) { void *p = memAlloc(void, p, size); if (!p) throw eOutOfMemory; return p; }
	inline void operator delete(void* p) { memFree(p); }
	inline void operator delete[](void* p) { memFree(p); }
	
	inline void lock() const { _threadLock(); }
	inline bool lockTry() const { return _threadLockTry(); }
	inline void unlock() const { _threadUnlock(); }
	
	inline bool operator ==(const Object& object) const { return equals(object); }
	inline bool operator !=(const Object& object) const { return !equals(object); }
	inline bool operator ==(const void* object) const { return equals(object); }
	inline bool operator !=(const void* object) const { return !equals(object); }
	
	// virtual inline Object clone() const { return Object(this); }
	
	String getClass() const;
	inline void notify() const { _threadUnlock(); }
	inline void notifyAll() const { _threadUnlock(); }
	inline void wait() const { _threadLock(); }
	bool wait(int milis, int nanos) const;
	bool wait(int milis) const;
};

class Object {
protected:
	volatile int32_t _lock = 0;
	
	virtual inline void _spinLock() const { AtomicLock((volatile int32_t*)&_lock); }
	virtual inline bool _spinLockTry() const { return AtomicLockTry((volatile int32_t*)&_lock); }
	virtual inline void _spinUnlock() const { AtomicUnlock((volatile int32_t*)&_lock); }

	virtual inline void initialize() {}
public:
	volatile ObjectRef* _object = NULL;
	
	inline void* operator new(size_t size) throw(const char*) { void *p = memAlloc(void, p, size); if (!p) throw eOutOfMemory; return p; }
	inline void* operator new[](size_t size) throw(const char*) { void *p = memAlloc(void, p, size); if (!p) throw eOutOfMemory; return p; }
	inline void operator delete(void* p) { memFree(p); }
	inline void operator delete[](void* p) { memFree(p); }

	virtual inline ~Object() {
		ObjectRef* prevObject = (ObjectRef*)AtomicExchangePtr((void* volatile*)&_object, (void*)NULL);
		if (prevObject != NULL) {
			if (OSAtomicDecrement32((int32_t*)&(prevObject->_retainCount)) == 0) {
				prevObject->finalize();
				memDelete(prevObject);
			}
		}
	}
	
	inline Object() { initialize(); }
	
	inline Object(const Object& object) {
		initialize();
		ObjectRef* newObject = (ObjectRef*)(object._object);
		AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
		if (newObject != NULL) {
			OSAtomicIncrement32((int32_t*)&(newObject->_retainCount));
		}
	}
	
	inline Object& operator =(const Object& object) {
		ObjectRef* newObject = (ObjectRef*)(object._object);
		ObjectRef* prevObject = (ObjectRef*)AtomicExchangePtr((void* volatile*)&_object, (void*)newObject);
		if (prevObject != newObject) {
			if (prevObject != NULL) {
				if (OSAtomicDecrement32((int32_t*)&(prevObject->_retainCount)) == 0) {
					prevObject->finalize();
					memDelete(prevObject);
				}
			}
			if (newObject != NULL) {
				OSAtomicIncrement32((int32_t*)&(newObject->_retainCount));
			}
		}
		return *this;
	}
	
	inline bool operator ==(const Object& object) const { return (_object == object._object); }
	inline bool operator !=(const Object& object) const { return (_object != object._object); }
	
	// support for 'null' value
	Object(const void* object);
	Object& operator =(const void* object);
	inline bool operator ==(const void* object) const { return (_object == object); }
	inline bool operator !=(const void* object) const { return (_object != object); }
	
	// support for 'new' value
	Object(const Object* object);
	Object& operator =(const Object* object);
	
	virtual inline ObjectRef* operator->() const { return (ObjectRef*)_object; }
	
	void setRef(const void* object);
	
	// Android
	
	inline bool equals(const Object& object) const { return THIS->equals(object); }
	inline bool equals(const void* object) const { return THIS->equals(object); }
	String getClass() const;
	virtual inline uint32_t hashCode() const { if (_object == NULL) return 0; return THIS->hashCode(); }
	inline void notify() const { THIS->notify(); }
	inline void notifyAll() const { THIS->notifyAll(); }
	String toString() const;
	inline void wait() const { THIS->wait(); }
	inline bool wait(int milis, int nanos) const { return THIS->wait(milis, nanos); }
	inline bool wait(int milis) const { return THIS->wait(milis); }

};

#endif //JAPPSY_UOBJECT_H