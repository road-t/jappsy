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

#ifndef JAPPSY_USPARSEARRAY_H
#define JAPPSY_USPARSEARRAY_H

#include <data/uObject.h>
#include <data/uJSON.h>

#define SPARSEARRAY_BLOCK_SIZE	16

template <typename Type>
class RefSparseArray : public RefObject {
protected:
	struct RefSparseArrayItem {
		int32_t index;
		Type* value;
	};

	RefSparseArrayItem* m_array;
	uint32_t m_count = 0;
	uint32_t m_size = 0;
	uint32_t m_initialSize = 0;
	
	inline void resize(uint32_t count) throw(const char*) {
		if (count < m_count) {
			for (int i = count; i < m_count; i++) {
				if (m_array[i].value != NULL) {
					delete m_array[i].value;
					m_array[i].value = NULL;
				}
			}
		}
		
		uint32_t size = count - (count % SPARSEARRAY_BLOCK_SIZE) + SPARSEARRAY_BLOCK_SIZE;
		if ((size > m_size) && (size >= m_initialSize)) {
			RefSparseArrayItem* newArray = memRealloc(RefSparseArrayItem, newArray, m_array, size * sizeof(RefSparseArrayItem));
			if (newArray) {
				m_array = newArray;
				m_size = size;
			} else
				throw eOutOfMemory;
		}
	}
	
	inline void autorelease(uint32_t count) throw(const char*) {
		if (count < m_count) {
			for (int i = count; i < m_count; i++) {
				if (m_array[i].value != NULL) {
					delete m_array[i].value;
					m_array[i].value = NULL;
				}
			}
		}
		
		uint32_t size = count - (count % SPARSEARRAY_BLOCK_SIZE) + SPARSEARRAY_BLOCK_SIZE;
		if (size < m_initialSize) size = m_initialSize;
		if (size < m_size) {
			RefSparseArrayItem* newArray = memRealloc(RefSparseArrayItem, newArray, m_array, size * sizeof(RefSparseArrayItem));
			if (newArray) {
				m_array = newArray;
				m_size = size;
			} else
				throw eOutOfMemory;
		}
	}

public:
	
	inline RefSparseArray() {
		TYPE = TypeSparseArray;
	}
	
	inline RefSparseArray(uint32_t initialCapacity) throw(const char*) {
		TYPE = TypeSparseArray;
		resize(initialCapacity);
		m_initialSize = m_size;
	}
	
	inline ~RefSparseArray() {
		if (m_array != NULL) {
			for (int i = 0; i < m_count; i++) {
				if (m_array[i].value != NULL) {
					delete m_array[i].value;
				}
			}
			memFree(m_array);
		}
	}
	
	virtual inline void append(int32_t key, const Type& value) throw(const char*) {
		resize(m_count + 1);
		
		int insertIndex = -1;
		for (int index = 0; index < m_count; index++) {
			if (m_array[index].index <= key) {
				insertIndex = index;
			}
		}
		insertIndex++;
		if (insertIndex < m_count) {
			memmove(&(m_array[insertIndex+1]), &(m_array[insertIndex]), (m_count - insertIndex) * sizeof(RefSparseArrayItem));
		}

		m_array[insertIndex].index = key;
		try {
			Type* item = new Type(value);
			if (item == NULL) throw eOutOfMemory;
			m_array[insertIndex].value = item;
		} catch (...) {
			m_array[insertIndex].value = NULL;
			throw;
		}
		m_count++;
	}
	
	virtual inline void clear() throw(const char*) {
		autorelease(0);
		m_count = 0;
	}
	
	virtual inline const Type& get(int32_t key) const throw(const char*) {
		for (int index = 0; index < m_count; index++) {
			if (m_array[index].index == key) {
				return *(m_array[index].value);
			}
		}
		throw eNotFound;
	}
	
	virtual inline const Type& opt(int32_t key, const Type& defaultValue) const {
		for (int index = 0; index < m_count; index++) {
			if (m_array[index].index == key) {
				return *(m_array[index].value);
			}
		}
		return defaultValue;
	}
	
	virtual inline int32_t indexOfKey(int32_t key) const {
		for (int index = 0; index < m_count; index++) {
			if (m_array[index].index == key) {
				return index;
			}
		}
		return -1;
	}
	
	virtual inline int32_t indexOfValue(const Type& value) const {
		for (int index = 0; index < m_count; index++) {
			if (*((Type*)(m_array[index].value)) == value) {
				return index;
			}
		}
		return -1;
	}
	
	virtual inline int32_t keyAt(int index) const throw(const char*) {
		if ((index >= 0) && (index < m_count))
			return m_array[index].index;

		throw eOutOfRange;
	}
	
	virtual inline Type& put(int32_t key, const Type& value) throw(const char*) {
		resize(m_count + 1);
		
		m_array[m_count].index = key;
		try {
			Type* item = new Type(value);
			if (item == NULL) throw eOutOfMemory;
			m_array[m_count].value = item;
			m_count++;
			return *item;
		} catch (...) {
			m_array[m_count].value = NULL;
			throw;
		}
	}
	
