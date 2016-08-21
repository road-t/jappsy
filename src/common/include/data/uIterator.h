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

#ifndef JAPPSY_UITERATOR_H
#define JAPPSY_UITERATOR_H

#include <data/uStack.h>

template <typename Type>
class JRefIterator : public JRefStack<Type> {
public:
	int32_t m_next = 0;
	int32_t m_last = -1;
	int32_t m_prev = -1;
	
public:
	inline JRefIterator() : JRefStack<Type>() {
		THIS.TYPE = TypeIterator;
	}
	
	inline JRefIterator(uint32_t initialCapacity) throw(const char*) : JRefStack<Type>(initialCapacity) {
		THIS.TYPE = TypeIterator;
	}
	
	virtual inline bool hasNext() const {
		return (m_next < THIS.m_count);
	}
	
	virtual inline const Type& next() const throw(const char*) {
		if (m_next >= THIS.m_count) {
			((JRefIterator*)this)->m_last = THIS.m_count;
			throw eOutOfRange;
		}
		
		((JRefIterator*)this)->m_last = THIS.m_next;
		((JRefIterator*)this)->m_prev = THIS.m_next;
		((JRefIterator*)this)->m_next++;
		return JRefStack<Type>::peek(THIS.m_last);
	}
	
	virtual inline const Type remove() throw(const char*) {
		if (m_last >= 0) {
			int32_t index = m_last;
			if (m_last < m_next) {
				m_next--;
			}
			if (m_prev <= m_last) {
				if (m_prev >= 0) m_prev--;
			}
			m_last = -1;
			return JRefStack<Type>::remove(index);
		}
		throw eOutOfRange;
	}
	
	virtual inline void reset(uint32_t index = 0) const {
		((JRefIterator*)this)->m_last = -1;
		if ((index >= 0) && (index < THIS.m_count)) {
			((JRefIterator*)this)->m_next = index;
			((JRefIterator*)this)->m_prev = index - 1;
		} else if (index >= THIS.m_count) {
			((JRefIterator*)this)->m_next = THIS.m_count;
			((JRefIterator*)this)->m_prev = THIS.m_count - 1;
		} else {
			((JRefIterator*)this)->m_next = 0;
			((JRefIterator*)this)->m_prev = -1;
		}
	}
};

template <typename Type>
class JIterator : public JStack<Type> {
public:
	JRefTemplate(JIterator, JIterator, JRefIterator)

	inline JIterator() {
		THIS.initialize();
	}
	
	inline JIterator(uint32_t initialCapacity) throw(const char*) {
		THIS.initialize();
		JRefIterator<Type>* o = new JRefIterator<Type>(initialCapacity);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	virtual inline bool hasNext() const throw(const char*) { return THIS.ref().hasNext(); }
	virtual inline const Type& next() const throw(const char*) { return THIS.ref().next(); }
	virtual inline const Type remove() throw(const char*) { return THIS.ref().remove(); }
	virtual inline void reset(uint32_t index = 0) const throw(const char*) { THIS.ref().reset(index); }
};

template <typename Type>
class CIterator : public CStack<Type> {
public:
	int32_t m_next = 0;
	int32_t m_last = -1;
	int32_t m_prev = -1;
	
public:
	inline CIterator(void (*onrelease)(Type& value) = NULL) : CStack<Type>(onrelease) { }
	inline CIterator(uint32_t initialCapacity, void (*onrelease)(Type& value) = NULL) throw(const char*) : CStack<Type>(initialCapacity, onrelease) { }
	
	virtual inline bool hasNext() const {
		return (m_next < THIS.m_count);
	}
	
	virtual inline const Type& next() const throw(const char*) {
		if (m_next >= THIS.m_count) {
			((CIterator*)this)->m_last = THIS.m_count;
			throw eOutOfRange;
		}
		
		((CIterator*)this)->m_last = THIS.m_next;
		((CIterator*)this)->m_prev = THIS.m_next;
		((CIterator*)this)->m_next++;
		return CStack<Type>::peek(THIS.m_last);
	}
	
	virtual inline const Type remove() throw(const char*) {
		if (m_last >= 0) {
			int32_t index = m_last;
			if (m_last < m_next) {
				m_next--;
			}
			if (m_prev <= m_last) {
				if (m_prev >= 0) m_prev--;
			}
			m_last = -1;
			return CStack<Type>::remove(index);
		}
		throw eOutOfRange;
	}
	
	virtual inline void reset(uint32_t index = 0) const {
		((CIterator*)this)->m_last = -1;
		if ((index >= 0) && (index < THIS.m_count)) {
			((CIterator*)this)->m_next = index;
			((CIterator*)this)->m_prev = index - 1;
		} else if (index >= THIS.m_count) {
			((CIterator*)this)->m_next = THIS.m_count;
			((CIterator*)this)->m_prev = THIS.m_count - 1;
		} else {
			((CIterator*)this)->m_next = 0;
			((CIterator*)this)->m_prev = -1;
		}
	}
};

#endif //JAPPSY_UITERATOR_H