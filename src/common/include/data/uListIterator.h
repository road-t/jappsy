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

#ifndef JAPPSY_ULISTITERATOR_H
#define JAPPSY_ULISTITERATOR_H

#include <data/uIterator.h>

template <typename Type>
class RefListIterator : public RefIterator<Type> {
public:
	inline RefListIterator() : RefIterator<Type>() {
		THIS.TYPE = TypeListIterator;
	}
	
	inline RefListIterator(uint32_t initialCapacity) throw(const char*) : RefIterator<Type>(initialCapacity) {
		THIS.TYPE = TypeListIterator;
	}
	
	virtual inline bool add(const Type& value) throw(const char*) {
		int32_t index = THIS.m_next;
		if ((index >= 0) && (index <= THIS.m_count)) {
			if ((THIS.m_count > 0) && (THIS.m_initialCapacity > 0) && (THIS.m_count >= THIS.m_initialCapacity)) {
				if (index > 0) {
					delete THIS.m_stack[0];
					if (index > 1)
						memmove(THIS.m_stack, THIS.m_stack + 1, (index - 1) * sizeof(Type*));
					index--;
					try {
						Type* item = new Type(value);
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
						Type* item = new Type(value);
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
				THIS.m_next++;
				try {
					Type* item = new Type(value);
					if (item == NULL) throw eOutOfMemory;
					THIS.m_stack[index] = item;
				} catch (...) {
					THIS.m_stack[index] = NULL;
					throw;
				}
			}
		} else
			throw eOutOfRange;
		
		return true;
	}
	
	virtual inline bool hasPrevious() const {
		return (THIS.m_prev >= 0);
	}
	
	virtual inline int32_t nextIndex() const {
		if (THIS.m_next >= THIS.m_count)
			return THIS.m_count;
		
		return THIS.m_next;
	}
	
	virtual inline const Type& previous() const throw(const char*) {
		if (THIS.m_prev < 0) {
			((RefIterator<Type>*)this)->m_last = -1;
			throw eOutOfRange;
		}
		
		((RefIterator<Type>*)this)->m_last = THIS.m_prev;
		((RefIterator<Type>*)this)->m_next = THIS.m_prev;
		((RefIterator<Type>*)this)->m_prev--;
		return JRefStack<Type>::peek(THIS.m_last);
	}
	
	virtual inline int32_t previousIndex() const {
		if (THIS.m_prev < 0)
			return -1;
		
		return THIS.m_prev;
	}
	
	virtual inline Type& set(const Type& value) throw(const char*) {
		int32_t index = THIS.m_last;
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
};

template <typename Type>
class ListIterator : public Iterator<Type> {
public:
	JRefTemplate(ListIterator, ListIterator, RefListIterator)
	
	inline ListIterator() {
		THIS.initialize();
	}
	
	inline ListIterator(uint32_t initialCapacity) throw(const char*) {
		THIS.initialize();
		RefListIterator<Type>* o = new RefListIterator<Type>(initialCapacity);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	virtual inline bool add(const Type& value) throw(const char*) { return THIS.ref().add(value); }
	virtual inline bool hasNext() const throw(const char*) { return THIS.ref().RefIterator<Type>::hasNext(); }
	virtual inline bool hasPrevious() const throw(const char*) { return THIS.ref().hasPrevious(); }
	virtual inline const Type& next() const throw(const char*) { return THIS.ref().RefIterator<Type>::next(); }
	virtual inline int32_t nextIndex() const throw(const char*) { return THIS.ref().nextIndex(); }
	virtual inline const Type& previous() const throw(const char*) { return THIS.ref().previous(); }
	virtual inline int32_t previousIndex() const throw(const char*) { return THIS.ref().previousIndex(); }
	virtual inline const Type remove() throw(const char*) { return THIS.ref().RefIterator<Type>::remove(); }
	virtual inline Type& set(const Type& value) throw(const char*) { return THIS.ref().set(value); }
};

#endif //JAPPSY_ULISTITERATOR_H