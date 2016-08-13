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
		this->TYPE = TypeListIterator;
	}
	
	inline RefListIterator(uint32_t initialCapacity) throw(const char*) : RefIterator<Type>(initialCapacity) {
		this->TYPE = TypeListIterator;
	}
	
	virtual inline void add(const Type& value) throw(const char*) {
		int32_t index = this->m_next;
		if ((index >= 0) && (index <= this->m_count)) {
			if ((this->m_count > 0) && (this->m_initialCapacity > 0) && (this->m_count >= this->m_initialCapacity)) {
				if (index > 0) {
					memDelete(this->m_stack[0]);
					if (index > 1)
						memmove(this->m_stack, this->m_stack + 1, (index - 1) * sizeof(Type*));
					index--;
					try {
						Type* item = memNew(item, Type(value));
						if (item == NULL) throw eOutOfMemory;
						this->m_stack[index] = item;
					} catch (...) {
						this->m_stack[index] = NULL;
						throw;
					}
				} else {
					memDelete(this->m_stack[this->m_count - 1]);
					if (this->m_count > 1)
						memmove(this->m_stack + 1, this->m_stack, (this->m_count - 1) * sizeof(Type*));
					try {
						Type* item = memNew(item, Type(value));
						if (item == NULL) throw eOutOfMemory;
						this->m_stack[0] = item;
					} catch (...) {
						this->m_stack[0] = NULL;
						throw;
					}
				}
			}
			
			this->resize(this->m_count + 1);
			if (index < this->m_count)
				memmove(this->m_stack + index + 1, this->m_stack + index, (this->m_count - index) * sizeof(Type*));
			
			this->m_count++;
			this->m_next++;
			try {
				Type* item = memNew(item, Type(value));
				if (item == NULL) throw eOutOfMemory;
				this->m_stack[index] = item;
			} catch (...) {
				this->m_stack[index] = NULL;
				throw;
			}
		} else
			throw eOutOfRange;
	}
	
	virtual inline bool hasPrevious() const {
		return (this->m_prev >= 0);
	}
	
	virtual inline int32_t nextIndex() const {
		if (this->m_next >= this->m_count)
			return this->m_count;
		
		return this->m_next;
	}
	
	virtual inline const Type& previous() const throw(const char*) {
		if (this->m_prev < 0) {
			((RefIterator<Type>*)this)->m_last = -1;
			throw eOutOfRange;
		}
		
		((RefIterator<Type>*)this)->m_last = this->m_prev;
		((RefIterator<Type>*)this)->m_next = this->m_prev;
		((RefIterator<Type>*)this)->m_prev--;
		return RefStack<Type>::peek(this->m_last);
	}
	
	virtual inline int32_t previousIndex() const {
		if (this->m_prev < 0)
			return -1;
		
		return this->m_prev;
	}
	
	virtual inline void set(const Type& value) throw(const char*) {
		int32_t index = this->m_last;
		if ((index >= 0) && (index < this->m_count)) {
			Type* item = this->m_stack[index];
			if (item != NULL) {
				memDelete(item);
			}
			try {
				item = memNew(item, Type(value));
				if (item == NULL) throw eOutOfMemory;
				this->m_stack[index] = item;
			} catch (...) {
				this->m_stack[index] = NULL;
				throw;
			}
		}
	}
};

template <typename Type>
class ListIterator : public Iterator<Type> {
public:
	RefClass(ListIterator, ListIterator<Type>)
	
	inline ListIterator(uint32_t initialCapacity) throw(const char*) {
		RefListIterator<Type>* o = memNew(o, RefListIterator<Type>(initialCapacity));
		if (o == NULL) throw eOutOfMemory;
		this->setRef(o);
	}
	
	virtual inline void add(const Type& value) throw(const char*) { CHECKTHIS; THIS->add(value); }
	virtual inline bool hasNext() const throw(const char*) { CHECKTHIS; return THIS->RefIterator<Type>::hasNext(); }
	virtual inline bool hasPrevious() const throw(const char*) { CHECKTHIS; return THIS->hasPrevious(); }
	virtual inline const Type& next() const throw(const char*) { CHECKTHIS; return THIS->RefIterator<Type>::next(); }
	virtual inline int32_t nextIndex() const throw(const char*) { CHECKTHIS; return THIS->nextIndex(); }
	virtual inline const Type& previous() const throw(const char*) { CHECKTHIS; return THIS->previous(); }
	virtual inline int32_t previousIndex() const throw(const char*) { CHECKTHIS; return THIS->previousIndex(); }
	virtual inline const Type remove() throw(const char*) { CHECKTHIS; return THIS->RefIterator<Type>::remove(); }
	virtual inline void set(const Type& value) throw(const char*) { CHECKTHIS; return THIS->set(value); }
	
	inline static void _test() {
		ListIterator<Object> lit = new ListIterator<Object>();
		lit.add(null);
		lit.hasNext();
		lit.hasPrevious();
		lit.next();
		lit.nextIndex();
		lit.previous();
		lit.previousIndex();
		lit.remove();
		lit.set(null);
	}
};

#endif //JAPPSY_ULISTITERATOR_H