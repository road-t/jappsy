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

#ifndef JAPPSY_UCOLLECTION_H
#define JAPPSY_UCOLLECTION_H

#include <data/uListIterator.h>

template <typename Type>
class JCollection;

template <typename Type>
class JRefCollection : public JRefListIterator<Type> {
public:
	inline JRefCollection() : JRefListIterator<Type>() { THIS.TYPE = TypeCollection; }
	
	inline JRefCollection(int initialCapacity) throw(const char*) : JRefListIterator<Type>(initialCapacity) {
		THIS.TYPE = TypeCollection;
	}
	
	virtual inline bool add(const Type& object) throw(const char*) {
		JRefStack<Type>::push(object);
		return true;
	}
	
	virtual inline bool addAll(JCollection<Type>& collection) throw(const char*) {
		JIterator<Type> it = collection.iterator();
		while (it.hasNext()) {
			JRefStack<Type>::push(it.next());
		}
		return true;
	}
	
	virtual inline bool containsAll(JCollection<Type>& collection) {
		JIterator<Type> it = collection.iterator();
		while (it.hasNext()) {
			if (!JRefStack<Type>::contains(it.next())) {
				return false;
			}
		}
		return true;
	}
	
	virtual inline const JIterator<Type> iterator() const {
		JRefIterator<Type>::reset(0);
		return new JIterator<Type>(this);
	}
	
	virtual inline const JListIterator<Type> listIterator(int32_t index = 0) const {
		JRefIterator<Type>::reset(index);
		return new JListIterator<Type>(this);
	}
	
	virtual inline bool remove(const Type& value) throw(const char*) {
		int32_t index = JRefStack<Type>::search(value);
		if (index >= 0) {
			JRefStack<Type>::remove(index);
			return true;
		}
		return false;
	}
	
	virtual inline bool removeAll(JCollection<Type>& collection) throw(const char*) {
		bool result = false;
		JIterator<Type> it = collection.iterator();
		while (it.hasNext()) {
			result |= remove(it.next());
		}
		return result;
	}
	
	virtual inline bool retainAll(JCollection<Type>& collection) throw(const char*) {
		bool result = false;
		JIterator<Type> it = iterator();
		while (it.hasNext()) {
			Type o = it.next();
			if (!collection.contains(o)) {
				it.remove();
				result = true;
			}
		}
		return result;
	}
	
	virtual inline const Type** toArray() const {
		return (const Type**)(THIS.m_stack);
	}
	
	// defined in JStack<Type>
	// inline void clear();
	// inline bool contains(Type* value) const;
	// inline bool isEmpty() const;
	// inline uint32_t size() const;
};

template <typename Type>
class JSynchronizedCollection;

template <typename Type>
class JCollection : public JListIterator<Type> {
public:
	JRefTemplate(JCollection, JCollection, JRefCollection)
	
	inline JCollection() {
		THIS.initialize();
	}
	
	inline JCollection(uint32_t initialCapacity) throw(const char*) {
		THIS.initialize();
		JRefCollection<Type>* o = new JRefCollection<Type>(initialCapacity);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	virtual inline bool add(const Type& object) throw(const char*) { return THIS.ref().add(object); }
	virtual inline bool addAll(JCollection<Type>& collection) throw(const char*) { return THIS.ref().addAll(collection); }
	virtual inline void clear() throw(const char*) { THIS.ref().JRefStack<Type>::clear(); }
	virtual inline bool contains(const Type& value) const throw(const char*) { return THIS.ref().JRefStack<Type>::contains(value); }
	virtual inline bool containsAll(JCollection<Type>& collection) throw(const char*) { return THIS.ref().containsAll(collection); }
	virtual inline bool isEmpty() const throw(const char*) { return THIS.ref().JRefStack<Type>::isEmpty(); }
	virtual inline const JIterator<Type> iterator() const throw(const char*) { return THIS.ref().iterator(); }
	virtual inline bool remove(const Type& value) throw(const char*) { return THIS.ref().remove(value); }
	virtual inline bool removeAll(JCollection<Type>& collection) throw(const char*) { return THIS.ref().removeAll(collection); }
	virtual inline bool retainAll(JCollection<Type>& collection) throw(const char*) { return THIS.ref().retainAll(collection); }
	virtual inline int32_t size() const throw(const char*) { return THIS.ref().JRefStack<Type>::size(); }
	virtual inline const Type** toArray() const throw(const char*) { return THIS.ref().toArray(); }
	
	static JSynchronizedCollection<Type> synchronizedCollection(JCollection<Type>* newCollection) {
		return JSynchronizedCollection<Type>(newCollection);
	}
};

template <typename Type>
class JSynchronizedCollection : public JCollection<Type> {
public:
	JRefTemplate(JSynchronizedCollection, JCollection, JRefCollection)
	
