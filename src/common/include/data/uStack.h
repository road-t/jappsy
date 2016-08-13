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

#ifndef JAPPSY_USTACK_H
#define JAPPSY_USTACK_H

#include <data/uObject.h>

#define STACK_BLOCK_SIZE	16

template <typename Type>
class RefStack : public RefObject {
public:
	Type** m_stack = NULL;
	uint32_t m_count = 0;
	uint32_t m_size = 0;
	uint32_t m_initialCapacity = 0;
	uint32_t m_initialSize = 0;
	
	inline void resize(uint32_t count) throw(const char*) {
		if (count < m_count) {
			for (int i = count; i < m_count; i++) {
				if (m_stack[i] != NULL) {
					memDelete(m_stack[i]);
					m_stack[i] = NULL;
				}
			}
		}
		
		uint32_t size = count - (count % STACK_BLOCK_SIZE) + STACK_BLOCK_SIZE;
		if ((size > m_size) && (size >= m_initialSize)) {
			Type** newStack = memRealloc(Type*, newStack, m_stack, size * sizeof(Type*));
			if (newStack) {
				m_stack = newStack;
				m_size = size;
			} else
				throw eOutOfMemory;
		}
	}
	
	inline void autorelease(uint32_t count) throw(const char*) {
		if (count < m_count) {
			for (int i = count; i < m_count; i++) {
				if (m_stack[i] != NULL) {
					memDelete(m_stack[i]);
					m_stack[i] = NULL;
				}
			}
		}
		
		uint32_t size = count - (count % STACK_BLOCK_SIZE) + STACK_BLOCK_SIZE;
		if (size < m_initialSize) size = m_initialSize;
		if (size < m_size) {
			Type** newStack = memRealloc(Type*, newStack, m_stack, size * sizeof(Type*));
			if (newStack) {
				m_stack = newStack;
				m_size = size;
			} else
				throw eOutOfMemory;
		}
	}
	
public:
	static const Type nullValue;
	
	inline RefStack() {	TYPE = TypeStack; }
	
	inline RefStack(uint32_t initialCapacity) throw(const char*) {
		TYPE = TypeStack;
		resize(initialCapacity);
		m_initialCapacity = initialCapacity;
		m_initialSize = m_size;
	}
	
	inline ~RefStack() {
		if (m_stack != NULL) {
			for (int i = 0; i < m_count; i++) {
				if (m_stack[i] != NULL) {
					memDelete(m_stack[i]);
				}
			}
			memFree(m_stack);
			m_stack = NULL;
		}
	}
	
	virtual inline Type& push(const Type& object) throw(const char*) {
		if ((m_count > 0) && (m_initialCapacity > 0) && (m_count >= m_initialCapacity)) {
			memDelete(m_stack[0]);
			if (m_count > 1) {
				memmove(m_stack, m_stack + 1, (m_count - 1) * sizeof(Type*));
			}
			try {
				Type* newObject = memNew(newObject, Type(object));
				if (newObject == NULL) throw eOutOfMemory;
				m_stack[m_count - 1] = newObject;
				return *newObject;
			} catch (...) {
				m_stack[m_count - 1] = NULL;
				throw;
			}
		} else {
			resize(m_count + 1);
			try {
				Type* newObject = memNew(newObject, Type(object));
				if (newObject == NULL) throw eOutOfMemory;
				m_stack[m_count++] = newObject;
				return *newObject;
			} catch (...) {
				m_stack[m_count++] = NULL;
				throw;
			}
		}
	}
	
	virtual inline Type& unshift(const Type& object) throw(const char*) {
		if ((m_count > 0) && (m_initialCapacity > 0) && (m_count >= m_initialCapacity)) {
			memDelete(m_stack[m_count - 1]);
			if (m_count > 1) {
				memmove(m_stack + 1, m_stack, (m_count - 1) * sizeof(Type*));
			}
			try {
				Type* newObject = memNew(newObject, Type(object));
				if (newObject == NULL) throw eOutOfMemory;
				m_stack[0] = newObject;
				return *newObject;
			} catch (...) {
				m_stack[0] = NULL;
				throw;
			}
		} else {
			resize(m_count + 1);
			memmove(m_stack + 1, m_stack, m_count * sizeof(Type*));
			m_count++;
			try {
				Type* newObject = memNew(newObject, Type(object));
				if (newObject == NULL) throw eOutOfMemory;
				m_stack[0] = newObject;
				return *newObject;
			} catch (...) {
				m_stack[0] = NULL;
				throw;
			}
		}
	}
	
