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

#ifndef JAPPSY_UARRAY_H
#define JAPPSY_UARRAY_H

#include <platform.h>
#include <core/uMemory.h>
#include <data/uObject.h>
#include <data/uString.h>

template <typename Type>
class Array : public Object {
private:
	Type** m_values;
	uint32_t m_count;
	
	int32_t find(const Type* value) {
		if (m_count > 0) {
			for (uint32_t i = 0; i < m_count; i++) {
				if (m_values[i] == value) {
					return (int32_t)i;
				}
			}
		}
		return -1;
	}
	
public:
	Array() {
		m_values = NULL;
		m_count = 0;
	}
	
	~Array() {
		if (m_count > 0) {
			for (uint32_t i = 0; i < m_count; i++) {
				memDelete(m_values[i]);
			}
			
			memFree(m_values);
			m_values = NULL;
			
			m_count = 0;
		}
	}
	
	Type* get(int32_t index) {
		if ((index >= 0) && (index < m_count))
			return m_values[index];
		return NULL;
	}
	
	void remove(int32_t index) {
		if ((index >= 0) && (index < m_count)) {
			memDelete(m_values[index]);
			
			if (index < m_count - 1) {
				memmove(&m_values[index], &m_values[index+1], (m_count-index-1) * sizeof(Type*));
			}
			m_count--;
			
			if (m_count == 0) {
				memFree(m_values);
				m_values = NULL;
			}
		}
		
	}
	
	void insert(Type* value) throw(const char*) {
		int32_t index = find(value);
		if (index >= 0) {
			return;
		}
		
		Type** newValues = memRealloc(Type*, newValues, m_values, (m_count+1) * sizeof(Type*));
		
		if (newValues != NULL)
			m_values = newValues;
		
		if (newValues == NULL)
			throw eOutOfMemory;
		
		m_values[m_count] = value;
		
		m_count++;
	}
};

#endif //JAPPSY_UCONTAINER_H