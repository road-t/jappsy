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

#ifndef JAPPSY_UATOMICOBJECT_H
#define JAPPSY_UATOMICOBJECT_H

#include <data/uObject.h>

template <typename Type>
class RefAtomic : public JRefObject {
private:
	Type* m_value;

public:

	inline RefAtomic() throw(const char*) {
		TYPE = TypeAtomicObject;
		m_value = new Type();
		if (m_value == NULL)
			throw eOutOfMemory;
	}
	
	inline ~RefAtomic() { delete m_value; }
	
	inline Type get() {
		Type res;
		_spinLock();
		res = Type(*m_value);
		_spinUnlock();
		return res;
	}
	
	inline void set(const Type& newValue) {
		_spinLock();
		*m_value = newValue;
		_spinUnlock();
	}
	
	inline Type getAndSet(const Type& newValue) {
		Type res;
		_spinLock();
		res = Type(*m_value);
		*m_value = newValue;
		_spinUnlock();
		return res;
	}
	
	inline bool compareAndSet(const Type& compareValue, const Type& newValue) {
		bool res = false;
		_spinLock();
		if (*m_value == compareValue) {
			*m_value = newValue;
			res = true;
		}
		_spinUnlock();
		return res;
	}
};

template <typename Type>
class AtomicReference : public JObject {
public:
	virtual inline RefAtomic<Type>* newRef() const throw(const char*) {
		RefAtomic<Type>* o = new RefAtomic<Type>();
		if (o == NULL) throw eOutOfMemory;
		return o;
	}
	
	inline AtomicReference() { }
	inline AtomicReference(const void* object) throw(const char*) { THIS.setRef(newRef()); }
	inline AtomicReference(const Type& initialValue) throw(const char*) { THIS.setRef(newRef()); THIS.ref().set(initialValue);	}
	inline AtomicReference(const AtomicReference<Type>* object) throw(const char*) {
		if (object != NULL) {
			if (object->_object == NULL) {
				try {
					THIS.setRef(object->newRef());
				} catch (...) {
					delete object;
					throw;
				}
			} else {
				THIS.setRef((void*)(object->_object));
			}
			delete object;
		}
	}
	virtual inline AtomicReference<Type>& operator =(const AtomicReference<Type>& object) {
		THIS.setRef((void*)(object._object));
		return *this;
	}
	virtual inline RefAtomic<Type>& ref() const throw(const char*) {
		if (_object == NULL) throw eInvalidPointer;
		return *((RefAtomic<Type>*)(THIS._object));
	}
	
	inline Type get() throw(const char*) {  return THIS.ref().get(); }
	inline void set(const Type& newValue) throw(const char*) {  THIS.ref().set(newValue); }
	inline Type getAndSet(const Type& newValue) throw(const char*) {  return THIS.ref().getAndSet(newValue); }
	inline bool compareAndSet(const Type& compareValue, const Type& newValue) throw(const char*) {  return THIS.ref().compareAndSet(compareValue, newValue); }
};

class AtomicObject : public JObject {
public:
	virtual inline RefAtomic<JObject>* newRef() const throw(const char*) {
		RefAtomic<JObject>* o = new RefAtomic<JObject>();
		if (o == NULL) throw eOutOfMemory;
		return o;
	}
	inline AtomicObject() { }
	inline AtomicObject(const void* object) throw(const char*) { THIS.setRef(newRef()); }
	inline AtomicObject(const JObject& initialValue) throw(const char*) { THIS.setRef(newRef()); THIS.ref().set(initialValue); }
	inline AtomicObject(const AtomicObject* object) throw(const char*) {
		if (object != NULL) {
			if (object->_object == NULL) {
				try {
					THIS.setRef(object->newRef());
				} catch (...) {
					delete object;
					throw;
				}
			} else {
				THIS.setRef((void*)(object->_object));
			}
			delete object;
		}
	}
	virtual inline AtomicObject& operator =(const AtomicObject& object) {
		THIS.setRef((void*)(object._object));
		return *this;
	}
	virtual inline RefAtomic<JObject>& ref() const throw(const char*) {
		return *((RefAtomic<JObject>*)(THIS._object));
	}
	
	inline JObject get() throw(const char*) { return THIS.ref().get(); }
	inline void set(const JObject& newValue) throw(const char*) { THIS.ref().set(newValue); }
	inline JObject getAndSet(const JObject& newValue) throw(const char*) { return THIS.ref().getAndSet(newValue); }
	inline bool compareAndSet(const JObject& compareValue, const JObject& newValue) throw(const char*) { return THIS.ref().compareAndSet(compareValue, newValue); }
};

typedef AtomicReference<bool>       AtomicBoolean;
typedef AtomicReference<int32_t>    AtomicInteger;
typedef AtomicReference<int64_t>    AtomicLong;

#endif //JAPPSY_UATOMICOBJECT_H