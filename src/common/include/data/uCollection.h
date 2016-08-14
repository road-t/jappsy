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
class Collection;

template <typename Type>
class RefCollection : public RefListIterator<Type> {
public:
	inline RefCollection() : RefListIterator<Type>() { this->TYPE = TypeCollection; }
	
	inline RefCollection(int initialCapacity) throw(const char*) : RefListIterator<Type>(initialCapacity) {
		this->TYPE = TypeCollection;
	}
	
	virtual inline bool add(const Type& object) throw(const char*) {
		RefStack<Type>::push(object);
		return true;
	}
	
	virtual inline bool addAll(Collection<Type>& collection) throw(const char*) {
		Iterator<Type> it = collection->iterator();
		while (it->hasNext()) {
			RefStack<Type>::push(it->next());
		}
		return true;
	}
	
	virtual inline bool containsAll(Collection<Type>& collection) {
		Iterator<Type> it = collection->iterator();
		while (it->hasNext()) {
			if (!RefStack<Type>::contains(it->next())) {
				return false;
			}
		}
		return true;
	}
	
	virtual inline const Iterator<Type> iterator() const {
		RefIterator<Type>::reset(0);
		return Iterator<Type>(*this);
	}
	
	virtual inline const ListIterator<Type> listIterator(int32_t index = 0) const {
		RefIterator<Type>::reset(index);
		return ListIterator<Type>(*this);
	}
	
	virtual inline bool remove(const Type& value) throw(const char*) {
		int32_t index = RefStack<Type>::search(value);
		if (index >= 0) {
			RefStack<Type>::remove(index);
			return true;
		}
		return false;
	}
	
	virtual inline bool removeAll(Collection<Type>& collection) throw(const char*) {
		bool result = false;
		Iterator<Type> it = collection->iterator();
		while (it->hasNext()) {
			result |= remove(it->next());
		}
		return result;
	}
	
	virtual inline bool retainAll(Collection<Type>& collection) throw(const char*) {
		bool result = false;
		Iterator<Type> it = iterator();
		while (it->hasNext()) {
			Type o = it->next();
			if (!collection->contains(o)) {
				it->remove();
				result = true;
			}
		}
		return result;
	}
	
	virtual inline const Type** toArray() const {
		return (const Type**)(this->m_stack);
	}
	
	// defined in Stack<Type>
	// inline void clear();
	// inline bool contains(Type* value) const;
	// inline bool isEmpty() const;
	// inline uint32_t size() const;
};

template <typename Type>
class SynchronizedCollection;

template <typename Type>
class Collection : public ListIterator<Type> {
public:
	RefClass(Collection, Collection<Type>)
	
	inline Collection(uint32_t initialCapacity) throw(const char*) {
		RefCollection<Type>* o = new RefCollection<Type>(initialCapacity);
		if (o == NULL) throw eOutOfMemory;
		this->setRef(o);
	}
	
	virtual inline bool add(const Type& object) throw(const char*) { CHECKTHIS; return THIS->add(object); }
	virtual inline bool addAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->addAll(collection); }
	virtual inline void clear() throw(const char*) { CHECKTHIS; THIS->RefStack<Type>::clear(); }
	virtual inline bool contains(const Type& value) const throw(const char*) { CHECKTHIS; return THIS->RefStack<Type>::contains(value); }
	virtual inline bool containsAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->containsAll(collection); }
	virtual inline bool isEmpty() const throw(const char*) { CHECKTHIS; return THIS->RefStack<Type>::isEmpty(); }
	virtual inline const Iterator<Type> iterator() const throw(const char*) { CHECKTHIS; return THIS->iterator(); }
	virtual inline bool remove(const Type& value) throw(const char*) { CHECKTHIS; return THIS->remove(value); }
	virtual inline bool removeAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->removeAll(collection); }
	virtual inline bool retainAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->retainAll(collection); }
	virtual inline int32_t size() const throw(const char*) { CHECKTHIS; return THIS->RefStack<Type>::size(); }
	virtual inline const Type** toArray() const throw(const char*) { CHECKTHIS; return THIS->toArray(); }
	
	static SynchronizedCollection<Type> synchronizedCollection(Collection<Type>* newCollection) {
		return SynchronizedCollection<Type>(newCollection);
	}
	
#ifdef DEBUG
	inline static void _test() {
		Collection<Object> col = new Collection<Object>();
		col.add(null);
		Collection<Object> col2 = new Collection<Object>();
		col.addAll(col2);
		col.clear();
		col.contains(null);
		col.containsAll(col2);
		col.isEmpty();
		col.iterator();
		col.remove(null);
		col.removeAll(col2);
		col.retainAll(col2);
		col.size();
		col.toArray();
	}
#endif
};

template <typename Type>
class SynchronizedCollection : public Collection<Type> {
public:
	RefClass(SynchronizedCollection, Collection<Type>)
	
	virtual inline bool add(const Type& object) throw(const char*) {
		synchronized(this) {
			try {
				THIS->add(object);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return true;
	}
	
	virtual inline bool addAll(Collection<Type>& collection) throw(const char*) {
		synchronized(this) {
			try {
				THIS->addAll(collection);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return true;
	}
	
	virtual inline void clear() throw(const char*) {
		synchronized(this) {
			try {
				THIS->RefStack<Type>::clear();
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
	}
	
	virtual inline bool contains(const Type& value) const throw(const char*) {
		bool result;
		synchronized(this) {
			result = THIS->RefStack<Type>::contains(value);
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline bool containsAll(Collection<Type>& collection) throw(const char*) {
		bool result;
		synchronized(this) {
			try {
				result = THIS->containsAll(collection);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline bool isEmpty() const throw(const char*) {
		bool result;
		synchronized(this) {
			result = THIS->RefStack<Type>::isEmpty();
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline const Iterator<Type> iterator() const throw(const char*) {
		Iterator<Type> result;
		synchronized(this) {
			try {
				result = THIS->iterator();
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline bool remove(const Type& value) throw(const char*) {
		bool result;
		synchronized(this) {
			try {
				result = THIS->remove(value);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline bool removeAll(Collection<Type>& collection) throw(const char*) {
		bool result;
		synchronized(this) {
			try {
				result = THIS->removeAll(collection);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline bool retainAll(Collection<Type>& collection) throw(const char*) {
		bool result;
		synchronized(this) {
			try {
				result = THIS->retainAll(collection);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline int32_t size() const throw(const char*) {
		uint32_t result;
		synchronized(this) {
			result = THIS->RefStack<Type>::size();
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline const Type** toArray() const throw(const char*) {
		const Type** result;
		synchronized(this) {
			result = THIS->toArray();
			this->notifyAll();
		}
		return result;
	}

#ifdef DEBUG
	inline static void _test() {
		SynchronizedCollection<Object> col = Collection<Object>::synchronizedCollection(new Collection<Object>());
		col.add(null);
		SynchronizedCollection<Object> col2 = Collection<Object>::synchronizedCollection(new Collection<Object>());
		col.addAll(col2);
		col.clear();
		col.contains(null);
		col.containsAll(col2);
		col.isEmpty();
		col.iterator();
		col.remove(null);
		col.removeAll(col2);
		col.retainAll(col2);
		col.size();
		col.toArray();
	}
#endif
};

#endif //JAPPSY_UCOLLECTION_H
