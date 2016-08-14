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
class RefIterator : public RefStack<Type> {
public:
	int32_t m_next = 0;
	int32_t m_last = -1;
	int32_t m_prev = -1;
	
public:
	inline RefIterator() : RefStack<Type>() {
		this->TYPE = TypeIterator;
	}
	
	inline RefIterator(uint32_t initialCapacity) throw(const char*) : RefStack<Type>(initialCapacity) {
		this->TYPE = TypeIterator;
	}
	
	virtual inline bool hasNext() const {
		return (m_next < this->m_count);
	}
	
	virtual inline const Type& next() const throw(const char*) {
		if (m_next >= this->m_count) {
			((RefIterator*)this)->m_last = this->m_count;
			throw eOutOfRange;
		}
		
		((RefIterator*)this)->m_last = this->m_next;
		((RefIterator*)this)->m_prev = this->m_next;
		((RefIterator*)this)->m_next++;
		return RefStack<Type>::peek(this->m_last);
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
			return RefStack<Type>::remove(index);
		}
		throw eOutOfRange;
	}
	
	virtual inline void reset(uint32_t index = 0) const {
		((RefIterator*)this)->m_last = -1;
		if ((index >= 0) && (index < this->m_count)) {
			((RefIterator*)this)->m_next = index;
			((RefIterator*)this)->m_prev = index - 1;
		} else if (index >= this->m_count) {
			((RefIterator*)this)->m_next = this->m_count;
			((RefIterator*)this)->m_prev = this->m_count - 1;
		} else {
			((RefIterator*)this)->m_next = 0;
			((RefIterator*)this)->m_prev = -1;
		}
	}
};

template <typename Type>
class Iterator : public Stack<Type> {
public:
	RefClass(Iterator, Iterator<Type>)
	
	inline Iterator(uint32_t initialCapacity) throw(const char*) {
		RefIterator<Type>* o = new RefIterator<Type>(initialCapacity);
		if (o == NULL) throw eOutOfMemory;
		this->setRef(o);
	}
	
	virtual inline bool hasNext() const throw(const char*) { CHECKTHIS; return THIS->hasNext(); }
	virtual inline const Type& next() const throw(const char*) { CHECKTHIS; return THIS->next(); }
	virtual inline const Type remove() throw(const char*) { CHECKTHIS; return THIS->remove(); }
	virtual inline void reset(uint32_t index = 0) const throw(const char*) { CHECKTHIS; THIS->reset(index); }
	
#ifdef DEBUG
	inline static void _test() {
		Iterator<Object> it = new Iterator<Object>();
		it.push(null);
		it.hasNext();
		it.next();
		it.remove();
	}
#endif
};

#endif //JAPPSY_UITERATOR_H