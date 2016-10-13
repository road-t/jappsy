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

#ifndef JAPPSY_UGLTYPEFACE_H
#define JAPPSY_UGLTYPEFACE_H

#include <platform.h>

class GLTypeface {
private:
	uint8_t m_value;
	
public:
	constexpr static uint8_t DEFAULT = 8;
	constexpr static uint8_t DEFAULT_BOLD = 9;
	constexpr static uint8_t MONOSPACE = 4;
	constexpr static uint8_t SANS_SERIF = 8;
	constexpr static uint8_t SERIF = 0;
	
	constexpr static uint8_t NORMAL = 0;
	constexpr static uint8_t BOLD = 1;
	constexpr static uint8_t ITALIC = 2;
	constexpr static uint8_t BOLD_ITALIC = 3;
	
	inline GLTypeface() {
		m_value = DEFAULT;
	}
	
	inline GLTypeface(uint8_t family, uint8_t style) {
		m_value = (family & (uint8_t)0xC) | (style & (uint8_t)0x3);
	}
	
	inline GLTypeface& operator=(const uint8_t& typeface) {
		m_value = typeface;
		return *this;
	}
	
	inline GLTypeface& operator=(const GLTypeface& typeface) {
		m_value = typeface.m_value;
		return *this;
	}
	
	inline operator uint8_t() const {
		return m_value;
	}
};

#endif //JAPPSY_UGLTYPEFACE_H