	virtual inline Type pop() throw(const char*) {
		if (m_count > 0) {
			m_count--;
			Type* object = m_stack[m_count];
			m_stack[m_count] = NULL;
			Type result = Type(*object);
			memDelete(object);
			return result;
		}
		
		throw eEmpty;
	}
	
	virtual inline Type shift() throw(const char*) {
		if (m_count > 0) {
			m_count--;
			Type* object = m_stack[0];
			if (m_count > 0) {
				memmove(m_stack, m_stack + 1, m_count * sizeof(Type*));
			}
			m_stack[m_count] = NULL;
			Type result = Type(*object);
			memDelete(object);
			return result;
		}
		
		throw eEmpty;
	}
	
	virtual inline bool contains(const Type& value) const {
		if (m_count > 0) {
			for (int i = m_count-1; i >= 0; i--) {
				if (m_stack[i] != NULL) {
					if (*(m_stack[i]) == value) return true;
				} else if (value == nullValue) {
					return true;
				}
			}
		}
		return false;
	}
	
	virtual inline int32_t count() const {
		return m_count;
	}
	
	// synonym to count()
	virtual inline int32_t size() const {
		return m_count;
	}
	
	virtual inline bool empty() const {
		return (m_count == 0);
	}
	
	// synonym to empty()
	virtual inline bool isEmpty() const {
		return (m_count == 0);
	}
	
	virtual inline void clear() throw(const char*) {
		autorelease(0);
		m_count = 0;
	}
	
	virtual inline const Type& peek() const throw(const char*) {
		if (m_count > 0) {
			if (m_stack[m_count-1] != NULL)
				return *(m_stack[m_count-1]);
		}
		throw eEmpty;
	}
	
	virtual inline const Type& peek(int32_t index) const throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			if (m_stack[index] != NULL)
				return *(m_stack[index]);
			throw eEmpty;
		}
		throw eOutOfRange;
	}
	
	// synonym to peek(index)
	virtual inline const Type& get(int32_t index) const throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			if (m_stack[index] != NULL)
				return *(m_stack[index]);
			throw eEmpty;
		}
		throw eOutOfRange;
	}
	
	virtual inline int32_t search(const Type& value) const {
		if (m_count > 0) {
			for (int i = m_count-1; i >= 0; i--) {
				if (m_stack[i] != NULL) {
					if (*(m_stack[i]) == value) return i;
				} else if (value == nullValue) {
					return i;
				}
			}
		}
		return -1;
	}
	
	// synonym to search(value)
	virtual inline int32_t indexOf(const Type& value) const {
		if (m_count > 0) {
			for (int i = m_count-1; i >= 0; i--) {
				if (m_stack[i] != NULL) {
					if (*(m_stack[i]) == value) return i;
				} else if (value == nullValue) {
					return i;
				}
			}
		}
		return -1;
	}
	
	virtual inline const Type remove(int32_t index) throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			m_count--;
			Type* object = m_stack[index];
			if ((m_count > 0) && (index < m_count)) {
				memmove(m_stack + index, m_stack + index + 1, (m_count - index) * sizeof(Type*));
			}
			m_stack[m_count] = NULL;
			Type result = Type(*object);
			memDelete(object);
			return result;
		}
		throw eOutOfRange;
	}
};

template <typename Type>
const Type RefStack<Type>::nullValue;

template <typename Type>
class Stack : public Object {
public:
	RefClass(Stack, Stack<Type>)
	
	inline Stack(uint32_t initialCapacity) throw(const char*) {
		RefStack<Type>* o = memNew(o, RefStack<Type>(initialCapacity));
		if (o == NULL) throw eOutOfMemory;
		this->setRef(o);
	}
	
	virtual inline bool empty() const { CHECKTHIS; return THIS->empty(); }
	virtual inline const Type& peek() const { CHECKTHIS; return THIS->peek(); }
	virtual inline Type pop() { CHECKTHIS; return THIS->pop(); }
	virtual inline Type shift() { CHECKTHIS; return THIS->shift(); }
	virtual inline Type& push(const Type& object) { CHECKTHIS; return THIS->push(object); }
	virtual inline Type& unshift(const Type& object) { CHECKTHIS; return THIS->unshift(object); }
	virtual inline int32_t search(const Type& value) const { CHECKTHIS; return THIS->search(value); }
	virtual inline int32_t size() const { CHECKTHIS; return THIS->size(); }
	virtual inline void clear() throw(const char*) { CHECKTHIS; THIS->clear(); }
	
	inline static void _test() {
		Stack<Object> test = new Stack<Object>();
		test.empty();
		test.peek();
		test.pop();
		test.push(null);
		test.search(null);
		test.clear();
	}
};

#endif //JAPPSY_USTACK_H