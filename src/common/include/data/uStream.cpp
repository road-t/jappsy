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
#include <cipher/uCipher.h>
#include <core/uSystem.h>

Stream::Stream(const wchar_t* data) throw(const char*) {
	if (data != NULL) {
		uint32_t size = wcs_toutf8_size(data);
		if (size != 0) {
			m_buffer = memAlloc(uint8_t, m_buffer, size);
			if (m_buffer == NULL)
				throw eOutOfMemory;
			wcs_toutf8(data, (char*)m_buffer, size);
			m_allocated = true;
			m_limit = m_size = size - 1;
			m_modificationDate = currentTimeMillis();
			m_sourcePath = (const void*)NULL;
		}
	}
}

Stream::Stream(const char* data, bool autorelease) {
	if (data != NULL) {
		uint32_t size = 0;
		uint32_t length = utf8_strlen(data, &size);
		if (length > 0) {
			m_buffer = (uint8_t*)data;
			m_limit = m_size = size - 1;
			m_allocated = autorelease;
			m_modificationDate = currentTimeMillis();
			m_sourcePath = (const void*)NULL;
		}
	}
}

Stream::Stream(const void* data, uint32_t length, bool autorelease) {
	if ((data != NULL) && (length != 0)) {
		m_buffer = (uint8_t*)data;
		m_limit = m_size = length;
		m_allocated = autorelease;
		m_modificationDate = currentTimeMillis();
		m_sourcePath = (const void*)NULL;
	}
}

Stream::~Stream() {
	if (m_allocated) {
		memFree(m_buffer);
	}
	
	m_allocated = false;
	m_buffer = NULL;
	m_limit = m_size = 0;
	m_position = 0;
}

uint32_t Stream::readBytes(void* buffer, uint32_t length) {
	if (length == 0)
		return 0;

	uint32_t end = m_position + length;
	if (end > m_limit)
		end = m_limit;

	uint32_t size = end - m_position;
	memcpy(buffer, m_buffer + m_position, size);
	m_position = end;

	return size;
}

uint8_t* Stream::readBytes(uint32_t length) throw(const char*) {
	if (length == 0)
		return NULL;
	
	uint32_t end = m_position + length;
	if (end > m_limit)
		throw eIOReadLimit;
	
	uint32_t size = end - m_position;
	uint8_t* data = memAlloc(uint8_t, data, size);
	if (data == NULL)
		throw eOutOfMemory;
	
	memcpy(data, m_buffer + m_position, size);
	m_position = end;
	return data;
}

uint32_t Stream::readU32() throw(const char*) {
	uint32_t end = m_position + 4;
	if (end > m_limit)
		throw eIOReadLimit;

	uint32_t value = readUInt32(m_buffer, m_position);
	m_position = end;
	return value;
}

uint16_t Stream::readU16() throw(const char*) {
	uint32_t end = m_position + 2;
	if (end > m_limit)
		throw eIOReadLimit;

	uint16_t value = readUInt16(m_buffer, m_position);
	m_position = end;
	return value;
}

uint8_t Stream::readU8() throw(const char*) {
	uint32_t end = m_position + 1;
	if (end > m_limit)
		throw eIOReadLimit;
	
	uint8_t value = readUInt8(m_buffer, m_position);
	m_position = end;
	return value;
}

int32_t Stream::readS32() throw(const char*) {
	uint32_t end = m_position + 4;
	if (end > m_limit)
		throw eIOReadLimit;

	int32_t value = (int32_t)readUInt32(m_buffer, m_position);
	m_position = end;
	return value;
}

int16_t Stream::readS16() throw(const char*) {
	uint32_t end = m_position + 2;
	if (end > m_limit)
		throw eIOReadLimit;

	int16_t value = (int16_t)readUInt16(m_buffer, m_position);
	m_position = end;
	return value;
}

int8_t Stream::readS8() throw(const char*) {
	uint32_t end = m_position + 1;
	if (end > m_limit)
		throw eIOReadLimit;

	int8_t value = (int8_t)readUInt8(m_buffer, m_position);
	m_position = end;
	return value;
}

int32_t Stream::skip(uint32_t length) {
	uint32_t end = m_position + length;
	if (end > m_limit)
		return -1;

	m_position = end;
	return (int32_t)length;
}

char* Stream::readString(uint32_t length) throw(const char*) {
	if (length == 0)
		return NULL;
	
	uint32_t end = m_position + length;
	if (end > m_limit)
		throw eIOReadLimit;
	
	uint32_t size = end - m_position;
	char* data = memAlloc(char, data, size + 1);
	if (data == NULL)
		throw eOutOfMemory;
	
	memcpy(data, m_buffer + m_position, size);
	data[size] = '\0';
	m_position = end;
	return data;
}

uint8_t* Stream::readGZip(uint32_t length, uint32_t* resultSize) throw(const char*) {
	uint8_t* data = readBytes(length);
	uint32_t dataSize = length;
	uint32_t ungzipSize = *resultSize;
	uint8_t* ungzipData;
	try {
		ungzipData = (uint8_t*)gzip_decode(data, length, &ungzipSize);
	} catch (...) {
		memFree(data);
		throw;
	}
	if (ungzipData != NULL) {
		memFree(data);
		data = ungzipData;
		dataSize = ungzipSize;
	}
	*resultSize = dataSize;
	return data;
}

char* Stream::readGZipString(uint32_t length) throw(const char*) {
	uint32_t dataSize = 0;
	uint8_t* data = readGZip(length, &dataSize);
	if (data != NULL)
		data[dataSize] = '\0';
	return (char*)data;
}

Stream* Stream::duplicate() throw(const char*) {
	Stream* result = NULL;

	if (m_size > 0) {
		uint8_t* buffer = memAlloc(uint8_t, buffer, m_size + 1);
		if (buffer == NULL) {
			throw eOutOfMemory;
		}

		memcpy(buffer, getBuffer(), m_size);

		result = new Stream(buffer, m_size, true);
		result->setLimit(getLimit());
		result->setSourcePath(getSourcePath());
		result->setModificationDate(getModificationDate());
		result->setPosition(getPosition());
	}

	return result;
}