	virtual inline bool add(const Type& object) throw(const char*) {
		synchronized(*this) {
			try {
				THIS.ref().add(object);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return true;
	}
	
	virtual inline bool addAll(JCollection<Type>& collection) throw(const char*) {
		synchronized(*this) {
			try {
				THIS.ref().addAll(collection);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return true;
	}
	
	virtual inline void clear() throw(const char*) {
		synchronized(*this) {
			try {
				THIS.ref().JRefStack<Type>::clear();
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
	}
	
	virtual inline bool contains(const Type& value) const throw(const char*) {
		bool result;
		synchronized(*this) {
			result = THIS.ref().JRefStack<Type>::contains(value);
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline bool containsAll(JCollection<Type>& collection) throw(const char*) {
		bool result;
		synchronized(*this) {
			try {
				result = THIS.ref().containsAll(collection);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline bool isEmpty() const throw(const char*) {
		bool result;
		synchronized(*this) {
			result = THIS.ref().JRefStack<Type>::isEmpty();
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline const JIterator<Type> iterator() const throw(const char*) {
		JIterator<Type> result;
		synchronized(*this) {
			try {
				result = THIS.ref().iterator();
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline bool remove(const Type& value) throw(const char*) {
		bool result;
		synchronized(*this) {
			try {
				result = THIS.ref().remove(value);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline bool removeAll(JCollection<Type>& collection) throw(const char*) {
		bool result;
		synchronized(*this) {
			try {
				result = THIS.ref().removeAll(collection);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline bool retainAll(JCollection<Type>& collection) throw(const char*) {
		bool result;
		synchronized(*this) {
			try {
				result = THIS.ref().retainAll(collection);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline int32_t size() const throw(const char*) {
		uint32_t result;
		synchronized(*this) {
			result = THIS.ref().JRefStack<Type>::size();
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline const Type** toArray() const throw(const char*) {
		const Type** result;
		synchronized(*this) {
			result = THIS.ref().toArray();
			THIS.notifyAll();
		}
		return result;
	}
};

template <typename Type>
class CCollection : public CListIterator<Type> {
public:
	inline CCollection(void (*onrelease)(Type& value) = NULL) : CListIterator<Type>(onrelease) { }
	inline CCollection(int initialCapacity, void (*onrelease)(Type& value) = NULL) throw(const char*) : CListIterator<Type>(initialCapacity, onrelease) { }
	
	virtual inline bool add(const Type& object) throw(const char*) {
		CStack<Type>::push(object);
		return true;
	}
	
	virtual inline bool addAll(CCollection<Type>& collection) throw(const char*) {
		int32_t count = collection.count();
		const Type* items = collection.items();
		for (int i = 0; i < count; i++) {
			CStack<Type>::push(items[i]);
		}
		return true;
	}
	
	virtual inline bool containsAll(CCollection<Type>& collection) {
		int32_t count = collection.count();
		const Type* items = collection.items();
		for (int i = 0; i < count; i++) {
			if (!CStack<Type>::contains(items[i])) {
				return false;
			}
		}
		return true;
	}
	
	virtual inline const CIterator<Type>& iterator() const {
		CIterator<Type>::reset(0);
		return CIterator<Type>(*this);
	}
	
	virtual inline const CListIterator<Type>& listIterator(int32_t index = 0) const {
		CIterator<Type>::reset(index);
		return CListIterator<Type>(*this);
	}
	
	virtual inline bool remove(const Type& value) throw(const char*) {
		int32_t index = CStack<Type>::search(value);
		if (index >= 0) {
			CStack<Type>::remove(index);
			return true;
		}
		return false;
	}
	
	virtual inline bool removeAll(CCollection<Type>& collection) throw(const char*) {
		bool result = false;
		int32_t count = collection.count();
		const Type* items = collection.items();
		for (int i = 0; i < count; i++) {
			result |= remove(items[i]);
		}
		return result;
	}
	
	virtual inline bool retainAll(CCollection<Type>& collection) throw(const char*) {
		bool result = false;
		int32_t count = THIS.count();
		Type* items = (Type*)THIS.items();
		for (int i = count - 1; i >= 0; i--) {
			if (!collection.contains(items[i])) {
				CStack<Type>::remove(i);
				result = true;
			}
		}
		return result;
	}
};

#endif //JAPPSY_UCOLLECTION_H
