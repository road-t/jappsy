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
#include <data/uJSON.h>

#define STACK_BLOCK_SIZE	16

template <typename Type>
class JRefStack : public JRefObject {
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
					delete m_stack[i];
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
					delete m_stack[i];
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
	inline JRefStack() {	TYPE = TypeStack; }
	
	inline JRefStack(uint32_t initialCapacity) throw(const char*) {
		TYPE = TypeStack;
		resize(initialCapacity);
		m_initialCapacity = initialCapacity;
		m_initialSize = m_size;
	}
	
	inline ~JRefStack() {
		if (m_stack != NULL) {
			for (int i = 0; i < m_count; i++) {
				if (m_stack[i] != NULL) {
					delete m_stack[i];
				}
			}
			memFree(m_stack);
			m_stack = NULL;
		}
	}
	
	virtual inline Type& push(const Type& object) throw(const char*) {
		if ((m_count > 0) && (m_initialCapacity > 0) && (m_count >= m_initialCapacity)) {
			delete m_stack[0];
			if (m_count > 1) {
				memmove(m_stack, m_stack + 1, (m_count - 1) * sizeof(Type*));
			}
			try {
				Type* newObject = new Type(object);
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
				Type* newObject = new Type(object);
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
			delete m_stack[m_count - 1];
			if (m_count > 1) {
				memmove(m_stack + 1, m_stack, (m_count - 1) * sizeof(Type*));
			}
			try {
				Type* newObject = new Type(object);
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
				Type* newObject = new Type(object);
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
			delete object;
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
			delete object;
			return result;
		}
		
		throw eEmpty;
	}
	
	virtual inline bool contains(const Type& value) const {
		if (m_count > 0) {
			for (int i = m_count-1; i >= 0; i--) {
				if (m_stack[i] != NULL) {
					if (*(m_stack[i]) == value) return true;
				} else if (JObject::isNull(value)) {
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
				} else if (JObject::isNull(value)) {
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
				} else if (JObject::isNull(value)) {
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
			delete object;
			return result;
		}
		throw eOutOfRange;
	}
	
	virtual inline JString toJSON() const {
		JString json = L"[";
		for (int i = 0; i < m_count; i++) {
			if (i != 0) json += L",";
			json += JSON::stringify(*(m_stack[i]));
		}
		json += L"]";
		return json;
	}

};

template <typename Type>
class JStack : public JObject {
public:
	JRefTemplate(JStack, JStack, JRefStack)

	inline JStack() {
		THIS.initialize();
	}
	
	inline JStack(uint32_t initialCapacity) throw(const char*) {
		THIS.initialize();
		JRefStack<Type>* o = new JRefStack<Type>(initialCapacity);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	virtual inline bool empty() const { return THIS.ref().empty(); }
	virtual inline const Type& peek() const { return THIS.ref().peek(); }
	virtual inline Type pop() { return THIS.ref().pop(); }
	virtual inline Type shift() { return THIS.ref().shift(); }
	virtual inline Type& push(const Type& object) { return THIS.ref().push(object); }
	virtual inline Type& unshift(const Type& object) { return THIS.ref().unshift(object); }
	virtual inline int32_t search(const Type& value) const { return THIS.ref().search(value); }
	virtual inline int32_t size() const { return THIS.ref().size(); }
	virtual inline void clear() throw(const char*) { THIS.ref().clear(); }
};

template <typename Type>
class CStack : public CObject {
public:
	typedef void (*CStackCallback)(Type& value);

	Type* m_stack = NULL;
	uint32_t m_count = 0;
	uint32_t m_size = 0;
	uint32_t m_initialCapacity = 0;
	uint32_t m_initialSize = 0;
	CStackCallback m_onrelease = NULL;
	
	virtual inline void foreach(CStackCallback callback) {
		for (int i = 0; i < m_count; i++) {
			callback(m_stack[i]);
		}
	}
	
	inline void resize(uint32_t count) throw(const char*) {
		if ((count < m_count) && (m_onrelease != NULL)) {
			for (int i = count; i < m_count; i++) {
				m_onrelease(m_stack[i]);
			}
		}
		
		uint32_t size = count - (count % STACK_BLOCK_SIZE) + STACK_BLOCK_SIZE;
		if ((size > m_size) && (size >= m_initialSize)) {
			Type* newStack = memRealloc(Type, newStack, m_stack, size * sizeof(Type));
			if (newStack) {
				m_stack = newStack;
				m_size = size;
			} else
				throw eOutOfMemory;
		}
	}
	
	inline void autorelease(uint32_t count) throw(const char*) {
		if ((count < m_count) && (m_onrelease != NULL)) {
			for (int i = count; i < m_count; i++) {
				m_onrelease(m_stack[i]);
			}
		}
		
		uint32_t size = count - (count % STACK_BLOCK_SIZE) + STACK_BLOCK_SIZE;
		if (size < m_initialSize) size = m_initialSize;
		if (size < m_size) {
			Type* newStack = memRealloc(Type, newStack, m_stack, size * sizeof(Type));
			if (newStack) {
				m_stack = newStack;
				m_size = size;
			} else
				throw eOutOfMemory;
		}
	}
	
public:
	inline CStack(CStackCallback onrelease = NULL) { m_onrelease = onrelease; }
	
	inline CStack(uint32_t initialCapacity, CStackCallback onrelease = NULL) throw(const char*) {
		m_onrelease = onrelease;
		resize(initialCapacity);
		m_initialCapacity = initialCapacity;
		m_initialSize = m_size;
	}
	
	inline ~CStack() {
		if (m_stack != NULL) {
			if (m_onrelease != NULL) {
				for (int i = 0; i < m_count; i++) {
					if (m_stack[i] != NULL) {
						m_onrelease(m_stack[i]);
					}
				}
			}
			memFree(m_stack);
			m_stack = NULL;
		}
	}
	
	virtual inline Type& push(const Type& object) throw(const char*) {
		if ((m_count > 0) && (m_initialCapacity > 0) && (m_count >= m_initialCapacity)) {
			if (m_onrelease != NULL) m_onrelease(m_stack[0]);
			if (m_count > 1) {
				memmove(m_stack, m_stack + 1, (m_count - 1) * sizeof(Type));
			}
			return m_stack[m_count - 1] = object;
		} else {
			resize(m_count + 1);
			return m_stack[m_count++] = object;
		}
	}
	
	virtual inline Type& unshift(const Type& object) throw(const char*) {
		if ((m_count > 0) && (m_initialCapacity > 0) && (m_count >= m_initialCapacity)) {
			if (m_onrelease != NULL) m_onrelease(m_stack[m_count - 1]);
			if (m_count > 1) {
				memmove(m_stack + 1, m_stack, (m_count - 1) * sizeof(Type));
			}
			return m_stack[0] = object;
		} else {
			resize(m_count + 1);
			memmove(m_stack + 1, m_stack, m_count * sizeof(Type));
			m_count++;
			return m_stack[0] = object;
		}
	}
	
	virtual inline Type pop() throw(const char*) {
		if (m_count > 0) {
			m_count--;
			return m_stack[m_count];
		}
		
		throw eEmpty;
	}
	
	virtual inline Type shift() throw(const char*) {
		if (m_count > 0) {
			m_count--;
			Type object = m_stack[0];
			if (m_count > 0) {
				memmove(m_stack, m_stack + 1, m_count * sizeof(Type));
			}
			return object;
		}
		
		throw eEmpty;
	}
	
	virtual inline bool contains(const Type& value) const {
		if (m_count > 0) {
			for (int i = m_count-1; i >= 0; i--) {
				if (m_stack[i] == value) return true;
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
	
	virtual inline void clear() throw(const char*) {
		autorelease(0);
		m_count = 0;
	}
	
	virtual inline const Type& peek() const throw(const char*) {
		if (m_count > 0) {
			return m_stack[m_count-1];
		}
		
		throw eEmpty;
	}
	
	virtual inline const Type& peek(int32_t index) const throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			return m_stack[index];
		}
		
		throw eOutOfRange;
	}
	
	// synonym to peek(index)
	virtual inline const Type& get(int32_t index) const throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			return m_stack[index];
		}

		throw eOutOfRange;
	}
	
	virtual inline int32_t search(const Type& value) const {
		if (m_count > 0) {
			for (int i = m_count-1; i >= 0; i--) {
				if (m_stack[i] == value) return i;
			}
		}
		return -1;
	}
	
	// synonym to search(value)
	virtual inline int32_t indexOf(const Type& value) const {
		if (m_count > 0) {
			for (int i = m_count-1; i >= 0; i--) {
				if (m_stack[i] == value) return i;
			}
		}
		return -1;
	}
	
	virtual inline const Type remove(int32_t index) throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			m_count--;
			Type object = m_stack[index];
			if ((m_count > 0) && (index < m_count)) {
				memmove(m_stack + index, m_stack + index + 1, (m_count - index) * sizeof(Type));
			}
			return object;
		}
		throw eOutOfRange;
	}
};

#endif //JAPPSY_USTACK_H