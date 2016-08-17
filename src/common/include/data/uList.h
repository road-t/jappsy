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

#ifndef JAPPSY_ULIST_H
#define JAPPSY_ULIST_H

#include <data/uCollection.h>

template <typename Type>
class List;

template <typename Type>
class RefList : public RefCollection<Type> {
public:
	inline RefList() : RefCollection<Type>() { this->TYPE = TypeList; }
	
	inline RefList(int initialCapacity) throw(const char*) : RefCollection<Type>(initialCapacity) {
		this->TYPE = TypeList;
	}
	
	virtual inline bool add(int32_t index, const Type& object) throw(const char*) {
		if ((index >= 0) && (index <= this->m_count)) {
			if ((this->m_count > 0) && (this->m_initialCapacity > 0) && (this->m_count >= this->m_initialCapacity)) {
				if (index > 0) {
					delete this->m_stack[0];
					if (index > 1)
						memmove(this->m_stack, this->m_stack + 1, (index - 1) * sizeof(Type*));
					
					index--;
					try {
						Type* item = new Type(object);
						if (item == NULL) throw eOutOfMemory;
						this->m_stack[index] = item;
					} catch (...) {
						this->m_stack[index] = NULL;
						throw;
					}
				} else {
					delete this->m_stack[this->m_count - 1];
					if (this->m_count > 1)
						memmove(this->m_stack + 1, this->m_stack, (this->m_count - 1) * sizeof(Type*));
					
					try {
						Type* item = new Type(object);
						if (item == NULL) throw eOutOfMemory;
						this->m_stack[0] = item;
					} catch (...) {
						this->m_stack[0] = NULL;
						throw;
					}
				}
			} else {
				this->resize(this->m_count + 1);

				if (index < this->m_count)
					memmove(this->m_stack + index + 1, this->m_stack + index, (this->m_count - index) * sizeof(Type*));

					this->m_count++;
				try {
					Type* item = new Type(object);
					if (item == NULL) throw eOutOfMemory;
					this->m_stack[index] = item;
				} catch (...) {
					this->m_stack[index] = NULL;
					throw;
				}
			}
		}
		return true;
	}
	
	virtual inline bool addAll(int32_t index, Collection<Type>& collection) throw(const char*) {
		if ((index >= 0) && (index <= this->m_count)) {
			int size = collection->size();
			if (size > 0) {
				if (this->m_initialCapacity > 0) {
					ListIterator<Type> it = collection->listIterator(size - 1);
					while (it->hasPrevious()) {
						add(index, it->previous());
					}
				} else {
					this->resize(this->m_count + size);
					
					if (index < this->m_count)
						memmove(this->m_stack + index + size, this->m_stack + index, (this->m_count - index) * sizeof(Type*));
					
					try {
						Iterator<Type> it = collection->iterator();
						while (it->hasNext()) {
							Type* item = new Type(it->next());
							if (item == NULL) throw eOutOfMemory;
							this->m_stack[index] = item;
							index++; size--;
							this->m_count++;
						}
					} catch (...) {
						while (size > 0) {
							this->m_stack[index] = NULL;
							index++; size--;
							this->m_count++;
						}
						throw;
					}
				}
			}
		} else
			throw eOutOfRange;
		
		return true;
	}
	
	virtual inline int32_t lastIndexOf(const Type& value) const {
		int32_t foundIndex = -1;
		if (this->m_count > 0) {
			for (int32_t i = this->m_count-1; i >= 0; i--) {
				if (this->m_stack[i] != NULL) {
					if (*(this->m_stack[i]) == value) foundIndex = i;
				} else {
					if (Object::isNull(value)) foundIndex = i;
				}
			}
		}
		return foundIndex;
	}
	
	virtual inline const ListIterator<Type> listIterator(int32_t index = 0) const {
		RefIterator<Type>::reset(index);
		return ListIterator<Type>(*this);
	}
	
	virtual inline Type& set(int32_t index, const Type& value) throw(const char*) {
		if ((index >= 0) && (index < this->m_count)) {
			Type* item = this->m_stack[index];
			if (item != NULL) {
				delete item;
			}
			
			try {
				item = new Type(value);
				if (item == NULL) throw eOutOfMemory;
				this->m_stack[index] = item;
				return *item;
			} catch (...) {
				this->m_stack[index] = NULL;
				throw;
			}
		} else
			throw eOutOfRange;
	}
	
