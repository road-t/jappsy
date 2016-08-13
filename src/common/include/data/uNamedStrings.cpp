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

#include "uNamedStrings.h"
#include <core/uMemory.h>
#include <data/uString.h>

int32_t NamedStrings::find(const wchar_t* key) {
	if (m_count > 0) {
		for (uint32_t i = 0; i < m_count; i++) {
			if ((m_keys[i] == key) || (wcscmp(m_keys[i], key) == 0)) {
				return (int32_t)i;
			}
		}
	}
	return -1;
}

NamedStrings::NamedStrings() {
	m_keys = NULL;
	m_values = NULL;
	m_count = 0;
}

NamedStrings::~NamedStrings() {
	if (m_count > 0) {
		memFree(m_keys);
		m_keys = NULL;
		memFree(m_values);
		m_values = NULL;
		
		m_count = 0;
	}
}

const wchar_t* NamedStrings::get(const wchar_t* key) {
	int32_t index = find(key);
	if (index >= 0)
		return m_values[index];
	return NULL;
}

void NamedStrings::remove(const wchar_t* key) {
	int32_t index = find(key);
	if (index >= 0) {
		if (index < m_count - 1) {
			memmove(&m_keys[index], &m_keys[index+1], (m_count-index-1) * sizeof(const wchar_t*));
			memmove(&m_values[index], &m_values[index+1], (m_count-index-1) * sizeof(const wchar_t*));
		}
		m_count--;
		
		if (m_count == 0) {
			memFree(m_keys);
			m_keys = NULL;
			memFree(m_values);
			m_values = NULL;
		}
	}
	
}

void NamedStrings::insert(const wchar_t* key, const wchar_t* value) throw(const char*) {
	int32_t index = find(key);
	if (index >= 0) {
		m_values[index] = value;
		return;
	}
	
	const wchar_t** newKeys = memRealloc(const wchar_t*, newKeys, m_keys, (m_count+1) * sizeof(const wchar_t*));
	const wchar_t** newValues = memRealloc(const wchar_t*, newValues, m_values, (m_count+1) * sizeof(const wchar_t*));
	
	if (newKeys != NULL)
		m_keys = newKeys;
		if (newValues != NULL)
			m_values = newValues;
			
			if ((newKeys == NULL) || (newValues == NULL))
				throw eOutOfMemory;
	
	m_keys[m_count] = key;
	m_values[m_count] = value;
	
	m_count++;
}
