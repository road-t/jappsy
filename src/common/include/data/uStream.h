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

#ifndef JAPPSY_USTREAM_H
#define JAPPSY_USTREAM_H

#include <platform.h>
#include <data/uObject.h>
#include <data/uString.h>

class Stream : public CObject {
private:
	bool m_allocated = false;
	uint8_t* m_buffer = NULL;
	uint32_t m_size = 0;
	uint32_t m_limit = 0;
	uint32_t m_position = 0;
	uint64_t m_modificationDate = 0;
	
	CString m_sourcePath;
	
	inline static bool equalBytes(const Stream& a, const Stream& b) {
		return a.m_size == b.m_size && memcmp(a.m_buffer, b.m_buffer, a.m_size) == 0;
	}
	
	inline static uint32_t readUInt32(const uint8_t* buffer, uint32_t offset) {
		return
			((uint32_t)buffer[offset+3] << 24) |
			((uint32_t)buffer[offset+2] << 16) |
			((uint32_t)buffer[offset+1] << 8) |
			((uint32_t)buffer[offset+0] << 0);
	}
	
	inline static uint16_t readUInt16(const uint8_t* buffer, uint32_t offset) {
		return
			((uint16_t)buffer[offset+1] << 8) |
			((uint16_t)buffer[offset+0] << 0);
	}
	
	inline static uint8_t readUInt8(const uint8_t* buffer, uint32_t offset) {
		return buffer[offset];
	}

public:
	Stream(const wchar_t* data) throw(const char*); // eOutOfMemory
	Stream(const char* data, bool autorelease = false);
	Stream(const void* data, uint32_t length, bool autorelease = false);
	~Stream();

	inline void setLimit(uint32_t position) { m_limit = (position > m_size) ? m_size : position; }
	inline uint32_t getLimit() { return m_limit; }

	inline void* getBuffer() { return m_buffer; }
	inline uint32_t getSize() { return m_size; }
	inline uint64_t getModificationDate() { return m_modificationDate; }
	inline const CString& getSourcePath() { return m_sourcePath; }
	
	inline void setModificationDate(uint64_t date) { m_modificationDate = date; }
	inline void setSourcePath(const CString& path) { m_sourcePath = path; }
	
	inline void setPosition(uint32_t position) throw(const char*) { if (position > m_limit) throw eOutOfRange; m_position = position; }
	inline uint32_t getPosition() { return m_position; }

	uint32_t readBytes(void* buffer, uint32_t length);
	uint8_t* readBytes(uint32_t length) throw(const char*); // eIOReadLimit, eOutOfMemory
	uint32_t readU32() throw(const char*); // eIOReadLimit
	uint16_t readU16() throw(const char*); // eIOReadLimit
	uint8_t readU8() throw(const char*); // eIOReadLimit
	int32_t readS32() throw(const char*); // eIOReadLimit
	int16_t readS16() throw(const char*); // eIOReadLimit
	int8_t readS8() throw(const char*); // eIOReadLimit

	char* readString(uint32_t length) throw(const char*);
	uint8_t* readGZip(uint32_t length, uint32_t* resultSize) throw(const char*);
	char* readGZipString(uint32_t length) throw(const char*);
	
	int32_t skip(uint32_t length);

	Stream* duplicate() throw(const char*);
};

#endif //JAPPSY_USTREAM_H