	virtual inline List<Type> subList(int32_t start, int32_t end) throw(const char*) {
		if (start >= this->m_count)
			return List<Type>(null);
		
		List<Type> list;
		if (start < 0) start = this->m_count + start;
		if (start < 0) start = 0;
		if (end < 0) end = this->m_count + end; else if (end > this->m_count) end = this->m_count;
		if (end < 0) end = 0;
		if (start != end) {
			for (int32_t i = start; i < end; i++) {
				list->push(this->get(i));
			}
		}
		return list;
	}
	
	// defined in Stack<Type>
	// inline void clear();
	// inline bool contains(Type* value) const;
	// inline Type* get(int32_t index) const;
	// inline int32_t indexOf(const Type* value) const;
	// inline bool isEmpty() const;
	// inline Type* remove(int32_t index);
	// inline uint32_t size() const;
};

template <typename Type>
class SynchronizedList;

template <typename Type>
class List : public Collection<Type> {
public:
	RefClass(List, List<Type>)
	
	inline List(uint32_t initialCapacity) {
		RefList<Type>* o = new RefList<Type>(initialCapacity);
		if (o == NULL) throw eOutOfMemory;
		this->setRef(o);
	}
	
	virtual inline bool add(const Type& object) throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::add(object); }
	virtual inline bool add(int32_t index, const Type& object) throw(const char*) { CHECKTHIS; return THIS->add(index, object); }
	virtual inline bool addAll(Collection<Type> collection) throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::addAll(collection); }
	virtual inline bool addAll(int32_t index, Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->addAll(index, collection); }
	virtual inline void clear() throw(const char*) { CHECKTHIS; THIS->RefStack<Type>::clear(); }
	virtual inline bool contains(const Type& value) const throw(const char*) { CHECKTHIS; return THIS->RefStack<Type>::contains(value); }
	virtual inline bool containsAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::containsAll(collection); }
	virtual inline const Type& get(int32_t index) const throw(const char*) { CHECKTHIS; return THIS->RefStack<Type>::get(index); }
	virtual inline int32_t indexOf(const Type& value) const throw(const char*) { CHECKTHIS; return THIS->RefStack<Type>::indexOf(value); }
	virtual inline bool isEmpty() const throw(const char*) { CHECKTHIS; return THIS->RefStack<Type>::isEmpty(); }
	virtual inline const Iterator<Type> iterator() const throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::iterator(); }
	virtual inline int32_t lastIndexOf(const Type& value) const throw(const char*) { CHECKTHIS; return THIS->lastIndexOf(value); }
	virtual inline const ListIterator<Type> listIterator(int32_t index = 0) const throw(const char*) { CHECKTHIS; return THIS->listIterator(index); }
	virtual inline const Type remove(int32_t index) throw(const char*) { CHECKTHIS; return THIS->RefStack<Type>::remove(index); }
	virtual inline bool remove(const Type& value) throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::remove(value); }
	virtual inline bool removeAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::removeAll(collection); }
	virtual inline bool retainAll(Collection<Type>& collection) throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::retainAll(collection); }
	virtual inline Type& set(int32_t index, const Type& value) throw(const char*) { CHECKTHIS; return THIS->set(index, value); }
	virtual inline int32_t size() const throw(const char*) { CHECKTHIS; return THIS->RefStack<Type>::size(); }
	virtual inline List<Type> subList(int32_t start, int32_t end) throw(const char*) { CHECKTHIS; return THIS->subList(start, end); }
	virtual inline const Type** toArray() const throw(const char*) { CHECKTHIS; return THIS->RefCollection<Type>::toArray(); }
	
	static SynchronizedList<Type> synchronizedList(List<Type>* newList) {
		return SynchronizedList<Type>(newList);
	}
	
#ifdef DEBUG
	inline static void _test() {
		List<Object> list = new List<Object>();
		Collection<Object> col = new Collection<Object>();
		list.add(null);
		list.add(0, null);
		list.addAll(col);
		list.addAll(0, col);
		list.clear();
		list.contains(null);
		list.containsAll(col);
		list.push(null);
		list.get(0);
		list.indexOf(null);
		list.isEmpty();
		list.iterator();
		list.lastIndexOf(null);
		list.listIterator();
		list.remove(0);
		list.remove(null);
		list.removeAll(col);
		list.retainAll(col);
		list.push(null);
		list.set(0, null);
		list.size();
		list.subList(0, 0);
		list.toArray();
	}
