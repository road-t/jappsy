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

extern const wchar_t TypeAtomic[];

template <typename Type>
class AtomicRef : public ObjectRef {
private:
	Type* m_value;

public:

	inline AtomicRef() { TYPE = TypeAtomic; m_value = memNew(m_value, Type()); }
	inline ~AtomicRef() { memDelete(m_value); }
	
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
class AtomicReference : public Object {
public:
	virtual inline AtomicRef<Type>* newRef() const { AtomicRef<Type>* o = memNew(o, AtomicRef<Type>()); return o; }
	inline AtomicReference() { }
	inline AtomicReference(const void* object) { this->setRef(newRef()); }
	inline AtomicReference(const Type& initialValue) { this->setRef(newRef()); THIS->set(initialValue);	}
	inline AtomicReference(const AtomicReference<Type>* object) {
		if (object != NULL) {
			if (object->_object == NULL) {
				this->setRef(object->newRef());
			} else {
				this->setRef((void*)(object->_object));
			}
			delete object;
		}
	}
	virtual inline AtomicReference<Type>& operator =(const AtomicReference<Type>& object) {
		this->setRef((void*)(object._object));
		return *this;
	}
	virtual inline AtomicRef<Type>* operator->() const {
		return (AtomicRef<Type>*)(this->_object);
	}
	
	inline Type get() { return THIS->get(); }
	inline void set(const Type& newValue) { THIS->set(newValue); }
	inline Type getAndSet(const Type& newValue) { return THIS->getAndSet(newValue); }
	inline bool compareAndSet(const Type& compareValue, const Type& newValue) { return THIS->compareAndSet(compareValue, newValue); }
};

class AtomicObject : public Object {
public:
	virtual inline AtomicRef<Object>* newRef() const { AtomicRef<Object>* o = memNew(o, AtomicRef<Object>()); return o; }
	inline AtomicObject() { }
	inline AtomicObject(const void* object) { this->setRef(newRef()); }
	inline AtomicObject(const Object& initialValue) { this->setRef(newRef()); THIS->set(initialValue); }
	inline AtomicObject(const AtomicObject* object) {
		if (object != NULL) {
			if (object->_object == NULL) {
				this->setRef(object->newRef());
			} else {
				this->setRef((void*)(object->_object));
			}
			delete object;
		}
	}
	virtual inline AtomicObject& operator =(const AtomicObject& object) {
		this->setRef((void*)(object._object));
		return *this;
	}
	virtual inline AtomicRef<Object>* operator->() const {
		return (AtomicRef<Object>*)(this->_object);
	}
	
	inline Object get() { return THIS->get(); }
	inline void set(const Object& newValue) { THIS->set(newValue); }
	inline Object getAndSet(const Object& newValue) { return THIS->getAndSet(newValue); }
	inline bool compareAndSet(const Object& compareValue, const Object& newValue) { return THIS->compareAndSet(compareValue, newValue); }
};

typedef AtomicReference<bool>       AtomicBoolean;
typedef AtomicReference<int32_t>    AtomicInteger;
typedef AtomicReference<int64_t>    AtomicLong;

#endif //JAPPSY_UATOMICOBJECT_H