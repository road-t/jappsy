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
	inline RefSet() : RefList<Type>() { THIS.TYPE = TypeSet; }
	inline RefSet(int initialCapacity) throw(const char*) : RefList<Type>(initialCapacity) { THIS.TYPE = TypeSet; }
	
	virtual inline bool add(const Type& object) throw(const char*) {
		if (!JRefStack<Type>::contains(object)) {
			JRefStack<Type>::push(object);
			return true;
		}
		return false;
	}
	
	virtual inline bool addAll(Collection<Type>& collection) throw(const char*) {
		bool result = false;
		JIterator<Type> it = collection.iterator();
		while (it.hasNext()) {
			result |= add(it.next());
		}
		return result;
	}

	virtual inline JString toJSON() const {
		JString json = L"[";
		JIterator<Type> it = THIS.iterator();
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
	JRefTemplate(Set, Set, RefSet)
	
	inline Set() {
		THIS.initialize();
	}
	
	inline Set(uint32_t initialCapacity) throw(const char*) {
		THIS.initialize();
		RefSet<Type>* o = new RefSet<Type>(initialCapacity);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	virtual inline bool add(const Type& object) throw(const char*) { return THIS.ref().add(object); }
	virtual inline bool addAll(Collection<Type>& collection) throw(const char*) { return THIS.ref().addAll(collection); }
	virtual inline void clear() throw(const char*) { THIS.ref().JRefStack<Type>::clear(); }
	virtual inline bool contains(const Type& value) const throw(const char*) { return THIS.ref().JRefStack<Type>::contains(value); }
	virtual inline bool containsAll(Collection<Type>& collection) throw(const char*) { return THIS.ref().RefCollection<Type>::containsAll(collection); }
	virtual inline bool isEmpty() const throw(const char*) { return THIS.ref().JRefStack<Type>::isEmpty(); }
	virtual inline const JIterator<Type> iterator() const throw(const char*) { return THIS.ref().RefCollection<Type>::iterator(); }
	virtual inline bool remove(const Type& value) throw(const char*) { return THIS.ref().RefCollection<Type>::remove(value); }
	virtual inline bool removeAll(Collection<Type>& collection) throw(const char*) { return THIS.ref().RefCollection<Type>::removeAll(collection); }
	virtual inline bool retainAll(Collection<Type>& collection) throw(const char*) { return THIS.ref().RefCollection<Type>::retainAll(collection); }
	virtual inline int32_t size() const throw(const char*) { return THIS.ref().JRefStack<Type>::size(); }
	virtual inline const Type** toArray() const throw(const char*) { return THIS.ref().RefCollection<Type>::toArray(); }
	
	static SynchronizedSet<Type> synchronizedSet(Set<Type>* newSet) {
		return SynchronizedSet<Type>(newSet);
	}
};

template <typename Type>
class SynchronizedSet : public Set<Type> {
public:
	JRefTemplate(SynchronizedSet, Set, RefSet)
	
	virtual inline bool add(const Type& object) throw(const char*) {
		bool result;
		synchronized(*this) {
			try {
				result = THIS.ref().add(object);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline bool addAll(Collection<Type>& collection) throw(const char*) {
		bool result;
		synchronized(*this) {
			try {
				result = THIS.ref().addAll(collection);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline void clear() throw(const char*) {
		synchronized(*this) {
			try {
				THIS.ref().clear();
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
			result = THIS.ref().contains(value);
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline bool containsAll(Collection<Type>& collection) throw(const char*) {
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
			result = THIS.ref().isEmpty();
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
	
	virtual inline bool removeAll(Collection<Type>& collection) throw(const char*) {
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
	
	virtual inline bool retainAll(Collection<Type>& collection) throw(const char*) {
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
			result = THIS.ref().size();
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

#endif //JAPPSY_USET_H

