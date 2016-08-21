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
class RefList : public JRefCollection<Type> {
public:
	inline RefList() : JRefCollection<Type>() { THIS.TYPE = TypeList; }
	
	inline RefList(int initialCapacity) throw(const char*) : JRefCollection<Type>(initialCapacity) {
		THIS.TYPE = TypeList;
	}
	
	virtual inline bool add(int32_t index, const Type& object) throw(const char*) {
		if ((index >= 0) && (index <= THIS.m_count)) {
			if ((THIS.m_count > 0) && (THIS.m_initialCapacity > 0) && (THIS.m_count >= THIS.m_initialCapacity)) {
				if (index > 0) {
					delete THIS.m_stack[0];
					if (index > 1)
						memmove(THIS.m_stack, THIS.m_stack + 1, (index - 1) * sizeof(Type*));
					
					index--;
					try {
						Type* item = new Type(object);
						if (item == NULL) throw eOutOfMemory;
						THIS.m_stack[index] = item;
					} catch (...) {
						THIS.m_stack[index] = NULL;
						throw;
					}
				} else {
					delete THIS.m_stack[THIS.m_count - 1];
					if (THIS.m_count > 1)
						memmove(THIS.m_stack + 1, THIS.m_stack, (THIS.m_count - 1) * sizeof(Type*));
					
					try {
						Type* item = new Type(object);
						if (item == NULL) throw eOutOfMemory;
						THIS.m_stack[0] = item;
					} catch (...) {
						THIS.m_stack[0] = NULL;
						throw;
					}
				}
			} else {
				THIS.resize(THIS.m_count + 1);

				if (index < THIS.m_count)
					memmove(THIS.m_stack + index + 1, THIS.m_stack + index, (THIS.m_count - index) * sizeof(Type*));

					THIS.m_count++;
				try {
					Type* item = new Type(object);
					if (item == NULL) throw eOutOfMemory;
					THIS.m_stack[index] = item;
				} catch (...) {
					THIS.m_stack[index] = NULL;
					throw;
				}
			}
		}
		return true;
	}
	