	virtual inline void remove(int32_t key) {
		int32_t count = m_count;
		int index = 0;
		while (index < count) {
			if (m_array[index].index == key) {
				Type* item = m_array[index].value;
				if (item != NULL) {
					delete item;
				}
				
				if (index < (m_count-1)) {
					memmove(&(m_array[index]), &(m_array[index+1]), (m_count - index - 1) * sizeof(RefSparseArrayItem));
				}
				
				count--;
				m_array[count].value = NULL;
			} else {
				index++;
			}
		}
		m_count = count;
	}
	
	virtual inline void removeAt(int index) throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			Type* item = m_array[index].value;
			if (item != NULL) {
				delete item;
			}
			
			if (index < (m_count-1)) {
				memmove(&(m_array[index]), &(m_array[index+1]), (m_count - index - 1) * sizeof(RefSparseArrayItem));
			}
			
			m_count--;
			m_array[m_count].value = NULL;
		} else
			throw eOutOfRange;
	}
	
	virtual inline void removeAtRange(int index, int size) throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			if ((index + size) > m_count)
				throw eOutOfRange;

			for (int ofs = 0; ofs < size; ofs++) {
				Type* item = m_array[index + ofs].value;
				if (item != NULL) {
					delete item;
				}
			}
			if ((index + size) < m_count) {
				memmove(&(m_array[index]), &(m_array[index + size]), (m_count - index - size) * sizeof(RefSparseArrayItem));
			}
			for (int ofs = 0; ofs < size; ofs++) {
				m_array[m_count - ofs - 1].value = NULL;
			}
			m_count -= size;
		} else if (size != 0)
			throw eOutOfRange;
	}
	
	virtual inline void setValueAt(int index, const Type& value) throw(const char*) {
		if ((index >= 0) && (index < m_count)) {
			Type* item = m_array[index].value;
			if (item != NULL) {
				delete item;
			}
			try {
				item = new Type(value);
				if (item == NULL) throw eOutOfMemory;
				m_array[index].value = item;
			} catch (...) {
				m_array[index].value = NULL;
				throw;
			}
		} else
			throw eOutOfRange;
	}
	
	virtual inline int32_t size() const {
		return m_count;
	}
	
	virtual inline const Type& valueAt(int index) const throw(const char*) {
		if ((index >= 0) && (index < m_count))
			return *(m_array[index].value);
		
		throw eOutOfRange;
	}
	
	virtual inline String toJSON() const {
		String json = L"{";
		for (int i = 0; i < m_count; i++) {
			if (i != 0) json += L",";
			json += L"\"";
			json += m_array[i].index;
			json += L"\":";
			json += JSON::stringify(*m_array[i].value);
		}
		json += L"}";
		return json;
	}
};

template <typename Type>
class SparseArray : public Object {
public:
	RefTemplate(SparseArray, SparseArray, RefSparseArray)
	
	inline SparseArray() {
		THIS.initialize();
	}
	
	inline SparseArray(uint32_t initialCapacity) {
		THIS.initialize();
		RefSparseArray<Type>* o = new RefSparseArray<Type>(initialCapacity);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	virtual inline void append(int32_t key, const Type& value) throw(const char*) { THIS.ref().append(key, value); }
	virtual inline void clear() throw(const char*) { THIS.ref().clear(); }
	virtual inline const Type& get(int32_t key) const throw(const char*) { return THIS.ref().get(key); }
	virtual inline const Type& opt(int32_t key, const Type& defaultValue) const throw(const char*) { return THIS.ref().opt(key, defaultValue); }
	virtual inline int32_t indexOfKey(int32_t key) const throw(const char*) { return THIS.ref().indexOfKey(key); }
	virtual inline int32_t indexOfValue(const Type& value) const throw(const char*) { return THIS.ref().indexOfValue(value); }
	virtual inline int32_t keyAt(int index) const throw(const char*) { return THIS.ref().keyAt(index); }
	virtual inline Type& put(int32_t key, const Type& value) throw(const char*) { return THIS.ref().put(key, value); }
	virtual inline void remove(int32_t key) throw(const char*) { THIS.ref().remove(key); }
	virtual inline void removeAt(int index) throw(const char*) { THIS.ref().removeAt(index); }
	virtual inline void removeAtRange(int index, int size) throw(const char*) { THIS.ref().removeAtRange(index, size); }
	virtual inline void setValueAt(int index, const Type& value) throw(const char*) { THIS.ref().setValueAt(index, value); }
	virtual inline int32_t size() const throw(const char*) { return THIS.ref().size(); }
	virtual inline const Type& valueAt(int index) const throw(const char*) { return THIS.ref().valueAt(index); }
};

#endif //JAPPSY_USPARSEARRAY_H