#endif
};

template <typename Type>
class ArrayList : public List<Type> {
public:
	RefClass(ArrayList, List<Type>)
};

template <typename Type>
class SynchronizedList : public List<Type> {
public:
	RefClass(SynchronizedList, List<Type>)
	
	virtual inline bool add(const Type& object) throw(const char*) {
		synchronized(this) {
			try {
				THIS->RefCollection<Type>::add(object);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return true;
	}
	
	virtual inline bool add(int32_t index, const Type& object) throw(const char*) {
		synchronized(this) {
			try {
				THIS->add(index, object);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return true;
	}
	
	virtual inline bool addAll(Collection<Type> collection) throw(const char*) {
		synchronized(this) {
			try {
				THIS->RefCollection<Type>::addAll(collection);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return true;
	}
	
	virtual inline bool addAll(int32_t index, Collection<Type>& collection) throw(const char*) {
		synchronized(this) {
			try {
				THIS->addAll(index, collection);
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
				result = THIS->RefCollection<Type>::containsAll(collection);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline const Type& get(int32_t index) const throw(const char*) {
		const Type* result;
		synchronized(this) {
			try {
				result = &(THIS->RefStack<Type>::get(index));
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return *result;
	}
	
	virtual inline int32_t indexOf(const Type& value) const throw(const char*) {
		int32_t result;
		synchronized(this) {
			result = THIS->RefStack<Type>::indexOf(value);
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
				result = THIS->RefCollection<Type>::iterator();
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline int32_t lastIndexOf(const Type& value) const throw(const char*) {
		int32_t result;
		synchronized(this) {
			result = THIS->lastIndexOf(value);
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline const ListIterator<Type> listIterator(int32_t index = 0) const throw(const char*) {
		ListIterator<Type> result;
		synchronized(this) {
			try {
				result = THIS->listIterator(index);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline const Type remove(int32_t index) throw(const char*) {
		Type result;
		synchronized(this) {
			try {
				result = THIS->RefStack<Type>::remove(index);
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
				result = THIS->RefCollection<Type>::remove(value);
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
				result = THIS->RefCollection<Type>::removeAll(collection);
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
				result = THIS->RefCollection<Type>::retainAll(collection);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline Type& set(int32_t index, const Type& value) throw(const char*) {
		Type* result;
		synchronized(this) {
			try {
				result = &(THIS->set(index, value));
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return *result;
	}
	
	virtual inline int32_t size() const throw(const char*) {
		uint32_t result;
		synchronized(this) {
			result = THIS->RefStack<Type>::size();
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline List<Type> subList(int32_t start, int32_t end) throw(const char*) {
		List<Type> result;
		synchronized(this) {
			try {
				result = THIS->subList(start, end);
			} catch (...) {
				this->notifyAll();
				throw;
			}
			this->notifyAll();
		}
		return result;
	}
	
	virtual inline const Type** toArray() const throw(const char*) {
		const Type** result;
		synchronized(this) {
			result = THIS->RefCollection<Type>::toArray();
			this->notifyAll();
		}
		return result;
	}
	
#ifdef DEBUG
	inline static void _test() {
		SynchronizedList<Object> list = List<Object>::synchronizedList(new ArrayList<Object>());
		SynchronizedCollection<Object> col = Collection<Object>::synchronizedCollection(new Collection<Object>());
		list.add(null);
		list.add(0, null);
		list.addAll(col);
		list.addAll(0, col);
		list.clear();
		list.contains(null);
		list.containsAll(col);
		list.push(null);
		list.get(0);
		list.indexOf(null);
		list.isEmpty();
		list.iterator();
		list.lastIndexOf(null);
		list.listIterator();
		list.remove(0);
		list.remove(null);
		list.removeAll(col);
		list.retainAll(col);
		list.push(null);
		list.set(0, null);
		list.size();
		list.subList(0, 0);
		list.toArray();
	}
#endif

};

#endif //JAPPSY_ULIST_H