	virtual inline bool addAll(int32_t index, JCollection<Type>& collection) throw(const char*) {
		if ((index >= 0) && (index <= THIS.m_count)) {
			int size = collection.size();
			if (size > 0) {
				if (THIS.m_initialCapacity > 0) {
					JListIterator<Type> it = collection.ref().listIterator(size - 1);
					while (it.hasPrevious()) {
						add(index, it.previous());
					}
				} else {
					THIS.resize(THIS.m_count + size);
					
					if (index < THIS.m_count)
						memmove(THIS.m_stack + index + size, THIS.m_stack + index, (THIS.m_count - index) * sizeof(Type*));
					
					try {
						JIterator<Type> it = collection.iterator();
						while (it.hasNext()) {
							Type* item = new Type(it.next());
							if (item == NULL) throw eOutOfMemory;
							THIS.m_stack[index] = item;
							index++; size--;
							THIS.m_count++;
						}
					} catch (...) {
						while (size > 0) {
							THIS.m_stack[index] = NULL;
							index++; size--;
							THIS.m_count++;
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
		if (THIS.m_count > 0) {
			for (int32_t i = THIS.m_count-1; i >= 0; i--) {
				if (THIS.m_stack[i] != NULL) {
					if (*(THIS.m_stack[i]) == value) foundIndex = i;
				} else {
					if (JObject::isNull(value)) foundIndex = i;
				}
			}
		}
		return foundIndex;
	}
	
	virtual inline const JListIterator<Type> listIterator(int32_t index = 0) const {
		JRefIterator<Type>::reset(index);
		return new JListIterator<Type>(this);
	}
	
	virtual inline Type& set(int32_t index, const Type& value) throw(const char*) {
		if ((index >= 0) && (index < THIS.m_count)) {
			Type* item = THIS.m_stack[index];
			if (item != NULL) {
				delete item;
			}
			
			try {
				item = new Type(value);
				if (item == NULL) throw eOutOfMemory;
				THIS.m_stack[index] = item;
				return *item;
			} catch (...) {
				THIS.m_stack[index] = NULL;
				throw;
			}
		} else
			throw eOutOfRange;
	}
	
	virtual inline List<Type> subList(int32_t start, int32_t end) throw(const char*) {
		if (start >= THIS.m_count)
			return List<Type>(null);
		
		List<Type> list;
		if (start < 0) start = THIS.m_count + start;
		if (start < 0) start = 0;
		if (end < 0) end = THIS.m_count + end; else if (end > THIS.m_count) end = THIS.m_count;
		if (end < 0) end = 0;
		if (start != end) {
			for (int32_t i = start; i < end; i++) {
				list.push(THIS.get(i));
			}
		}
		return list;
	}
	
	// defined in JStack<Type>
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
class List : public JCollection<Type> {
public:
	JRefTemplate(List, List, RefList)
	
	inline List() {
		THIS.initialize();
	}
	
	inline List(uint32_t initialCapacity) {
		THIS.initialize();
		RefList<Type>* o = new RefList<Type>(initialCapacity);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	virtual inline bool add(const Type& object) throw(const char*) { return THIS.ref().JRefCollection<Type>::add(object); }
	virtual inline bool add(int32_t index, const Type& object) throw(const char*) { return THIS.ref().add(index, object); }
	virtual inline bool addAll(JCollection<Type> collection) throw(const char*) { return THIS.ref().JRefCollection<Type>::addAll(collection); }
	virtual inline bool addAll(int32_t index, JCollection<Type>& collection) throw(const char*) { return THIS.ref().addAll(index, collection); }
	virtual inline void clear() throw(const char*) { THIS.ref().JRefStack<Type>::clear(); }
	virtual inline bool contains(const Type& value) const throw(const char*) { return THIS.ref().JRefStack<Type>::contains(value); }
	virtual inline bool containsAll(JCollection<Type>& collection) throw(const char*) { return THIS.ref().JRefCollection<Type>::containsAll(collection); }
	virtual inline const Type& get(int32_t index) const throw(const char*) { return THIS.ref().JRefStack<Type>::get(index); }
	virtual inline int32_t indexOf(const Type& value) const throw(const char*) { return THIS.ref().JRefStack<Type>::indexOf(value); }
	virtual inline bool isEmpty() const throw(const char*) { return THIS.ref().JRefStack<Type>::isEmpty(); }
	virtual inline const JIterator<Type> iterator() const throw(const char*) { return THIS.ref().JRefCollection<Type>::iterator(); }
	virtual inline int32_t lastIndexOf(const Type& value) const throw(const char*) { return THIS.ref().lastIndexOf(value); }
	virtual inline const JListIterator<Type> listIterator(int32_t index = 0) const throw(const char*) { return THIS.ref().listIterator(index); }
	virtual inline const Type remove(int32_t index) throw(const char*) { return THIS.ref().JRefStack<Type>::remove(index); }
	virtual inline bool remove(const Type& value) throw(const char*) { return THIS.ref().JRefCollection<Type>::remove(value); }
	virtual inline bool removeAll(JCollection<Type>& collection) throw(const char*) { return THIS.ref().JRefCollection<Type>::removeAll(collection); }
	virtual inline bool retainAll(JCollection<Type>& collection) throw(const char*) { return THIS.ref().JRefCollection<Type>::retainAll(collection); }
	virtual inline Type& set(int32_t index, const Type& value) throw(const char*) { return THIS.ref().set(index, value); }
	virtual inline int32_t size() const throw(const char*) { return THIS.ref().JRefStack<Type>::size(); }
	virtual inline List<Type> subList(int32_t start, int32_t end) throw(const char*) { return THIS.ref().subList(start, end); }
	virtual inline const Type** toArray() const throw(const char*) { return THIS.ref().JRefCollection<Type>::toArray(); }
	
	static SynchronizedList<Type> synchronizedList(List<Type>* newList) {
		return SynchronizedList<Type>(newList);
	}
};

template <typename Type>
class ArrayList : public List<Type> {
public:
	JRefTemplate(ArrayList, List, RefList)

	inline ArrayList() {
		THIS.initialize();
	}
};

template <typename Type>
class SynchronizedList : public List<Type> {
public:
	JRefTemplate(SynchronizedList, List, RefList)
	
	virtual inline bool add(const Type& object) throw(const char*) {
		synchronized(*this) {
			try {
				THIS.ref().JRefCollection<Type>::add(object);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return true;
	}
	
	virtual inline bool add(int32_t index, const Type& object) throw(const char*) {
		synchronized(*this) {
			try {
				THIS.ref().add(index, object);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return true;
	}
	
	virtual inline bool addAll(JCollection<Type> collection) throw(const char*) {
		synchronized(*this) {
			try {
				THIS.ref().JRefCollection<Type>::addAll(collection);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return true;
	}
	
	virtual inline bool addAll(int32_t index, JCollection<Type>& collection) throw(const char*) {
		synchronized(*this) {
			try {
				THIS.ref().addAll(index, collection);
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
				result = THIS.ref().JRefCollection<Type>::containsAll(collection);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline const Type& get(int32_t index) const throw(const char*) {
		const Type* result;
		synchronized(*this) {
			try {
				result = &(THIS.ref().JRefStack<Type>::get(index));
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return *result;
	}
	
	virtual inline int32_t indexOf(const Type& value) const throw(const char*) {
		int32_t result;
		synchronized(*this) {
			result = THIS.ref().JRefStack<Type>::indexOf(value);
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
				result = THIS.ref().JRefCollection<Type>::iterator();
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline int32_t lastIndexOf(const Type& value) const throw(const char*) {
		int32_t result;
		synchronized(*this) {
			result = THIS.ref().lastIndexOf(value);
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline const JListIterator<Type> listIterator(int32_t index = 0) const throw(const char*) {
		JListIterator<Type> result;
		synchronized(*this) {
			try {
				result = THIS.ref().listIterator(index);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline const Type remove(int32_t index) throw(const char*) {
		Type result;
		synchronized(*this) {
			try {
				result = THIS.ref().JRefStack<Type>::remove(index);
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
				result = THIS.ref().JRefCollection<Type>::remove(value);
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
				result = THIS.ref().JRefCollection<Type>::removeAll(collection);
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
				result = THIS.ref().JRefCollection<Type>::retainAll(collection);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline Type& set(int32_t index, const Type& value) throw(const char*) {
		Type* result;
		synchronized(*this) {
			try {
				result = &(THIS.ref().set(index, value));
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return *result;
	}
	
	virtual inline int32_t size() const throw(const char*) {
		uint32_t result;
		synchronized(*this) {
			result = THIS.ref().JRefStack<Type>::size();
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline List<Type> subList(int32_t start, int32_t end) throw(const char*) {
		List<Type> result;
		synchronized(*this) {
			try {
				result = THIS.ref().subList(start, end);
			} catch (...) {
				THIS.notifyAll();
				throw;
			}
			THIS.notifyAll();
		}
		return result;
	}
	
	virtual inline const Type** toArray() const throw(const char*) {
		const Type** result;
		synchronized(*this) {
			result = THIS.ref().JRefCollection<Type>::toArray();
			THIS.notifyAll();
		}
		return result;
	}
};

#endif //JAPPSY_ULIST_H