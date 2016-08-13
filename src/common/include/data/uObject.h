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
#include <core/uError.h>

#define synchronized(var)   (var)->wait();

#define THIS (*this)
#define CHECKTHIS { if (this->_object == NULL) throw eNullPointer; }

#define RefClass(Base, Class, ...) \
	virtual inline Ref ## Class, ## __VA_ARGS__* newRef() const throw(const char*) { \
		Ref ## Class, ## __VA_ARGS__* o = memNew(o, Ref ## Class, ## __VA_ARGS__()); \
		if (o == NULL) throw eOutOfMemory; \
		return o; \
	} \
	inline Base(const void* object) { \
		this->initialize(); \
		*((Object*)this) = object; \
	} \
	inline Base(const Object& object) { \
		this->initialize(); \
		*((Object*)this) = object; \
	} \
	virtual inline Class, ## __VA_ARGS__& operator =(const void* object) { \
		*((Object*)this) = object; \
		return *this; \
	} \
	virtual inline Class, ## __VA_ARGS__& operator =(const Object& object) { \
		*((Object*)this) = object; \
		return *this; \
	} \
	inline Base(const Ref ## Class, ## __VA_ARGS__& object) { \
		this->initialize(); \
		this->setRef(&object); \
	} \
	virtual inline Class, ## __VA_ARGS__& operator =(const Ref ## Class, ## __VA_ARGS__& object) { \
		this->setRef(&object); \
		return *this; \
	} \
	inline Base(const Class, ## __VA_ARGS__* object) throw(const char*) { \
		this->initialize(); \
		if (object != NULL) { \
			if (object->_object == NULL) { \
				try { \
					this->setRef(object->newRef()); \
				} catch (...) { \
					this->setRef(NULL); \
					memDelete(object); \
					throw; \
				} \
			} else { \
				this->setRef((void*)(object->_object)); \
			} \
			memDelete(object); \
		} \
	} \
	inline Class, ## __VA_ARGS__& operator =(const Class, ## __VA_ARGS__* object) throw(const char*) { \
		if (object != NULL) { \
			if (object->_object == NULL) { \
				try { \
					this->setRef(object->newRef()); \
				} catch (...) { \
					this->setRef(NULL); \
					memDelete(object); \
					throw; \
				} \
			} else { \
				this->setRef((void*)(object->_object)); \
			} \
			memDelete(object); \
		} \
		return *this; \
	} \
	inline Base() { this->initialize(); } \
	virtual inline Ref ## Class, ## __VA_ARGS__* operator->() const { \
		return (Ref ## Class, ## __VA_ARGS__*)(this->_object); \
	} \
	virtual inline Ref ## Class, ## __VA_ARGS__& reference() const { \
		return *(Ref ## Class, ## __VA_ARGS__*)(this->_object); \
	}

