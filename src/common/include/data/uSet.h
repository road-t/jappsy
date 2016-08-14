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

#ifndef JAPPSY_USET_H
#define JAPPSY_USET_H

#include <data/uList.h>
#include <data/uJSON.h>

template <typename Type>
class RefSet : public RefList<Type> {
public:
	inline RefSet() : RefList<Type>() { this->TYPE = TypeSet; }
	inline RefSet(int initialCapacity) throw(const char*) : RefList<Type>(initialCapacity) { this->TYPE = TypeSet; }
	
	virtual inline bool add(const Type& object) throw(const char*) {
		if (!RefStack<Type>::contains(object)) {
			RefStack<Type>::push(object);
			return true;
		}
		return false;
	}
	
	virtual inline bool addAll(Collection<Type>& collection) throw(const char*) {
		bool result = false;
		Iterator<Type> it = collection->iterator();
		while (it->hasNext()) {
			result |= add(it->next());
		}
		return result;
	}

	virtual inline String toJSON() const {
		String json = L"[";
		Iterator<Type> it = this->iterator();
		bool first = true;
		while (it.hasNext()) {
			if (first) first = false; else json += L",";
			json += JSON::stringify(it.next());
		}
		json += L"]";
		return json;
	}
};

template <typename Type>
class SynchronizedSet;

template <typename Type>
class Set : public List<Type> {
public:
	RefClass(Set, Set<Type>)
	
	inline Set(uint32_t initialCapacity) throw(const char*) {
		RefSet<Type>* o = new RefSet<Type>(initialCapacity);
		if (o == NULL) throw eOutOfMemory;
		this->setRef(o);
	}
	
	virtual inline bool add(const Type& object) throw(const char*) { CHECKTHIS; return THIS->add(object); }
	virtual inline bool addAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->addAll(collection); }
	virtual inline void clear() throw(const char*) { CHECKTHIS; THIS->RefStack<Type>::clear(); }
	virtual inline bool contains(const Type& value) const throw(const char*) { CHECKTHIS; return THIS->RefStack<Type>::contains(value); }
	virtual inline bool containsAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::containsAll(collection); }
	virtual inline bool isEmpty() const throw(const char*) { CHECKTHIS; return THIS->RefStack<Type>::isEmpty(); }
	virtual inline const Iterator<Type> iterator() const throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::iterator(); }
	virtual inline bool remove(const Type& value) throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::remove(value); }
	virtual inline bool removeAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::removeAll(collection); }
	virtual inline bool retainAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::retainAll(collection); }
	virtual inline int32_t size() const throw(const char*) { CHECKTHIS; return THIS->RefStack<Type>::size(); }
	virtual inline const Type** toArray() const throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::toArray(); }
	
	static SynchronizedSet<Type> synchronizedSet(Set<Type>* newSet) {
		return SynchronizedSet<Type>(newSet);
	}
	
#ifdef DEBUG
	inline static void _test() {
		Set<Object> set = new Set<Object>();
		Collection<Object> col = new Collection<Object>();
		set.add(null);
		set.addAll(col);
		set.clear();
		set.contains(null);
		set.containsAll(col);
		set.isEmpty();
		set.iterator();
		set.remove(null);
		set.removeAll(col);
		set.retainAll(col);
		set.size();
		set.toArray();
	}
#endif
};

template <typename Type>
class SynchronizedSet : public Set<Type> {
public:
	RefClass(SynchronizedSet, Set<Type>)
	
	virtual inline bool add(const Type& object) throw(const char*) {
		bool result;
		synchronized(this) {
			try {
				result = THIS->add(object);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline bool addAll(Collection<Type>& collection) throw(const char*) {
		bool result;
		synchronized(this) {
			try {
				result = THIS->addAll(collection);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline void clear() throw(const char*) {
		synchronized(this) {
			try {
				THIS->clear();
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
			result = THIS->contains(value);
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
			result = THIS->isEmpty();
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
			result = THIS->size();
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
		SynchronizedSet<Object> set = Set<Object>::synchronizedSet(new Set<Object>());
		SynchronizedCollection<Object> col = Collection<Object>::synchronizedCollection(new Collection<Object>());
		set.add(null);
		set.addAll(col);
		set.clear();
		set.contains(null);
		set.containsAll(col);
		set.isEmpty();
		set.iterator();
		set.remove(null);
		set.removeAll(col);
		set.retainAll(col);
		set.size();
		set.toArray();
	}
#endif
};

#endif //JAPPSY_USET_H

