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

class RefStream : public RefObject {
private:
	bool m_allocated = false;
	uint8_t* m_buffer = NULL;
	uint32_t m_size = 0;
	uint32_t m_position = 0;
	
	inline static bool equalBytes(const RefStream& a, const RefStream& b) {
		if (a.m_size != b.m_size)
			return false;
		return (memcmp(a.m_buffer, b.m_buffer, a.m_size) == 0);
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
	inline RefStream() {}
	RefStream(const wchar_t* data) throw(const char*); // eOutOfMemory
	RefStream(const char* data, bool autorelease = false);
	RefStream(const void* data, uint32_t length, bool autorelease = false);
	~RefStream();
	
	inline void* getBuffer() { return m_buffer; }
	inline uint32_t getSize() { return m_size; }
	
	uint8_t* readBytes(uint32_t length) throw(const char*); // eIOReadLimit, eOutOfMemory
	uint32_t readInt() throw(const char*); // eIOReadLimit
	uint8_t readUnsignedByte() throw(const char*); // eIOReadLimit
	
	int32_t skip(uint32_t length);
};

class Stream : public Object {
public:
	RefClass(Stream, RefStream);
	
	inline Stream(const wchar_t* data) throw(const char*) {
		RefStream* o = new RefStream(data);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}

	inline Stream(const char* data, bool autorelease) throw(const char*) {
		RefStream* o = new RefStream(data, autorelease);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	inline Stream(const void* data, uint32_t length, bool autorelease) throw(const char*) {
		RefStream* o = new RefStream(data, length, autorelease);
		if (o == NULL) throw eOutOfMemory;
		THIS.setRef(o);
	}
	
	inline void* getBuffer() throw(const char*) { return THIS.ref().getBuffer(); }
	inline uint32_t getSize() throw(const char*) { return THIS.ref().getSize(); }

	inline uint8_t* readBytes(uint32_t length) throw(const char*) { return THIS.ref().readBytes(length); }
	inline uint32_t readInt() throw(const char*) { return THIS.ref().readInt(); }
	inline uint8_t readUnsignedByte() throw(const char*) { return THIS.ref().readUnsignedByte(); }
	inline int32_t skip(uint32_t length) throw(const char*) { return THIS.ref().skip(length); }
};

#endif //JAPPSY_USTREAM_H