#define RefCallback(Base, Class, ...) \
	virtual inline Ref ## Class, ## __VA_ARGS__* newRef() const throw(const char*) { \
		Ref ## Class, ## __VA_ARGS__* o = memNew(o, Ref ## Class, ## __VA_ARGS__()); \
		if (o == NULL) throw eOutOfMemory; \
		return o; \
	} \
	inline Base(const void* object) { \
		this->initialize(); \
		*((Object*)this) = object; \
	} \
	inline Base(const Object& object) { \
		this->initialize(); \
		*((Object*)this) = object; \
	} \
	virtual inline Class, ## __VA_ARGS__& operator =(const void* object) { \
		*((Object*)this) = object; \
		return *this; \
	} \
	virtual inline Class, ## __VA_ARGS__& operator =(const Object& object) { \
		*((Object*)this) = object; \
		return *this; \
	} \
	inline Base(const Ref ## Class, ## __VA_ARGS__& object) { \
		this->initialize(); \
		this->setRef(&object); \
	} \
	virtual inline Class, ## __VA_ARGS__& operator =(const Ref ## Class, ## __VA_ARGS__& object) { \
		this->setRef(&object); \
		return *this; \
	} \
	inline Base(const Class, ## __VA_ARGS__* object) throw(const char*) { \
		this->initialize(); \
		if (object != NULL) { \
			if (object->_object == NULL) { \
				try { \
					this->setRef(object->newRef()); \
				} catch (...) { \
					this->setRef(NULL); \
					memDelete(object); \
					throw; \
				} \
			} else { \
				this->setRef((void*)(object->_object)); \
			} \
			memDelete(object); \
		} \
	} \
	inline Class, ## __VA_ARGS__& operator =(const Class, ## __VA_ARGS__* object) throw(const char*) { \
		if (object != NULL) { \
			if (object->_object == NULL) { \
				try { \
					this->setRef(object->newRef()); \
				} catch (...) { \
					this->setRef(NULL); \
					memDelete(object); \
					throw; \
				} \
			} else { \
				this->setRef((void*)(object->_object)); \
			} \
			memDelete(object); \
		} \
		return *this; \
	} \
	inline Base() { this->initialize(); } \
	virtual inline Ref ## Class, ## __VA_ARGS__* operator->() const { \
		return (Ref ## Class, ## __VA_ARGS__*)(this->_object); \
	} \
	inline Base(const Ref ## Class, ## __VA_ARGS__* c) { this->initialize(); setRef(c); }

class String;
class Object;

extern const wchar_t TypeNull[];
extern const wchar_t TypeObject[];
extern const wchar_t TypeAtomicObject[];
extern const wchar_t TypeStack[];
extern const wchar_t TypeIterator[];
extern const wchar_t TypeListIterator[];
extern const wchar_t TypeCollection[];
extern const wchar_t TypeList[];
extern const wchar_t TypeSet[];
extern const wchar_t TypeHashSet[];
extern const wchar_t TypeLinkedHashSet[];

class RefObject {
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
	
	inline RefObject() : TYPE(TypeObject) { }
	
	virtual inline ~RefObject() {}
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
	volatile RefObject* _object = NULL;
	
	inline void* operator new(size_t size) throw(const char*) { void *p = memAlloc(void, p, size); if (!p) throw eOutOfMemory; return p; }
	inline void* operator new[](size_t size) throw(const char*) { void *p = memAlloc(void, p, size); if (!p) throw eOutOfMemory; return p; }
	inline void operator delete(void* p) { memFree(p); }
	inline void operator delete[](void* p) { memFree(p); }

	virtual ~Object();
	inline Object() { initialize(); }
	
	Object(const Object& object);
	Object& operator =(const Object& object);
	
	inline bool operator ==(const Object& object) const { return (_object == object._object); }
	inline bool operator !=(const Object& object) const { return (_object != object._object); }
	
	// support for 'null' value
	Object(const void* object) throw(const char*);
	Object& operator =(const void* object) throw(const char*);
	inline bool operator ==(const void* object) const { return (_object == object); }
	inline bool operator !=(const void* object) const { return (_object != object); }
	
	// support for 'new' value
	Object(const Object* object) throw(const char*);
	Object& operator =(const Object* object) throw(const char*);
	
	virtual inline RefObject* operator->() const { return (RefObject*)_object; }
	
	void setRef(const void* object);
	
	// Android
	
	inline bool equals(const Object& object) const throw(const char*) { CHECKTHIS; return THIS->equals(object); }
	inline bool equals(const void* object) const throw(const char*) { CHECKTHIS; return THIS->equals(object); }
	String getClass() const;
	virtual inline uint32_t hashCode() const { if (_object == NULL) return 0; return THIS->hashCode(); }
	inline void notify() const throw(const char*) { CHECKTHIS; THIS->notify(); }
	inline void notifyAll() const throw(const char*) { CHECKTHIS; THIS->notifyAll(); }
	String toString() const;
	inline void wait() const throw(const char*) { CHECKTHIS; THIS->wait(); }
	inline bool wait(int milis, int nanos) const throw(const char*) { CHECKTHIS; return THIS->wait(milis, nanos); }
	inline bool wait(int milis) const throw(const char*) { CHECKTHIS; return THIS->wait(milis); }

};

#endif //JAPPSY_UOBJECT_H