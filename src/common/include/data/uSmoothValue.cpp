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

#include "uSmoothValue.h"
#include <core/uMemory.h>

SmoothValue::SmoothValue(uint32_t count) throw(const char*) {
	m_items = NULL;
	m_size = m_count = 0;

	if (count > 0) {
		m_items = memAlloc(float, m_items, count * sizeof(float));
		if (m_items == NULL)
			throw eOutOfMemory;
		m_size = count;
	}
}

SmoothValue::~SmoothValue() {
	if (m_items != NULL) {
		memFree(m_items);
		m_items = NULL;
		m_size = m_count = 0;
	}
}

void SmoothValue::put(float v) {
	if (m_count > 0) {
		uint32_t len = m_count;
		if (len >= m_size) len = m_size - 1;

		for (uint32_t i = len; i > 0; i--)
			m_items[i] = m_items[i-1];
	}
	m_items[0] = v;
	if (m_count < m_size)
		m_count++;
}

float SmoothValue::value() {
	if (m_count > 0) {
		float total = 0;
		for (uint32_t i = 0; i < m_count; i++)
			total += m_items[i];
		return total / m_count;
	}
	return 0;
}
