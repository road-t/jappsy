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

#include "uStream.h"
#include <core/uMemory.h>
#include <data/uString.h>

Stream::Stream(const wchar_t* data) throw(const char*) {
	m_allocated = false;
	m_buffer = NULL;
	m_size = 0;
	m_position = 0;
	
	if (data != NULL) {
		uint32_t size = wcs_toutf8_size(data);
		if (size != 0) {
			m_buffer = memAlloc(uint8_t, m_buffer, size);
			if (m_buffer == NULL)
				throw eOutOfMemory;
			wcs_toutf8(data, (char*)m_buffer, size);
			m_allocated = true;
			m_size = size - 1;
		}
	}
}

Stream::Stream(const char* data) {
	m_allocated = false;
	m_buffer = NULL;
	m_size = 0;
	m_position = 0;

	if (data != NULL) {
		uint32_t size = 0;
		uint32_t length = utf8_strlen(data, &size);
		if (length > 0) {
			m_buffer = (uint8_t*)data;
			m_size = size - 1;
		}
	}
}

Stream::Stream(const char* data, bool autorelease) {
	m_allocated = false;
	m_buffer = NULL;
	m_size = 0;
	m_position = 0;
	
	if (data != NULL) {
		uint32_t size = 0;
		uint32_t length = utf8_strlen(data, &size);
		if (length > 0) {
			m_buffer = (uint8_t*)data;
			m_size = size - 1;
			m_allocated = autorelease;
		}
	}
}

Stream::Stream(const void* data, uint32_t length) {
	m_allocated = false;
	m_buffer = NULL;
	m_size = 0;
	m_position = 0;
	
	if ((data != NULL) && (length != 0)) {
		m_buffer = (uint8_t*)data;
		m_size = length;
	}
}

Stream::Stream(const void* data, uint32_t length, bool autorelease) {
	m_allocated = false;
	m_buffer = NULL;
	m_size = 0;
	m_position = 0;
	
	if ((data != NULL) && (length != 0)) {
		m_buffer = (uint8_t*)data;
		m_size = length;
		m_allocated = autorelease;
	}
}

Stream::~Stream() {
	if (m_allocated) {
		memFree(m_buffer);
	}
	
	m_allocated = false;
	m_buffer = NULL;
	m_size = 0;
	m_position = 0;
}

uint8_t* Stream::readBytes(uint32_t length) throw(const char*) {
	if (length == 0)
		return NULL;
	
	uint32_t end = m_position + length;
	if (end > m_size)
		throw eIOReadLimit;
	
	uint32_t size = end - m_position;
	uint8_t* data = memAlloc(uint8_t, data, size);
	if (data == NULL)
		throw eOutOfMemory;
	
	memcpy(data, m_buffer + m_position, size);
	m_position = end;
	return data;
}

uint32_t Stream::readInt() throw(const char*) {
	uint32_t end = m_position + 4;
	if (end > m_size)
		throw eIOReadLimit;

	uint32_t value = readUInt32(m_buffer, m_position);
	m_position = end;
	return value;
}

uint8_t Stream::readUnsignedByte() throw(const char*) {
	uint32_t end = m_position + 1;
	if (end > m_size)
		throw eIOReadLimit;
	
	uint8_t value = readUInt8(m_buffer, m_position);
	m_position = end;
	return value;
}

int32_t Stream::skip(uint32_t length) {
	uint32_t end = m_position + length;
	if (end > m_size)
		return -1;

	m_position = end;
	return (int32_t)length;
}
