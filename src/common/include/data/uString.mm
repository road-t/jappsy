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

#include "uString.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	uint32_t utf8_strlen(const char* src, uint32_t* strsize) {
		uint8_t* ptr = (uint8_t*)src;
		uint32_t len = 0;
		uint32_t size = 0;
		uint8_t ch;
		while ((ch = *ptr) != 0) {
			ptr++; size++;
			if (ch & 0x80) { // found UTF8
				int cnt;
				if ((ch & 0xE0) == 0xC0) { // 2 byte UTF8
					cnt = 1;
				}
				else if ((ch & 0xF0) == 0xE0) { // 3 byte UTF8
					cnt = 2;
				}
				else if ((ch & 0xF8) == 0xF0) { // 4 byte UTF8
					cnt = 3;
				}
				else if ((ch & 0xFC) == 0xF8) { // 5 byte UTF8
					cnt = 4;
				}
				else if ((ch & 0xFE) == 0xFC) { // 6 byte UTF8
					cnt = 5;
				}
				else { // Not UTF8
					len++; continue;
				}
				int skip = 0;
				while (((ch = *ptr) != 0) && (cnt > 0)) {
					ptr++; skip++;
					if ((ch & 0xC0) != 0x80) // Not UTF8
						break;
					cnt--;
				}
				if (cnt > 0) { // Not UTF8
					ptr -= skip;
				}
				else {
					size += skip;
				}
			}
			len++;
		}
		if (strsize != 0) (*strsize) = size+1;
		return len;
	}
	
	uint32_t utf8_strlen_nzt(const char* src, uint32_t srcsize, uint32_t* strsize) {
		uint8_t* ptr = (uint8_t*)src;
		uint32_t len = 0;
		uint32_t size = 0;
		uint8_t ch;
		while ((srcsize > 0) && ((ch = *ptr) != 0)) {
			ptr++; size++; srcsize--;
			if (ch & 0x80) { // found UTF8
				int cnt;
				if ((ch & 0xE0) == 0xC0) { // 2 byte UTF8
					cnt = 1;
				}
				else if ((ch & 0xF0) == 0xE0) { // 3 byte UTF8
					cnt = 2;
				}
				else if ((ch & 0xF8) == 0xF0) { // 4 byte UTF8
					cnt = 3;
				}
				else if ((ch & 0xFC) == 0xF8) { // 5 byte UTF8
					cnt = 4;
				}
				else if ((ch & 0xFE) == 0xFC) { // 6 byte UTF8
					cnt = 5;
				}
				else { // Not UTF8
					len++; continue;
				}
				uint32_t skip = 0;
				while ((skip < srcsize) && ((ch = *ptr) != 0) && (cnt > 0)) {
					ptr++; skip++;
					if ((ch & 0xC0) != 0x80) // Not UTF8
						break;
					cnt--;
				}
				if (cnt > 0) { // Not UTF8
					ptr -= skip;
				}
				else {
					size += skip;
					srcsize--;
				}
			}
			len++;
		}
		if (strsize != 0) (*strsize) = size+1;
		return len;
	}
	
	uint32_t utf8_toutf16_size(const char* src) {
		char* ptr = (char*)src;
		uint32_t ressize = 2;
		uint8_t ch;
		while ((ch = *((uint8_t*)ptr)) != 0) {
			ptr++;
			if (ch & 0x80) { // found UTF8
				int cnt;
				if ((ch & 0xE0) == 0xC0) { // 2 byte UTF8
					cnt = 1;
				}
				else if ((ch & 0xF0) == 0xE0) { // 3 byte UTF8
					cnt = 2;
				}
				else if ((ch & 0xF8) == 0xF0) { // 4 byte UTF8
					cnt = 3;
				}
				else if ((ch & 0xFC) == 0xF8) { // 5 byte UTF8
					cnt = 4;
				}
				else if ((ch & 0xFE) == 0xFC) { // 6 byte UTF8
					cnt = 5;
				}
				else { // Not UTF8
					ressize += 2;
					continue;
				}
				int skip = 0;
				uint32_t utf8 = (uint32_t)(ch);
				while (((ch = *((uint8_t*)ptr)) != 0) && (cnt > 0)) {
					ptr++; skip++;
					if ((ch & 0xC0) != 0x80) // Not UTF8
						break;
					cnt--;
				}
				if (cnt > 0) { // Not UTF8
					ptr -= skip;
					ressize += 2;
				}
				else {
					int shift = (6 * skip);
					utf8 = (utf8 & (0x1F >> (skip - 1))) << shift;
					ptr -= skip;
					while (skip > 0) {
						shift -= 6;
						utf8 |= (((uint32_t)(*((uint8_t*)ptr))) & 0x3F) << shift;
						ptr++;
						skip--;
					}
					
					if (utf8 <= 0xFFFF) { // 2 byte UTF16
						ressize += 2;
					}
					else if (utf8 <= 0xFFFFF) { // 4 byte UTF16
						ressize += 4;
					}
					else {
						return (uint32_t)-1;
					}
				}
			}
			else {
				ressize += 2;
			}
		}
		return ressize;
	}
	
	uint32_t utf8_toutf16(const char* src, uint16_t* dst, uint32_t dstsize) {
		dstsize &= 0xFFFFFFFE;
		if (dstsize < 2)
			return 0;
		if (dstsize == 2) {
			*dst = 0;
			return 0;
		}
		char* ptr = (char*)src;
		uint32_t ressize = dstsize;
		uint32_t size = 0;
		uint8_t ch;
		while ((ch = *((uint8_t*)ptr)) != 0) {
			ptr++; size++;
			if (ch & 0x80) { // found UTF8
				int cnt;
				if ((ch & 0xE0) == 0xC0) { // 2 byte UTF8
					cnt = 1;
				}
				else if ((ch & 0xF0) == 0xE0) { // 3 byte UTF8
					cnt = 2;
				}
				else if ((ch & 0xF8) == 0xF0) { // 4 byte UTF8
					cnt = 3;
				}
				else if ((ch & 0xFC) == 0xF8) { // 5 byte UTF8
					cnt = 4;
				}
				else if ((ch & 0xFE) == 0xFC) { // 6 byte UTF8
					cnt = 5;
				}
				else { // Not UTF8
					*dst = ' ';
					dst++;
					dstsize -= 2;
					if (dstsize == 2) {
						break;
					}
					continue;
				}
				int skip = 0;
				uint32_t utf8 = (uint32_t)(ch);
				while (((ch = *((uint8_t*)ptr)) != 0) && (cnt > 0)) {
					ptr++; skip++;
					if ((ch & 0xC0) != 0x80) // Not UTF8
						break;
					cnt--;
				}
				if (cnt > 0) { // Not UTF8
					ptr -= skip;
					*dst = ' ';
					dst++;
					dstsize -= 2;
					if (dstsize == 2) {
						break;
					}
				}
				else {
					int shift = (6 * skip);
					utf8 = (utf8 & (0x1F >> (skip - 1))) << shift;
					ptr -= skip;
					while (skip > 0) {
						shift -= 6;
						utf8 |= (((uint32_t)(*((uint8_t*)ptr))) & 0x3F) << shift;
						ptr++;
						size++;
						skip--;
					}
					
					if (utf8 <= 0xFFFF) { // 2 byte UTF16
						if (dstsize <= 2) break;
						*dst = (uint16_t)utf8; dst++;
						dstsize -= 2;
					}
					else if (utf8 <= 0xFFFFF) { // 4 byte UTF16
						if (dstsize <= 4) break;
						*dst = (uint16_t)((utf8 >> 10) + 0xD7C0); dst++;
						*dst = (uint16_t)((utf8 & 0x3FF) + 0xDC00); dst++;
						dstsize -= 4;
					}
					else {
						return (uint16_t)-1;
					}
					
					if (dstsize == 2) {
						break;
					}
				}
			}
			else {
				*dst = (uint16_t)(ch);
				dst++;
				dstsize -= 2;
				if (dstsize == 2) {
					break;
				}
			}
		}
		*dst = 0;
		return ressize - dstsize;
	}
	
	uint32_t utf8_toutf32(const char* src, uint32_t* dst, uint32_t dstsize) {
		dstsize &= 0xFFFFFFFC;
		if (dstsize == 0)
			return 0;
		if (dstsize == 4) {
			*dst = 0;
			return 0;
		}
		char* ptr = (char*)src;
		uint32_t ressize = dstsize;
		uint32_t size = 0;
		uint8_t ch;
		while ((ch = *((uint8_t*)ptr)) != 0) {
			ptr++; size++;
			if (ch & 0x80) { // found UTF8
				int cnt;
				if ((ch & 0xE0) == 0xC0) { // 2 byte UTF8
					cnt = 1;
				}
				else if ((ch & 0xF0) == 0xE0) { // 3 byte UTF8
					cnt = 2;
				}
				else if ((ch & 0xF8) == 0xF0) { // 4 byte UTF8
					cnt = 3;
				}
				else if ((ch & 0xFC) == 0xF8) { // 5 byte UTF8
					cnt = 4;
				}
				else if ((ch & 0xFE) == 0xFC) { // 6 byte UTF8
					cnt = 5;
				}
				else { // Not UTF8
					*dst = L' ';
					dst++;
					dstsize -= 4;
					if (dstsize == 4) {
						break;
					}
					continue;
				}
				int skip = 0;
				uint32_t utf8 = (uint32_t)(ch);
				while (((ch = *((uint8_t*)ptr)) != 0) && (cnt > 0)) {
					ptr++; skip++;
					if ((ch & 0xC0) != 0x80) // Not UTF8
						break;
					cnt--;
				}
				if (cnt > 0) { // Not UTF8
					ptr -= skip;
					*dst = L' ';
					dst++;
					dstsize -= 4;
					if (dstsize == 4) {
						break;
					}
				}
				else {
					int shift = (6 * skip);
					utf8 = (utf8 & (0x1F >> (skip - 1))) << shift;
					ptr -= skip;
					while (skip > 0) {
						shift -= 6;
						utf8 |= (((uint32_t)(*((uint8_t*)ptr))) & 0x3F) << shift;
						ptr++;
						size++;
						skip--;
					}
					*dst = utf8;
					dst++;
					dstsize -= 4;
					if (dstsize == 4) {
						break;
					}
				}
			}
			else {
				*dst = (uint32_t)(ch);
				dst++;
				dstsize -= 4;
				if (dstsize == 4) {
					break;
				}
			}
		}
		*dst = 0;
		return ressize - dstsize;
	}
	
	uint32_t utf16_strlen(const uint16_t* src, uint32_t* strsize) {
		uint16_t* ptr = (uint16_t*)src;
		uint32_t len = 0;
		uint32_t size = 0;
		uint16_t ch;
		while ((ch = *ptr) != 0) {
			ptr++; size += 2;
			
			if ((ch & 0xFC00) == 0xD800) { // found UTF16
				ch = *ptr;
				if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
					ptr++;
					size += 2;
				}
				else { // Not UTF16
				}
			}
			len++;
		}
		if (strsize != NULL) (*strsize) = size+2;
		return len;
	}
	
	uint32_t utf16_toutf8_size(const uint16_t* src) {
		uint16_t* ptr = (uint16_t*)src;
		uint32_t ressize = 1;
		uint16_t ch;
		while ((ch = *ptr) != 0) {
			uint32_t utf16 = (uint32_t)(ch);
			ptr++;
			if ((ch & 0xFC00) == 0xD800) { // found UTF16
				ch = *ptr;
				if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
					ptr++;
					utf16 = (utf16 << 10) + (uint32_t)ch - 0x35FDC00;
				}
				else { // Not UTF16
					utf16 = L' ';
				}
			}
			
			if (utf16 <= 0x7F) { // 1 byte UTF8
				ressize++;
			}
			else if (utf16 <= 0x7FF) { // 2 byte UTF8
				ressize += 2;
			}
			else if (utf16 <= 0xFFFF) { // 3 byte UTF8
				ressize += 3;
			}
			else if (utf16 <= 0x1FFFFF) { // 4 byte UTF8
				ressize += 4;
			}
			else if (utf16 <= 0x3FFFFFF) { // 5 byte UTF8
				ressize += 5;
			}
			else if (utf16 <= 0x7FFFFFFF) { // 6 byte UTF8
				ressize += 6;
			}
			else {
				return (uint32_t)-1;
			}
		}
		return ressize;
	}
	
	uint32_t utf16_toutf8(const uint16_t* src, char* dst, uint32_t dstsize) {
		if (dstsize == 0)
			return 0;
		if (dstsize == 1) {
			*dst = 0;
			return 0;
		}
		uint16_t* ptr = (uint16_t*)src;
		uint32_t ressize = dstsize;
		uint32_t size = 0;
		uint16_t ch;
		while ((ch = *ptr) != 0) {
			uint32_t utf16 = (uint32_t)(ch);
			ptr++; size++;
			if ((ch & 0xFC00) == 0xD800) { // found UTF16
				ch = *ptr;
				if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
					ptr++;
					utf16 = (utf16 << 10) + (uint32_t)ch - 0x35FDC00;
				}
				else { // Not UTF16
					utf16 = L' ';
				}
			}
			
			if (utf16 <= 0x7F) { // 1 byte UTF8
				if (dstsize <= 1) break;
				*(uint8_t*)dst = (uint8_t)utf16; dst++;
				dstsize--;
			}
			else if (utf16 <= 0x7FF) { // 2 byte UTF8
				if (dstsize <= 2) break;
				*(uint8_t*)dst = (uint8_t)(0xC0 | (utf16 >> 6)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | (utf16 & 0x3F)); dst++;
				dstsize -= 2;
			}
			else if (utf16 <= 0xFFFF) { // 3 byte UTF8
				if (dstsize <= 3) break;
				*(uint8_t*)dst = (uint8_t)(0xE0 | (utf16 >> 12)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((utf16 >> 6) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | (utf16 & 0x3F)); dst++;
				dstsize -= 3;
			}
			else if (utf16 <= 0x1FFFFF) { // 4 byte UTF8
				if (dstsize <= 4) break;
				*(uint8_t*)dst = (uint8_t)(0xF0 | (utf16 >> 18)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((utf16 >> 12) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((utf16 >> 6) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | (utf16 & 0x3F)); dst++;
				dstsize -= 4;
			}
			else if (utf16 <= 0x3FFFFFF) { // 5 byte UTF8
				if (dstsize <= 5) break;
				*(uint8_t*)dst = (uint8_t)(0xF8 | (utf16 >> 24)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((utf16 >> 18) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((utf16 >> 12) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((utf16 >> 6) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | (utf16 & 0x3F)); dst++;
				dstsize -= 5;
			}
			else if (utf16 <= 0x7FFFFFFF) { // 6 byte UTF8
				if (dstsize <= 6) break;
				*(uint8_t*)dst = (uint8_t)(0xFC | (utf16 >> 30)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((utf16 >> 24) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((utf16 >> 18) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((utf16 >> 12) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((utf16 >> 6) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | (utf16 & 0x3F)); dst++;
				dstsize -= 6;
			}
			else {
				return (uint32_t)-1;
			}
		}
		*dst = 0;
		return ressize - dstsize;
	}
	
	uint32_t utf16_toutf32(const uint16_t* src, uint32_t* dst, uint32_t dstsize) {
		dstsize &= 0xFFFFFFFC;
		if (dstsize == 0)
			return 0;
		if (dstsize == 4) {
			*dst = 0;
			return 0;
		}
		uint16_t* ptr = (uint16_t*)src;
		uint32_t ressize = dstsize;
		uint32_t size = 0;
		uint16_t ch;
		while ((ch = *ptr) != 0) {
			ptr++; size++;
			if ((ch & 0xFC00) == 0xD800) { // found UTF16
				uint32_t utf16 = (uint32_t)(ch);
				ch = *ptr;
				if ((ch & 0xFC00) == 0xDC00) { // 4 byte UTF16
					ptr++;
					utf16 = (utf16 << 10) + (uint32_t)ch - 0x35FDC00;
					*dst = utf16;
				}
				else { // Not UTF16
					*dst = L' ';
				}
				dst++;
				dstsize -= 4;
				if (dstsize == 4) {
					break;
				}
			}
			else {
				*dst = (uint32_t)(ch);
				dst++;
				dstsize -= 4;
				if (dstsize == 4) {
					break;
				}
			}
		}
		*dst = 0;
		return ressize - dstsize;
	}
	
	uint32_t utf32_strlen(const uint32_t* src, uint32_t* strsize) {
		uint32_t* ptr = (uint32_t*)src;
		uint32_t len = 0;
		uint32_t size = 0;
		uint32_t ch;
		while ((ch = *ptr) != 0) {
			ptr++; size += 4;
			len++;
		}
		if (strsize != NULL) (*strsize) = size + 4;
		return len;
	}
	
	uint32_t utf32_toutf8_size(const uint32_t* src) {
		uint32_t* ptr = (uint32_t*)src;
		uint32_t ch;
		uint32_t ressize = 1;
		while ((ch = (uint32_t)(*ptr)) != 0) {
			ptr++;
			if (ch <= 0x7F) { // 1 byte UTF8
				ressize++;
			}
			else if (ch <= 0x7FF) { // 2 byte UTF8
				ressize += 2;
			}
			else if (ch <= 0xFFFF) { // 3 byte UTF8
				ressize += 3;
			}
			else if (ch <= 0x1FFFFF) { // 4 byte UTF8
				ressize += 4;
			}
			else if (ch <= 0x3FFFFFF) { // 5 byte UTF8
				ressize += 5;
			}
			else if (ch <= 0x7FFFFFFF) { // 6 byte UTF8
				ressize += 6;
			}
			else {
				return (uint32_t)-1;
			}
		}
		return ressize;
	}
	
	uint32_t utf32_toutf8(const uint32_t* src, char* dst, uint32_t dstsize) {
		if (dstsize == 0)
			return 0;
		if (dstsize == 1) {
			*dst = 0;
			return 0;
		}
		uint32_t* ptr = (uint32_t*)src;
		uint32_t ch;
		uint32_t ressize = dstsize;
		while ((ch = (uint32_t)(*ptr)) != 0) {
			ptr++;
			if (ch <= 0x7F) { // 1 byte UTF8
				if (dstsize <= 1) break;
				*(uint8_t*)dst = (uint8_t)ch; dst++;
				dstsize--;
			}
			else if (ch <= 0x7FF) { // 2 byte UTF8
				if (dstsize <= 2) break;
				*(uint8_t*)dst = (uint8_t)(0xC0 | (ch >> 6)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | (ch & 0x3F)); dst++;
				dstsize -= 2;
			}
			else if (ch <= 0xFFFF) { // 3 byte UTF8
				if (dstsize <= 3) break;
				*(uint8_t*)dst = (uint8_t)(0xE0 | (ch >> 12)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((ch >> 6) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | (ch & 0x3F)); dst++;
				dstsize -= 3;
			}
			else if (ch <= 0x1FFFFF) { // 4 byte UTF8
				if (dstsize <= 4) break;
				*(uint8_t*)dst = (uint8_t)(0xF0 | (ch >> 18)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((ch >> 12) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((ch >> 6) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | (ch & 0x3F)); dst++;
				dstsize -= 4;
			}
			else if (ch <= 0x3FFFFFF) { // 5 byte UTF8
				if (dstsize <= 5) break;
				*(uint8_t*)dst = (uint8_t)(0xF8 | (ch >> 24)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((ch >> 18) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((ch >> 12) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((ch >> 6) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | (ch & 0x3F)); dst++;
				dstsize -= 5;
			}
			else if (ch <= 0x7FFFFFFF) { // 6 byte UTF8
				if (dstsize <= 6) break;
				*(uint8_t*)dst = (uint8_t)(0xFC | (ch >> 30)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((ch >> 24) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((ch >> 18) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((ch >> 12) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | ((ch >> 6) & 0x3F)); dst++;
				*(uint8_t*)dst = (uint8_t)(0x80 | (ch & 0x3F)); dst++;
				dstsize -= 6;
			}
			else {
				return (uint8_t)-1;
			}
		}
		*dst = 0;
		return ressize - dstsize;
	}
	
	uint32_t utf32_toutf16_size(const uint32_t* src) {
		uint32_t* ptr = (uint32_t*)src;
		uint32_t ch;
		uint32_t ressize = 2;
		while ((ch = (uint32_t)(*ptr)) != 0) {
			ptr++;
			if (ch <= 0xFFFF) { // 2 byte UTF16
				ressize += 2;
			}
			else if (ch <= 0xFFFFF) { // 4 byte UTF16
				ressize += 4;
			}
			else {
				return (uint32_t)-1;
			}
		}
		return ressize;
	}
	
	uint32_t utf32_toutf16(const uint32_t* src, uint16_t* dst, uint32_t dstsize) {
		dstsize &= 0xFFFFFFFE;
		if (dstsize == 0)
			return 0;
		if (dstsize == 2) {
			*dst = 0;
			return 0;
		}
		uint32_t* ptr = (uint32_t*)src;
		uint32_t ch;
		uint32_t ressize = dstsize;
		while ((ch = (uint32_t)(*ptr)) != 0) {
			ptr++;
			if (ch <= 0xFFFF) { // 2 byte UTF16
				if (dstsize <= 2) break;
				*dst = (uint16_t)ch; dst++;
				dstsize -= 2;
			}
			else if (ch <= 0xFFFFF) { // 4 byte UTF16
				if (dstsize <= 4) break;
				*dst = (uint16_t)((ch >> 10) + 0xD7C0); dst++;
				*dst = (uint16_t)((ch & 0x3FF) + 0xDC00); dst++;
				dstsize -= 4;
			}
			else {
				return (uint32_t)-1;
			}
		}
		*dst = 0;
		return ressize - dstsize;
	}
	
	//==============================================================
	
#ifdef STRING_WCSLWR
	wchar_t* wcslwr(wchar_t* s) {
		wchar_t* p = s;
		while ((*p = towlower(*p)) != 0) p++;
		return s;
	}
	
	wchar_t* wcsupr(wchar_t* s) {
		wchar_t* p = s;
		while ((*p = towupper(*p)) != 0) p++;
		return s;
	}
#endif
	
	int utf8_cmpi(const char* ptr1, const char* ptr2) {
		if (ptr1 == NULL)
			return (ptr2 == NULL) ? 0 : -1;
		else if (ptr2 == NULL)
			return 1;
		
		uint32_t len1 = (uint32_t)utf8_strlen(ptr1, NULL);
		uint32_t len2 = (uint32_t)utf8_strlen(ptr2, NULL);
		uint32_t len = (len1 < len2) ? len1 : len2;
		if (len1 == 0)
			return (len2 == 0) ? 0 : -1;
		else if (len2 == 0)
			return 1;
		
		while (len-- > 0) {
			char ch1 = *ptr1; ptr1++;
			char ch2 = *ptr2; ptr2++;
			uint32_t v1;
			uint32_t v2;
			if (ch1 & 0x80) {
				if (ch2 & 0x80) {
					do {
						int cnt;
						if ((ch1 & 0xE0) == 0xC0) cnt = 1;
						else if ((ch1 & 0xF0) == 0xE0) cnt = 2;
						else if ((ch1 & 0xF8) == 0xF0) cnt = 3;
						else if ((ch1 & 0xFC) == 0xF8) cnt = 4;
						else if ((ch1 & 0xFE) == 0xFC) cnt = 6;
						else {
							v1 = ch1;
							break;
						}
						
						int skip = 0;
						uint32_t utf8 = (uint32_t)ch1;
						while (((ch1 = *ptr1) != '\0') && (cnt > 0)) {
							ptr1++; skip++;
							if ((ch1 & 0xC0) != 0x80)
								break;
							cnt--;
						}
						if (cnt > 0) {
							ptr1 -= skip;
							break;
						} else {
							int shift = (6 * skip);
							utf8 = (utf8 & (0x1F >> (skip - 1))) << shift;
							ptr1 -= skip;
							while (skip > 0) {
								shift -= 6;
								utf8 |= ((uint32_t)(*ptr1) & 0x3F) << shift;
								ptr1++;
								skip--;
							}
							
							#if __WCHAR_MAX__ > 0x10000
								v1 = towlower((wchar_t)utf8);
							#else
								if (utf8 <= 0xFFFF) {
									v1 = towlower((wchar_t)utf8);
								} else {
									v1 = utf8;
								}
							#endif
						}
					} while (false);
							
					do {
						int cnt;
						if ((ch2 & 0xE0) == 0xC0) cnt = 1;
						else if ((ch2 & 0xF0) == 0xE0) cnt = 2;
						else if ((ch2 & 0xF8) == 0xF0) cnt = 3;
						else if ((ch2 & 0xFC) == 0xF8) cnt = 4;
						else if ((ch2 & 0xFE) == 0xFC) cnt = 6;
						else {
							v2 = ch2;
							break;
						}
						
						int skip = 0;
						uint32_t utf8 = (uint32_t)ch2;
						while (((ch2 = *ptr2) != '\0') && (cnt > 0)) {
							ptr2++; skip++;
							if ((ch2 & 0xC0) != 0x80)
								break;
							cnt--;
						}
						if (cnt > 0) {
							ptr2 -= skip;
							break;
						} else {
							int shift = (6 * skip);
							utf8 = (utf8 & (0x1F >> (skip - 1))) << shift;
							ptr2 -= skip;
							while (skip > 0) {
								shift -= 6;
								utf8 |= ((uint32_t)(*ptr2) & 0x3F) << shift;
								ptr2++;
								skip--;
							}
							
							#if __WCHAR_MAX__ > 0x10000
								v2 = towlower((wchar_t)utf8);
							#else
								if (utf8 <= 0xFFFF) {
									v2 = towlower((wchar_t)utf8);
								} else {
									v2 = utf8;
								}
							#endif
						}
					} while (false);
				} else {
					return 1;
				}
			} else if (ch2 & 0x80) {
				return -1;
			} else {
				v1 = towlower((wchar_t)ch1);
				v2 = towlower((wchar_t)ch2);
			}
			
			if (v1 != v2) return (v1 < v2) ? -1 : 1;
		}
		if (len1 == len2)
			return 0;
		else
			return (len1 < len2) ? -1 : 1;
	}
	
	int wcs_cmpi(const wchar_t* ptr1, const wchar_t* ptr2) {
		if (ptr1 == NULL)
			return (ptr2 == NULL) ? 0 : -1;
		else if (ptr2 == NULL)
			return 1;
		
		uint32_t len1 = (uint32_t)wcs_strlen(ptr1, NULL);
		uint32_t len2 = (uint32_t)wcs_strlen(ptr2, NULL);
		uint32_t len = (len1 < len2) ? len1 : len2;
		if (len1 == 0)
			return (len2 == 0) ? 0 : -1;
		else if (len2 == 0)
			return 1;
		
		while (len-- > 0) {
			wchar_t v1 = towlower(*ptr1); ptr1++;
			wchar_t v2 = towlower(*ptr2); ptr2++;
			if (v1 != v2) return (v1 < v2) ? -1 : 1;
		}
		if (len1 == len2)
			return 0;
		else
			return (len1 < len2) ? -1 : 1;
	}
	
#ifdef __cplusplus
}
#endif

//==============================================================

#include <core/uSystem.h>

#define StringTrue L"true"
#define StringFalse L"false"
#define StringNan L"NaN"
#define StringInfinite L"Infinity"
#define StringNegInfinite L"-Infinity"
#define StringEmpty L""
#define StringNil NULL
#define StringNull L"null"
#define StringUndefined L"undefined"

#define STRING_BLOCK_SIZE	32

void CString::setSize(uint32_t size) throw(const char*) {
	uint32_t newSize = size;
	uint32_t newLength = (newSize / sizeof(wchar_t)) - 1;
	
	if (newSize < sizeof(wchar_t)) {
		if (this->m_data != NULL) {
			memFree(this->m_data);
			this->m_data = NULL;
		}
		this->m_length = 0;
		this->m_size = 0;
		this->m_memorySize = 0;
		return;
	}
	
	uint32_t newMemSize = newSize - (newSize % STRING_BLOCK_SIZE) + STRING_BLOCK_SIZE;
	if (this->m_memorySize != newMemSize) {
		wchar_t* newString = memRealloc(wchar_t, newString, this->m_data, newMemSize);
		if (newString) {
			this->m_data = newString;
			this->m_memorySize = newMemSize;
		} else {
			throw eOutOfMemory;
		}
	}
	
	this->m_data[newLength] = 0;
	this->m_length = newLength;
	this->m_size = newSize;
}

void CString::setLength(uint32_t length) throw(const char*) {
	uint32_t newSize = (length + 1) * sizeof(wchar_t);
	uint32_t newMemSize = newSize - (newSize % STRING_BLOCK_SIZE) + STRING_BLOCK_SIZE;
	if (this->m_memorySize != newMemSize) {
		wchar_t* newString = memRealloc(wchar_t, newString, this->m_data, newMemSize);
		if (newString) {
			this->m_data = newString;
			this->m_memorySize = newMemSize;
		} else {
			throw eOutOfMemory;
		}
	}
	
	this->m_data[length] = 0;
	this->m_length = length;
	this->m_size = newSize;
}

//==============================================================

void CString::release() {
	if (this->m_data != NULL) {
		memFree(this->m_data);
		this->m_data = NULL;
	}
	this->m_length = 0;
	this->m_size = 0;
	this->m_memorySize = 0;
}

//==============================================================

CString::CString(const void* string) throw(const char*) {
	if (string != NULL)
		throw eInvalidPointer;
}

CString& CString::operator =(const void* string) throw(const char*) {
	if (string != NULL) {
		throw eInvalidPointer;
	}
	this->setSize(0);
	return *this;
}

CString::CString(const CString& string) throw(const char*) {
	if (string.m_size > 0) {
		this->setSize(string.m_size);
		memcpy(this->m_data, string.m_data, this->m_size);
	}
}

CString& CString::operator =(const CString& string) throw(const char*) {
	this->setSize(string.m_size);
	if (string.m_size > 0)
		memcpy(this->m_data, string.m_data, this->m_size);
	return *this;
}

CString& CString::concat(const CString& string) throw(const char*) {
	uint32_t length = string.m_length;
	if (length != 0) {
		uint32_t prevLength = this->m_length;
		setLength(prevLength + length);
		memcpy(this->m_data + prevLength, string.m_data, string.m_size);
	}
	return *this;
}

#if defined(__IOS__)

CString::CString(const NSString* string) throw(const char*) {
	if (string != NULL) {
		uint32_t length = (uint32_t)string.length;
		if (length > 0) {
			this->setLength(length);
			uint32_t newSize = utf8_towcs(string.UTF8String, this->m_data, this->m_size);
			this->setSize(newSize);
		} else {
			this->setLength(0);
		}
	}
}

CString& CString::operator =(const NSString* string) throw(const char*) {
	if (string != NULL) {
		uint32_t length = (uint32_t)string.length;
		if (length > 0) {
			this->setLength(length);
			uint32_t newSize = utf8_towcs(string.UTF8String, this->m_data, this->m_size);
			this->setSize(newSize);
		} else {
			this->setLength(0);
		}
	} else {
		this->setSize(0);
	}
	return *this;
}

CString& CString::concat(const NSString* string) throw(const char*) {
	if (string != NULL) {
		uint32_t prevLength = this->m_length;
		uint32_t length = (uint32_t)string.length;
		if (length > 0) {
			this->setLength(prevLength + length);
			uint32_t newSize = utf8_towcs(string.UTF8String, this->m_data + prevLength, (length+1) * sizeof(wchar_t));
			this->setSize(newSize);
		}
	}
	return *this;
}

#endif

CString::CString(const char* string) throw(const char*) {
	if (string != NULL) {
		uint32_t length = utf8_strlen(string, NULL);
		if (length > 0) {
			this->setLength(length);
			utf8_towcs(string, this->m_data, this->m_size);
		} else {
			this->setLength(0);
		}
	}
}

CString& CString::operator =(const char* string) throw(const char*) {
	if (string != NULL) {
		uint32_t length = utf8_strlen(string, NULL);
		if (length > 0) {
			this->setLength(length);
			utf8_towcs(string, this->m_data, this->m_size);
		} else {
			this->setLength(0);
		}
	} else {
		this->setSize(0);
	}
	return *this;
}

CString& CString::concat(const char* string) throw(const char*) {
	if (string != NULL) {
		uint32_t prevLength = this->m_length;
		uint32_t strSize;
		uint32_t length = utf8_strlen(string, &strSize);
		if (length > 0) {
			this->setLength(prevLength + length);
			utf8_towcs(string, this->m_data + prevLength, (length+1) * sizeof(wchar_t));
		}
	}
	return *this;
}

CString::CString(const char* string, uint32_t size) throw(const char*) {
	if (string != NULL) {
		uint32_t strSize = 0;
		uint32_t length = utf8_strlen_nzt(string, size, &strSize);
		this->setLength(length);
		if (length > 0) {
			utf8_towcs(string, this->m_data, this->m_size);
		}
	}
}

CString::CString(const wchar_t* string) throw(const char*) {
	if (string != NULL) {
		uint32_t newSize = 0;
		uint32_t length = wcs_strlen(string, &newSize);
		if (length > 0) {
			this->setSize(newSize);
			memcpy(this->m_data, string, this->m_size);
		} else {
			this->setLength(0);
		}
	}
}

CString& CString::operator =(const wchar_t* string) throw(const char*) {
	if (string != NULL) {
		uint32_t newSize = 0;
		uint32_t length = wcs_strlen(string, &newSize);
		if (length > 0) {
			this->setSize(newSize);
			memcpy(this->m_data, string, this->m_size);
		} else {
			this->setLength(0);
		}
	} else {
		this->setSize(0);
	}
	return *this;
}

CString& CString::concat(const wchar_t* string) throw(const char*) {
	if (string != NULL) {
		uint32_t prevLength = this->m_length;
		uint32_t strSize;
		uint32_t length = wcs_strlen(string, &strSize);
		if (length > 0) {
			this->setLength(this->m_length + length);
			memcpy(this->m_data + prevLength, string, strSize);
		}
	}
	return *this;
}

CString::CString(const wchar_t* string, uint32_t length) throw(const char*) {
	if (string != NULL) {
		this->setLength(length);
		if (length > 0)
			memcpy(this->m_data, string, length * sizeof(wchar_t));
	}
}

CString::CString(const char character) throw(const char*) {
	this->setLength(1);
	this->m_data[0] = (wchar_t)character;
}

CString& CString::operator =(const char character) throw(const char*) {
	this->setLength(1);
	this->m_data[0] = (wchar_t)character;
	return *this;
}

CString& CString::concat(const char character) throw(const char*) {
	uint32_t prevLength = this->m_length;
	this->setLength(prevLength + 1);
	this->m_data[prevLength] = (wchar_t)character;
	return *this;
}

CString::CString(const wchar_t character) throw(const char*) {
	this->setLength(1);
	this->m_data[0] = character;
}

CString& CString::operator =(const wchar_t character) throw(const char*) {
	this->setLength(1);
	this->m_data[0] = character;
	return *this;
}

CString& CString::concat(const wchar_t character) throw(const char*) {
	uint32_t prevLength = this->m_length;
	this->setLength(prevLength + 1);
	this->m_data[prevLength] = character;
	return *this;
}

CString::CString(bool value) throw(const char*) {
	this->operator =(value);
}

CString& CString::operator =(bool value) throw(const char*) {
	if (value)
		return this->operator =(StringTrue);
	else
		return this->operator =(StringFalse);
}

CString& CString::concat(const bool value) throw(const char*) {
	if (value)
		return this->concat(StringTrue);
	else
		return this->concat(StringFalse);
}

static inline int swprintf(wchar_t* target, int8_t value) {
	return ::swprintf(target, 4, L"%hhd", value);
}

CString::CString(int8_t value) throw(const char*) {
	this->setLength(4);
	int length = swprintf(this->m_data, value);
	if (length != 4) {
		this->setLength(static_cast<uint32_t>(length));
	}
}

CString& CString::operator =(int8_t value) throw(const char*) {
	this->setLength(4);
	int length = swprintf(this->m_data, value);
	if (length != 4) {
		this->setLength(static_cast<uint32_t>(length));
	}
	return *this;
}

CString& CString::concat(const int8_t value) throw(const char*) {
	uint32_t prevLength = this->m_length;
	this->setLength(prevLength + 4);
	int length = swprintf(this->m_data + prevLength, value);
	if (length != 4) {
		this->setLength(prevLength + static_cast<uint32_t>(length));
	}
	return *this;
}

static inline int swprintf(wchar_t* target, uint8_t value) {
	return ::swprintf(target, 3, L"%hhu", value);
}

CString::CString(uint8_t value) throw(const char*) {
	this->setLength(3);
	int length = swprintf(this->m_data, value);
	if (length != 3) {
		this->setLength(static_cast<uint32_t>(length));
	}
}

CString& CString::operator =(uint8_t value) throw(const char*) {
	this->setLength(3);
	int length = swprintf(this->m_data, value);
	if (length != 3) {
		this->setLength(static_cast<uint32_t>(length));
	}
	return *this;
}

CString& CString::concat(const uint8_t value) throw(const char*) {
	uint32_t prevLength = this->m_length;
	this->setLength(prevLength + 3);
	int length = swprintf(this->m_data + prevLength, value);
	if (length != 3) {
		this->setLength(prevLength + static_cast<uint32_t>(length));
	}
	return *this;
}

static inline int swprintf(wchar_t* target, int16_t value) {
	return ::swprintf(target, 6, L"%hd", value);
}

CString::CString(int16_t value) throw(const char*) {
	this->setLength(6);
	int length = swprintf(this->m_data, value);
	if (length != 6) {
		this->setLength(static_cast<uint32_t>(length));
	}
}

CString& CString::operator =(int16_t value) throw(const char*) {
	this->setLength(6);
	int length = swprintf(this->m_data, value);
	if (length != 6) {
		this->setLength(static_cast<uint32_t>(length));
	}
	return *this;
}

CString& CString::concat(const int16_t value) throw(const char*) {
	uint32_t prevLength = this->m_length;
	this->setLength(prevLength + 6);
	int length = swprintf(this->m_data + prevLength, value);
	if (length != 6) {
		this->setLength(prevLength + static_cast<uint32_t>(length));
	}
	return *this;
}

static inline int swprintf(wchar_t* target, uint16_t value) {
	return ::swprintf(target, 5, L"%hu", value);
}

CString::CString(uint16_t value) throw(const char*) {
	this->setLength(5);
	int length = swprintf(this->m_data, value);
	if (length != 5) {
		this->setLength(static_cast<uint32_t>(length));
	}
}

CString& CString::operator =(uint16_t value) throw(const char*) {
	this->setLength(5);
	int length = swprintf(this->m_data, value);
	if (length != 5) {
		this->setLength(static_cast<uint32_t>(length));
	}
	return *this;
}

CString& CString::concat(const uint16_t value) throw(const char*) {
	uint32_t prevLength = this->m_length;
	this->setLength(prevLength + 5);
	int length = swprintf(this->m_data + prevLength, value);
	if (length != 5) {
		this->setLength(prevLength + static_cast<uint32_t>(length));
	}
	return *this;
}

int swprintf(wchar_t* target, int32_t value) {
	return ::swprintf(target, 11, L"%ld", value);
}

CString::CString(int32_t value) throw(const char*) {
	this->setLength(11);
	int length = swprintf(this->m_data, value);
	if (length != 11) {
		this->setLength(static_cast<uint32_t>(length));
	}
}

CString& CString::operator =(int32_t value) throw(const char*) {
	this->setLength(11);
	int length = swprintf(this->m_data, value);
	if (length != 11) {
		this->setLength(static_cast<uint32_t>(length));
	}
	return *this;
}

CString& CString::concat(const int32_t value) throw(const char*) {
	uint32_t prevLength = this->m_length;
	this->setLength(prevLength + 11);
	int length = swprintf(this->m_data + prevLength, value);
	if (length != 11) {
		this->setLength(prevLength + static_cast<uint32_t>(length));
	}
	return *this;
}

static inline int swprintf(wchar_t* target, uint32_t value) {
	return ::swprintf(target, 10, L"%lu", value);
}

CString::CString(uint32_t value) throw(const char*) {
	this->setLength(10);
	int length = swprintf(this->m_data, value);
	if (length != 10) {
		this->setLength(static_cast<uint32_t>(length));
	}
}

CString& CString::operator =(uint32_t value) throw(const char*) {
	this->setLength(10);
	int length = swprintf(this->m_data, value);
	if (length != 10) {
		this->setLength(static_cast<uint32_t>(length));
	}
	return *this;
}

CString& CString::concat(const uint32_t value) throw(const char*) {
	uint32_t prevLength = this->m_length;
	this->setLength(prevLength + 10);
	int length = swprintf(this->m_data + prevLength, value);
	if (length != 10) {
		this->setLength(prevLength + static_cast<uint32_t>(length));
	}
	return *this;
}

static inline int swprintf(wchar_t* target, int64_t value) {
	return ::swprintf(target, 21, L"%lld", value);
}

CString::CString(int64_t value) throw(const char*) {
	this->setLength(21);
	int length = swprintf(this->m_data, value);
	if (length != 21) {
		this->setLength(static_cast<uint32_t>(length));
	}
}

CString& CString::operator =(int64_t value) throw(const char*) {
	this->setLength(21);
	int length = swprintf(this->m_data, value);
	if (length != 21) {
		this->setLength(static_cast<uint32_t>(length));
	}
	return *this;
}

CString& CString::concat(const int64_t value) throw(const char*) {
	uint32_t prevLength = this->m_length;
	this->setLength(prevLength + 21);
	int length = swprintf(this->m_data + prevLength, value);
	if (length != 21) {
		this->setLength(prevLength + static_cast<uint32_t>(length));
	}
	return *this;
}

static inline int swprintf(wchar_t* target, uint64_t value) {
	return ::swprintf(target, 20, L"%llu", value);
}

CString::CString(uint64_t value) throw(const char*) {
	this->setLength(20);
	int length = swprintf(this->m_data, value);
	if (length != 20) {
		this->setLength(static_cast<uint32_t>(length));
	}
}

CString& CString::operator =(uint64_t value) throw(const char*) {
	this->setLength(20);
	int length = swprintf(this->m_data, value);
	if (length != 20) {
		this->setLength(static_cast<uint32_t>(length));
	}
	return *this;
}

CString& CString::concat(const uint64_t value) throw(const char*) {
	uint32_t prevLength = this->m_length;
	this->setLength(prevLength + 20);
	int length = swprintf(this->m_data + prevLength, value);
	if (length != 20) {
		this->setLength(prevLength + static_cast<uint32_t>(length));
	}
	return *this;
}

#ifndef __IOS__
#include <math.h>
#endif

static inline int swprintf(wchar_t* target, float value) {
	int length = ::swprintf(target, 23, L"%.7f", value);
	if (length == EOF)
		return ::swprintf(target, 23, L"%.7e", value);
	else {
		wchar_t* temp = target + length - 1;
		wchar_t ch;
		while ( ((ch = *temp) == L'0') || (ch == L'.') || (length > 8) ) {
			*temp = 0; temp--;
			length--;
			if (ch == L'.') break;
		}
		int len = length;
		if (ch == L'.') {
			while ( len > 7 ) {
				*temp = L'0'; temp--;
				len--;
			}
		}
	}
	return length;
}

CString::CString(float value) throw(const char*) {
	if (isnan(value)) {
		this->operator=(StringNan);
	} else if (isinf(value)) {
		if (signbit(value)) {
			this->operator=(StringNegInfinite);
		} else {
			this->operator=(StringInfinite);
		}
	} else {
		this->setLength(23);
		int length = swprintf(this->m_data, value);
		if (length == EOF) {
			this->setSize(0);
			throw eConvert;
		} else if (length != 23)
			this->setLength(static_cast<uint32_t>(length));
	}
}

CString& CString::operator =(float value) throw(const char*) {
	if (isnan(value))
		return this->operator=(StringNan);
	else if (isinf(value)) {
		if (signbit(value))
			return this->operator=(StringNegInfinite);
		else
			return this->operator=(StringInfinite);
	}
	
	this->setLength(23);
	int length = swprintf(this->m_data, value);
	if (length == EOF) {
		this->setSize(0);
		throw eConvert;
	} else if (length != 23)
		this->setLength(static_cast<uint32_t>(length));
	return *this;
}

CString& CString::concat(const float value) throw(const char*) {
	if (isnan(value))
		return this->concat(StringNan);
	else if (isinf(value)) {
		if (signbit(value))
			return this->concat(StringNegInfinite);
		else
			return this->concat(StringInfinite);
	}
	
	uint32_t prevSize = this->m_size;
	uint32_t prevLength = this->m_length;
	this->setLength(prevLength + 23);
	int length = swprintf(this->m_data + prevLength, value);
	if (length == EOF) {
		this->setSize(prevSize);
		throw eConvert;
	} else if (length != 23)
		this->setLength(prevLength + static_cast<uint32_t>(length));
	return *this;
}

static inline int swprintf(wchar_t* target, double value) {
	int length = ::swprintf(target, 31, L"%.15f", value);
	if (length == EOF)
		return ::swprintf(target, 31, L"%.15e", value);
	else {
		wchar_t* temp = target + length - 1;
		wchar_t ch;
		while ( ((ch = *temp) == L'0') || (ch == L'.') || (length > 16) ) {
			*temp = 0; temp--;
			length--;
			if (ch == L'.') break;
		}
		int len = length;
		if (ch == L'.') {
			while ( len > 15 ) {
				*temp = L'0'; temp--;
				len--;
			}
		}
	}
	return length;
}

CString::CString(double value) throw(const char*) {
	if (isnan(value)) {
		this->operator=(StringNan);
	} else if (isinf(value)) {
		if (signbit(value)) {
			this->operator=(StringNegInfinite);
		} else {
			this->operator=(StringInfinite);
		}
	} else {
		this->setLength(31);
		int length = swprintf(this->m_data, value);
		if (length == EOF) {
			this->setSize(0);
			throw eConvert;
		} else if (length != 31)
			this->setLength(static_cast<uint32_t>(length));
	}
}

CString& CString::operator =(double value) throw(const char*) {
	if (isnan(value))
		return this->operator=(StringNan);
	else if (isinf(value)) {
		if (signbit(value))
			return this->operator=(StringNegInfinite);
		else
			return this->operator=(StringInfinite);
	}
	
	this->setLength(31);
	int length = swprintf(this->m_data, value);
	if (length == EOF) {
		this->setSize(0);
		throw eConvert;
	} else if (length != 31)
		this->setLength(static_cast<uint32_t>(length));
	return *this;
}

CString& CString::concat(const double value) throw(const char*) {
	if (isnan(value))
		return this->concat(StringNan);
	else if (isinf(value)) {
		if (signbit(value))
			return this->concat(StringNegInfinite);
		else
			return this->concat(StringInfinite);
	}
	
	uint32_t prevSize = this->m_size;
	uint32_t prevLength = this->m_length;
	this->setLength(prevLength + 31);
	int length = swprintf(this->m_data + prevLength, value);
	if (length == EOF) {
		this->setSize(prevSize);
		throw eConvert;
	} else if (length != 31)
		this->setLength(prevLength + static_cast<uint32_t>(length));
	return *this;
}

#ifdef __IOS__

CString::operator NSString*() const {
	NSString* result = NULL;
	if (this->m_data != NULL) {
#if __WCHAR_MAX__ > 0x10000
		result = [[NSString alloc] initWithBytes:(char*)(this->m_data) length:(this->m_length) * sizeof(wchar_t) encoding:NSUTF32LittleEndianStringEncoding];
#else
		result = [[NSString alloc] initWithBytes:(char*)(this->m_data) length:(this->m_length) * sizeof(wchar_t) encoding:NSUTF16LittleEndianStringEncoding];
#endif
	}
	return result;
}

#endif

//==============================================================

enum StringNumberFormat { snfNone, snfUndefined, snfNil, snfNull, snfBoolTrue, snfBoolFalse, snfHex, snfOct, snfBit, snfFloat, snfFloatEx, snfFloatNan, snfFloatInfPos, snfFloatInfNeg, snfInt, snfUInt };

static int getStringNumberFormat(wchar_t* src, uint32_t srcLen, uint32_t* pos, uint32_t* len) {
	if (srcLen > 0) {
		wchar_t* lpStr = src;
		wchar_t ch = lpStr[0];
		bool isHex = false;
		bool isOct = false;
		bool isBit = false;
		bool isNeg = false;
		
		if (pos) *pos = 0;
		if (len) *len = 0;
		if ((ch == L'T') || (ch == L't')) { // возможно bool
			if ((wcscmp(src, L"true") == 0) || (wcscmp(src, L"TRUE") == 0)) {
				if (len) *len = 4;
				return snfBoolTrue;
			}
			return snfNone;
		} else if (ch == L'b') { // возможно bit number или hex
			isBit = true; isHex = true;
		} else if ((ch == L'F') || (ch == L'f')) { // возможно bool
			if ((wcscmp(src, L"false") == 0) || (wcscmp(src, L"FALSE") == 0)) {
				if (len) *len = 5;
				return snfBoolFalse;
			}
			isHex = true;
		} else if (ch == L'u') { // возможно undefined
			if (wcscmp(src, L"undefined") == 0) {
				if (len) *len = 9;
				return snfUndefined;
			}
			return snfNone;
		} else if (ch == L'N') { // возможно NaN NULL
			if (wcscmp(src, L"NaN") == 0) {
				if (len) *len = 3;
				return snfFloatNan;
			} else if (wcscmp(src, L"NULL") == 0) {
				if (len) *len = 4;
				return snfNull;
			}
			return snfNone;
		} else if (ch == L'n') { // возможно nil null
			if (wcscmp(src, L"nil") == 0) {
				if (len) *len = 3;
				return snfNil;
			} else if (wcscmp(src, L"null") == 0) {
				if (len) *len = 4;
				return snfNull;
			}
			return snfNone;
		} else if (ch == L'I') { // Возможно INF Inf Infinity
			if ((wcscmp(src, L"INF") == 0) || (wcscmp(src, L"Inf") == 0)) {
				if (len) *len = 3;
				return snfFloatInfPos;
			} else if (wcscmp(src, L"Infinity") == 0) {
				if (len) *len = 8;
				return snfFloatInfPos;
			}
			return snfNone;
		} else if (ch == L'0') { // возможно oct number или hex
			if (lpStr[1] == L'x') {
				if (pos) *pos = 2;
				isHex = true;
				lpStr++;
			} else {
				isOct = true;
				while (*lpStr == L'0') {
					lpStr++;
					if (pos) (*pos)++;
				}
				lpStr--;
			}
		} else if (ch == L'$') {
			if (pos) *pos = 1;
			isHex = true;
		} else if ( ((ch >= L'A') && (ch <= L'F')) || ((ch >= L'a') && (ch <= L'f')) ) {
			isHex = true;
		} else if (ch == L'-') {
			if ((wcscmp(src, L"-INF") == 0) || (wcscmp(src, L"-Inf") == 0)) {
				if (len) *len = 4;
				return snfFloatInfNeg;
			} else if (wcscmp(src, L"-Infinity") == 0) {
				if (len) *len = 9;
				return snfFloatInfNeg;
			}
			isNeg = true;
		} else if ((ch == '.') || (ch == ',')) {
			lpStr--;
		} else if (!((ch >= L'0') && (ch <= L'9'))) { // начальный символ не соответствует числовому значению
			return snfNone;
		}
		
		bool isFloat = false;
		bool isFloatExponent = false;
		lpStr++;
		while ((ch = *lpStr) != 0) {
			if ((ch == L'.') || (ch == L',')) { // float?
				if ((!isHex) && (!isBit)) {
					if (!isFloat)
						isFloat = true;
					else // не может быть 2 раза точка в числе
						return snfNone;
				} else // не может быть hex или bit с точкой
					return snfNone;
			} else if ( ((ch >= L'A') && (ch <= L'F')) || ((ch >= L'a') && (ch <= L'f')) ) { // hex?
				if (!isFloat) {
					if (!isNeg) {
						if (isBit) isBit = false;
						if (isOct) isOct = false;
						if (!isHex) isHex = true;
					} else // не может быть hex со знаком минус
						return snfNone;
				} else if (((ch == L'e') || (ch == L'E')) && (!isFloatExponent)) {
					lpStr++;
					if ((ch = *lpStr) == 0) // непредвиденный конец
						return snfNone;
					if ((ch != L'+') && (ch != L'-')) // экспонета должна начинатся со знака + или -
						return snfNone;
					lpStr++;
					if ((ch = *lpStr) == 0) // непредвиденный конец
						return snfNone;
					if ((ch < L'0') || (ch > L'9')) // после знака должна быть цифра
						return snfNone;
					isFloatExponent = true;
				} else // не может быть float с буквами
					return snfNone;
			} else if ((ch >= L'2') && (ch <= L'9')) {
				if (isBit) isBit = false;
				if ((ch >= L'8') && (ch <= L'9')) {
					if (isOct) isOct = false;
				}
			} else if (ch == L'h') {
				if (len) (*len)--;
				if ((!isHex) || (*(lpStr+1) != 0)) // только hex может заканчиваться на h
					return snfNone;
			} else if (!((ch >= L'0') && (ch <= L'9'))) { // не может число содержать недопустимые символы
				return snfNone;
			}
			lpStr++;
		}
		if (srcLen == 1) {
			if (isOct) isOct = false;
		}
		if (isBit) {
			if (pos) (*pos)++;
		}
		if ((len) && (pos)) *len = srcLen-(*pos)+(*len);
		if (isBit) return snfBit;
		if (isHex) return snfHex;
		if (isFloat) {
			wchar_t* point = wcsstr(src,L",");
			if (point) *point = L'.';
			if (isFloatExponent)
				return snfFloatEx;
			else
				return snfFloat;
		}
		if (isOct) return snfOct;
		return (isNeg ? snfInt : snfUInt);
	}
	return snfNone;
}

static uint64_t wtoull(const wchar_t* data, uint32_t len, uint32_t type) {
	switch (type) {
		case snfNone:
		case snfUndefined:
		case snfNil:
		case snfNull:
		case snfBoolFalse:
		case snfFloatNan:
			return 0;
			
		case snfBoolTrue:
			return -1;
			
		case snfFloatInfPos:
		case snfFloatInfNeg:
			return 0;
			
		case snfHex:
			return wcstoull(data, NULL, 16);
			
		case snfOct:
			return wcstoull(data, NULL, 8);
			
		case snfBit:
			return wcstoull(data, NULL, 2);
			
		case snfFloat:
		case snfFloatEx: {
			long double value = wcstold(data, NULL);
			return (uint64_t)((int64_t)value);
		}
			
		case snfInt:
			return (uint64_t)(wcstoll(data, NULL, 10));
			
		case snfUInt:
			return wcstoull(data, NULL, 10);
	}
	
	return 0;
}

int64_t wtoll(const wchar_t* data, uint32_t len, uint32_t type) {
	switch (type) {
		case snfNone:
		case snfUndefined:
		case snfNil:
		case snfNull:
		case snfBoolFalse:
		case snfFloatNan:
			return 0;
			
		case snfBoolTrue:
			return -1;
			
		case snfFloatInfPos:
		case snfFloatInfNeg:
			return 0;
			
		case snfHex:
			return (int64_t)(wcstoull(data, NULL, 16));
			
		case snfOct:
			return (int64_t)(wcstoull(data, NULL, 8));
			
		case snfBit:
			return (int64_t)(wcstoull(data, NULL, 2));
			
		case snfFloat:
		case snfFloatEx: {
			long double value = wcstold(data, NULL);
			return (int64_t)value;
		}
			
		case snfInt:
			return wcstoll(data, NULL, 10);
			
		case snfUInt:
			return (int64_t)(wcstoull(data, NULL, 10));
	}
	
	return 0;
}

long double wtod(const wchar_t* data, uint32_t len, uint32_t type) {
	switch (type) {
		case snfNone:
		case snfUndefined:
		case snfNil:
		case snfNull:
		case snfBoolFalse:
			return 0;
			
		case snfBoolTrue:
			return -1;
			
		case snfFloatNan:
			return NAN;
			
		case snfFloatInfPos:
			return INFINITY;
			
		case snfFloatInfNeg:
			return -INFINITY;
			
		case snfHex:
			return (long double)(wcstoull(data, NULL, 16));
			
		case snfOct:
			return (long double)(wcstoull(data, NULL, 8));
			
		case snfBit:
			return (long double)(wcstoull(data, NULL, 2));
			
		case snfFloat:
		case snfFloatEx:
			return wcstold(data, NULL);
			
		case snfInt:
			return (long double)(wcstoll(data, NULL, 10));
			
		case snfUInt:
			return (long double)(wcstoull(data, NULL, 10));
	}
	
	return 0;
}

//==============================================================

CString::operator bool() const {
	if (this->m_data == NULL)
		return false;
	
	uint32_t pos;
	uint32_t len;
	int type = getStringNumberFormat(this->m_data, this->m_length, &pos, &len);
	switch (type) {
		case snfNone:
			return this->m_length != 0; // Не пустое?
			
		case snfUndefined:
		case snfNil:
		case snfNull:
		case snfBoolFalse:
		case snfFloatNan:
			return false;
			
		case snfBoolTrue:
		case snfFloatInfPos:
		case snfFloatInfNeg:
			return true;
	}
	
	return wtoll(this->m_data + pos, len, type) != 0;
}

CString::operator int8_t() const {
	if (this->m_data == NULL)
		return 0;
	
	uint32_t pos, len;
	int type = getStringNumberFormat(this->m_data, this->m_length, &pos, &len);
	return static_cast<uint8_t>(wtoll(this->m_data + pos, len, type));
}

CString::operator uint8_t() const {
	if (this->m_data == NULL)
		return 0;
	
	uint32_t pos, len;
	int type = getStringNumberFormat(this->m_data, this->m_length, &pos, &len);
	return static_cast<uint8_t>(wtoull(this->m_data + pos, len, type));
}

CString::operator int16_t() const {
	if (this->m_data == NULL)
		return 0;
	
	uint32_t pos, len;
	int type = getStringNumberFormat(this->m_data, this->m_length, &pos, &len);
	return static_cast<int16_t>(wtoll(this->m_data + pos, len, type));
}

CString::operator uint16_t() const {
	if (this->m_data == NULL)
		return 0;
	
	uint32_t pos, len;
	int type = getStringNumberFormat(this->m_data, this->m_length, &pos, &len);
	return static_cast<uint16_t>(wtoull(this->m_data + pos, len, type));
}

CString::operator int32_t() const {
	if (this->m_data == NULL)
		return 0;
	
	uint32_t pos, len;
	int type = getStringNumberFormat(this->m_data, this->m_length, &pos, &len);
	return static_cast<int32_t>(wtoll(this->m_data + pos, len, type));
}

CString::operator uint32_t() const {
	if (this->m_data == NULL)
		return 0;
	
	uint32_t pos, len;
	int type = getStringNumberFormat(this->m_data, this->m_length, &pos, &len);
	return static_cast<uint32_t>(wtoull(this->m_data + pos, len, type));
}

CString::operator int64_t() const {
	if (this->m_data == NULL)
		return 0;
	
	uint32_t pos, len;
	int type = getStringNumberFormat(this->m_data, this->m_length, &pos, &len);
	return wtoll(this->m_data + pos, len, type);
}

CString::operator uint64_t() const {
	if (this->m_data == NULL)
		return 0;
	
	uint32_t pos, len;
	int type = getStringNumberFormat(this->m_data, this->m_length, &pos, &len);
	return wtoull(this->m_data + pos, len, type);
}

CString::operator float() const {
	if (this->m_data == NULL)
		return NAN;
	
	uint32_t pos, len;
	int type = getStringNumberFormat(this->m_data, this->m_length, &pos, &len);
	return (float)(wtod(this->m_data + pos, len, type));
}

CString::operator double() const {
	if (this->m_data == NULL)
		return NAN;
	
	uint32_t pos, len;
	int type = getStringNumberFormat(this->m_data, this->m_length, &pos, &len);
	return (double)(wtod(this->m_data + pos, len, type));
}

//==============================================================

bool CString::equals(const CString& string) const {
	if (m_length == string.m_length) {
		if (m_length == 0)
			return true;
		else
			return memcmp(m_data, string.m_data, m_size) == 0;
	}
	return false;
}

bool CString::equals(const wchar_t* string) const {
	if (string == NULL)
		return (m_length == 0);
	else if (m_length == wcs_strlen(string, NULL))
		return memcmp(m_data, string, m_size) == 0;
		
	return false;
}

bool CString::equals(const char* string) const {
	if (string == NULL)
		return (m_length == 0);
	else if (m_length != 0)
		return CString(string).equals(*this);
	
	return false;
}


int CString::compareTo(const CString& string) const {
	if (m_length == 0)
		return (string.m_length == 0) ? 0 : -1;
	else if (string.m_length == 0)
		return 1;
	else
		return wcscmp(m_data, string.m_data);
}

int CString::compareTo(const wchar_t* string) const {
	if (string == NULL)
		return (m_length == 0) ? 0 : 1;
	else if (m_length == 0)
		return *string == 0 ? 0 : -1;
	else if (*string == 0)
		return 1;
	else
		return wcscmp(m_data, string);
}

//==============================================================

CString& CString::toLowerCase() {
	if (m_length != 0)
		wcslwr(m_data);
	return *this;
}

CString& CString::toUpperCase() {
	if (m_length != 0)
		wcsupr(m_data);
	return *this;
}

//==============================================================

bool CString::equalsIgnoreCase(const CString& string) const {
	if (m_length == string.m_length) {
		if (m_length == 0)
			return true;
		
		wchar_t* ptr1 = m_data;
		wchar_t* ptr2 = string.m_data;
		uint32_t len = m_length;
		while (len-- > 0) {
			wchar_t v1 = towlower(*ptr1); ptr1++;
			wchar_t v2 = towlower(*ptr2); ptr2++;
			if (v1 != v2) return false;
		}
		return true;
	}
	return false;
}

bool CString::equalsIgnoreCase(const wchar_t* string) const {
	if (string == NULL)
		return (m_length == 0);
	
	if (m_length == wcs_strlen(string, NULL)) {
		if (m_length == 0)
			return true;
		else {
			wchar_t* ptr1 = m_data;
			wchar_t* ptr2 = (wchar_t*)string;
			uint32_t len = m_length;
			while (len > 0) {
				wchar_t v1 = towlower(*ptr1);
				wchar_t v2 = towlower(*ptr2);
				if (v1 != v2) return false;
				ptr1++; ptr2++; len--;
			}
			return true;
		}
	}
	return false;
}

int CString::compareToIgnoreCase(const CString& string) const {
	uint32_t len1 = m_length;
	uint32_t len2 = string.m_length;
	uint32_t len = (len1 < len2) ? len1 : len2;
	if (len1 == 0)
		return (len2 == 0) ? 0 : -1;
	else if (len2 == 0)
		return 1;
	
	wchar_t* ptr1 = m_data;
	wchar_t* ptr2 = string.m_data;
	while (len-- > 0) {
		wchar_t v1 = towlower(*ptr1); ptr1++;
		wchar_t v2 = towlower(*ptr2); ptr2++;
		if (v1 != v2) return (v1 < v2) ? -1 : 1;
	}
	if (len1 == len2)
		return 0;
	else
		return (len1 < len2) ? -1 : 1;
}

int CString::compareToIgnoreCase(const wchar_t* string) const {
	if (string == NULL)
		return (m_length == 0) ? 0 : 1;
	
	uint32_t len1 = m_length;
	uint32_t len2 = (uint32_t)wcs_strlen(string, NULL);
	uint32_t len = (len1 < len2) ? len1 : len2;
	if (len1 == 0)
		return (len2 == 0) ? 0 : -1;
	else if (len2 == 0)
		return 1;
	
	wchar_t* ptr1 = m_data;
	wchar_t* ptr2 = (wchar_t*)string;
	while (len-- > 0) {
		wchar_t v1 = towlower(*ptr1); ptr1++;
		wchar_t v2 = towlower(*ptr2); ptr2++;
		if (v1 != v2) return (v1 < v2) ? -1 : 1;
	}
	if (len1 == len2)
		return 0;
	else
		return (len1 < len2) ? -1 : 1;
}

//==============================================================

bool CString::startsWith(const CString& string, uint32_t start) const {
	uint32_t len2 = string.m_length;
	if (len2 == 0)
		return true;
	
	uint32_t len1 = m_length;
	if (len1 == 0)
		return false;
	
	uint32_t end = start + len2;
	if (len1 < end)
		return false;
	
	return memcmp(m_data + start, string.m_data, len2 * sizeof(wchar_t)) == 0;
}

bool CString::startsWith(const wchar_t* string, uint32_t start) const {
	if (string == NULL)
		return true;
	
	uint32_t len2 = (uint32_t)wcs_strlen(string, NULL);
	if (len2 == 0)
		return true;
	
	uint32_t len1 = m_length;
	if (len1 == 0)
		return false;
	
	uint32_t end = start + len2;
	if (len1 < end)
		return false;
	
	return memcmp(m_data + start, string, len2 * sizeof(wchar_t)) == 0;
}

bool CString::endsWith(const CString& string) const {
	uint32_t len2 = string.m_length;
	if (len2 == 0)
		return true;
	
	uint32_t len1 = m_length;
	if (len1 == 0)
		return false;
	
	if (len1 < len2)
		return false;

	return memcmp(m_data + len1 - len2, string.m_data, len2 * sizeof(wchar_t)) == 0;
}

bool CString::endsWith(const wchar_t* string) const {
	if (string == NULL)
		return true;
	
	uint32_t len2 = (uint32_t)wcs_strlen(string, NULL);
	if (len2 == 0)
		return true;

	uint32_t len1 = m_length;
	if (len1 == 0)
		return false;

	if (len1 < len2)
		return false;
	
	return memcmp(m_data + len1 - len2, string, len2 * sizeof(wchar_t)) == 0;
}

//==============================================================

int CString::indexOf(wchar_t character) const {
	if (m_length == 0)
		return -1;
	
	wchar_t* ptr = m_data;
	uint32_t len = m_length;
	uint32_t idx = 0;
	while (idx < len) {
		if (*ptr == character)
			return idx;
		idx++; ptr++;
	}
	return -1;
}

int CString::indexOf(const CString& string) const {
	uint32_t len1 = m_length;
	if (len1 == 0)
		return -1;
	
	uint32_t len2 = string.m_length;
	if ((len2 == 0) || (len2 > len1))
		return -1;

	if (len1 == len2)
		return memcmp(m_data, string.m_data, len1 * sizeof(wchar_t)) == 0;
	
	wchar_t* ptr = wcsstr(m_data, string.m_data);
	if (ptr == NULL)
		return -1;
	
	return (uint32_t)(intptr_t(ptr) - intptr_t(m_data)) / sizeof(wchar_t);
}

int CString::indexOf(const wchar_t* string) const {
	if (string == NULL)
		return -1;
	
	uint32_t len1 = m_length;
	if (len1 == 0)
		return -1;

	uint32_t len2 = (uint32_t)wcslen(string);
	if ((len2 == 0) || (len2 > len1))
		return -1;

	if (len1 == len2)
		return memcmp(m_data, string, len1 * sizeof(wchar_t)) == 0;

	wchar_t* ptr = wcsstr(m_data, string);
	if (ptr == NULL)
		return -1;

	return (uint32_t)(intptr_t(ptr) - intptr_t(m_data)) / sizeof(wchar_t);
}

int CString::lastIndexOf(wchar_t character) const {
	if (m_length == 0)
		return -1;
	
	uint32_t len = m_length - 1;
	wchar_t* ptr = m_data + len;
	while (len > 0) {
		ptr--; len--;
		if (*ptr == character) return len;
	}
	return -1;
}

int CString::lastIndexOf(const CString& string) const {
	uint32_t len1 = m_length;
	if (len1 == 0)
		return -1;
	
	uint32_t len2 = string.m_length;
	if ((len2 == 0) || (len2 > len1))
		return -1;
	
	if (len1 == len2)
		return memcmp(m_data, string.m_data, len1 * sizeof(wchar_t)) == 0;
	
	wchar_t* ptr2 = string.m_data;
	wchar_t ch2 = *ptr2;
	int32_t len = len1 - len2;
	wchar_t* ptr1 = m_data + len;
	while (len >= 0) {
		if (*ptr1 == ch2) {
			if (memcmp(ptr1, ptr2, len2 * sizeof(wchar_t)) == 0)
				return len;
		}
		ptr1--; len--;
	}
	return -1;
}

int CString::lastIndexOf(const wchar_t* string) const {
	if (string == NULL)
		return -1;

	uint32_t len1 = m_length;
	if (len1 == 0)
		return -1;

	uint32_t len2 = (uint32_t)wcslen(string);
	if ((len2 == 0) || (len2 > len1))
		return -1;

	if (len1 == len2)
		return memcmp(m_data, string, len1 * sizeof(wchar_t)) == 0;

	wchar_t ch2 = *string;
	int32_t len = len1 - len2;
	wchar_t* ptr1 = m_data + len;
	while (len >= 0) {
		if (*ptr1 == ch2) {
			if (memcmp(ptr1, string, len2 * sizeof(wchar_t)) == 0)
				return len;
		}
		ptr1--; len--;
	}
	return -1;
}

//==============================================================

CString CString::substring(int start, int end) const {
	int32_t len = m_length;
	int32_t pos;
	if (start < 0) {
		pos = len + start;
		if (pos < 0) pos = 0;
		len -= pos;
	} else {
		pos = start;
		if (pos >= len)
			return StringNil;
		
		if ((end < 0) || (end > len)) {
			len -= pos;
		} else {
			len = (end - pos);
		}
	}
	if (len == 0)
		return StringEmpty;
	
	return CString(m_data + pos, len);
}

wchar_t& CString::operator [](int index) const throw(const char*) {
	if ((this->m_data == NULL) || (index < 0) || (index >= m_length))
		throw eOutOfRange;
	return m_data[index];
}

wchar_t CString::charAt(int index) const throw(const char*) {
	if ((this->m_data == NULL) || (index < 0) || (index >= m_length))
		throw eOutOfRange;
	return m_data[index];
}

//==============================================================

CString CString::replace(const CString& target, const CString& replacement) const {
	int pos = indexOf(target);
	if (pos < 0)
		return *this;
	
	int len = target.m_length;
	if (pos > 0) {
		if (m_length > (pos + len))
			return substring(0, pos).concat(replacement).concat(substring(pos + len));
		else
			return substring(0, pos).concat(replacement);
	} else
		return CString(replacement).concat(substring(pos + len));
}

CString CString::replace(const wchar_t* target, const wchar_t* replacement) const {
	int pos = indexOf(target);
	if (pos < 0)
		return *this;
	
	int len = (uint32_t)wcs_strlen(target, NULL);
	if (pos > 0) {
		if (m_length > (pos + len))
			return substring(0, pos).concat(replacement).concat(substring(pos + len));
		else
			return substring(0, pos).concat(replacement);
	} else
		return CString(replacement).concat(substring(pos + len));
}

//==============================================================

#if defined(__IOS__)

bool CString::matches(const CString& regularExpression) const {
	if (regularExpression.m_length == 0) {
		return m_length == 0;
	}
	NSString* expression = regularExpression;
	NSString* string = this->operator NSString*();
	if ((expression == NULL) || (string == NULL))
		return false;
	
	NSError *error = NULL;
	NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:expression options:NSRegularExpressionCaseInsensitive error:&error];
	int start = 0;
	int end = m_length;
	NSTextCheckingResult* find = [regex firstMatchInString:string options:0 range:NSMakeRange(start, end - start)];
	return (find != NULL);
}

bool CString::regionMatches(bool ignoreCase, int thisStart, const CString& string, int start, int length) const {
	if (!ignoreCase)
		return regionMatches(thisStart, string, start, length);
	
	if ((thisStart < 0) || (length < 0))
		return false;

	uint32_t len1 = m_length;
	if (thisStart > len1)
		return false;
		
	uint32_t len2 = string.m_length;
	if (thisStart == len1)
		return len2 == 0;

	if (start >= len2)
		return false;

	if ((start + length) > len2)
		length = len2 - start;

	if ((thisStart + length) > len1)
		length = len1 - thisStart;

	wchar_t* ptr1 = m_data + thisStart;
	wchar_t* ptr2 = string.m_data + start;
	while (length-- > 0) {
		wchar_t v1 = towlower(*ptr1); ptr1++;
		wchar_t v2 = towlower(*ptr2); ptr2++;
		if (v1 != v2) return false;
	}
	return true;
}

bool CString::regionMatches(int thisStart, const CString& string, int start, int length) const {
	if ((thisStart < 0) || (length < 0))
		return false;
	
	uint32_t len1 = m_length;
	if (thisStart > len1)
		return false;

	uint32_t len2 = string.m_length;
	if (thisStart == len1)
		return len2 == 0;

	if (start >= len2)
		return false;

	if ((start + length) > len2)
		length = len2 - start;

	if ((thisStart + length) > len1)
		length = len1 - thisStart;

	return memcmp(m_data + thisStart, string.m_data + start, length * sizeof(wchar_t)) == 0;
}

CString CString::replaceAll(const CString& regularExpression, const CString& replacement) const {
	if (regularExpression.m_length == 0)
		return *this;

	NSString* expression = regularExpression;
	NSString* string = this->operator NSString*();
	if ((expression == NULL) || (string == NULL))
		return *this;
	
	NSError *error = NULL;
	NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:expression options:0 error:&error];
	int start = 0;
	int end = m_length;
	CString result = StringEmpty;
	NSArray* matches = [regex matchesInString:string options:0 range:NSMakeRange(start, end - start)];
	if ((matches == NULL) || (matches.count == 0))
		return *this;
	
	for (NSTextCheckingResult* find in matches) {
		if (find.range.location == start)
			result.concat(replacement);
		else
			result.concat(substring(start, (int32_t)find.range.location)).concat(replacement);

		start = (int32_t)find.range.location + (int32_t)find.range.length;
	}
	if (start < end)
		result.concat(substring(start));

	return result;
}

CString CString::replaceFirst(const CString& regularExpression, const CString& replacement) const {
	if (regularExpression.m_length == 0)
		return *this;

	NSString* expression = regularExpression;
	NSString* string = this->operator NSString*();
	if ((expression == NULL) || (string == NULL))
		return *this;

	NSError *error = NULL;
	NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:expression options:0 error:&error];
	int start = 0;
	int end = m_length;
	NSTextCheckingResult* find = [regex firstMatchInString:string options:0 range:NSMakeRange(start, end - start)];
	if (find == NULL)
		return *this;

	if (find.range.location == 0)
		return CString(replacement).concat(substring((int32_t)find.range.location + (int32_t)find.range.length));
	else if ((find.range.location + find.range.length) >= m_length)
		return substring(0, (int32_t)find.range.location).concat(replacement);
	else
		return substring(0, (int32_t)find.range.location).concat(replacement).concat(substring((int32_t)find.range.location + (int32_t)find.range.length));
}

/*
JList<JString> CString::split(const JString& regularExpression, uint32_t limit = 0) const {
 JList<JString> list = new JArrayList<JString>();
 if ((regularExpression._object == nil) || (((CString*)regularExpression._object)->strLength == 0)) {
 list.add(*this);
 return list;
 }
 NSString* expression = regularExpression;
 NSString* string = toNSString();
 if ((expression == nil) || (string == nil)) {
 list.add(*this);
 return list;
 }
 NSError *error = nil;
 NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:expression options:0 error:&error];
 int start = 0;
 int end = strLength;
 NSArray* matches = [regex matchesInString:string options:0 range:NSMakeRange(start, end - start)];
 if ((matches == nil) || (matches.count == 0)) {
 list.add(*this);
 return list;
 }
 int count = 0;
 for (NSTextCheckingResult* find in matches) {
 if (find.range.location != start) {
 list.add(substring(start, (int32_t)find.range.location));
 count++;
 if ((limit != 0) && (count >= limit)) break;
 }
 start = (int32_t)find.range.location + (int32_t)find.range.length;
 }
 if (start < end) {
 if ((limit == 0) || (count < limit)) {
 list.add(substring(start));
 }
 }
 return list;
}
 */

#endif

//==============================================================

CString& CString::trim() {
	if (m_length == 0)
		return *this;
	
	wchar_t* src = m_data;
	wchar_t* srcEnd = src + m_length - 1;
	while ((*src == L'\n') || (*src == L'\r') || (*src == L'\t') || (*src == L' ')) {
		src++;
	}

	if (srcEnd != src) {
		while ((*srcEnd == L'\n') || (*srcEnd == L'\r') || (*srcEnd == L'\t') || (*srcEnd == L' ')) {
			srcEnd--;
			if (srcEnd < src) break;
		}
	}
	srcEnd++;
	uint32_t len = (uint32_t)((intptr_t)srcEnd - (intptr_t)src);
	if (len > 0)
		memcpy(m_data, src, len);
	
	setLength(len / sizeof(wchar_t));
	return *this;
}

//==============================================================

#define va_format(type) \
	int fmtLen = (int)((intptr_t)(++ptr) - (intptr_t)fmt) / sizeof(wchar_t); \
	memcpy(format, fmt, fmtLen * sizeof(wchar_t)); \
	format[fmtLen] = 0; \
	type val = va_arg(arglist, type); \
	int resLen = ::swprintf(buffer, 64, format, val); \
	if (resLen != EOF) { \
		int ofs = m_length; \
		setLength(ofs + resLen); \
		memcpy(m_data + ofs, buffer, (resLen + 1) * sizeof(wchar_t)); \
	}

#define va_store(type) \
	++ptr; \
	type* val = va_arg(arglist, type*); \
	*val = m_length;

#define va_concat(type) \
	++ptr; \
	type val = va_arg(arglist, type); \
	this->concat(val);

int CString::vswprintf(const wchar_t* string, va_list arglist) {
	setLength(0);
	
	bool error = false;
	
	wchar_t format[32];
	wchar_t buffer[64];
	
	wchar_t *src = (wchar_t*)string;
	wchar_t *ptr = src;
	wchar_t ch;
	while ((ch = *ptr) != 0) {
		// %([-\+\ #0])?([0123456789]+|\*)?(\.([0123456789]+)|(\*))?(hh|h|l|ll|j|z|t|L)?(d|i|u|o|x|X|f|F|e|E|g|G|a|A|c|s|p|n|%)
		if (ch == L'%') {
			if (src != ptr) {
				uint32_t resLen = (uint32_t)((intptr_t)ptr - (intptr_t)src) / sizeof(wchar_t);
				int ofs = m_length;
				setLength(ofs + resLen);
				memcpy(m_data + ofs, src, resLen * sizeof(wchar_t));
			}
			
			wchar_t *fmt = ptr;
			ch = *(++ptr);
			if (ch == L'%') {
				src = ptr;
				ptr++;
			} else {
				while (ch != 0) {
					if ((ch != L'-') && (ch != L'+') && (ch != L' ') && (ch != L'#') && (ch != L'0')) break;
					ch = *(++ptr);
				}
				while (ch != 0) {
					if (ch == L'*') { ++ptr; break; }
					else if ((ch < L'0') || (ch > L'9')) break;
					ch = *(++ptr);
				}
				if (ch == L'.') {
					ch = *(++ptr);
					while (ch != 0) {
						if (ch == L'*') { ++ptr; break; }
						else if ((ch < L'0') || (ch > L'9')) break;
						ch = *(++ptr);
					}
				}
				if (ch == L'h') {
					ch = *(++ptr);
					if (ch == L'h') {
						ch = *(++ptr);
						if ((ch == L'd') || (ch == L'i')) {
							va_format(int8_t);
						} else if ((ch == L'u') || (ch == L'o') || (ch == L'x') || (ch == L'X')) {
							va_format(uint8_t);
						} else if (ch == L'n') {
							va_store(int8_t);
						} else {
							error = true;
							break;
						}
					} else {
						if ((ch == L'd') || (ch == L'i')) {
							va_format(int16_t);
						} else if ((ch == L'u') || (ch == L'o') || (ch == L'x') || (ch == L'X')) {
							va_format(uint16_t);
						} else if (ch == L'n') {
							va_store(int16_t);
						} else {
							error = true;
							break;
						}
					}
				} else if (ch == L'l') {
					ch = *(++ptr);
					if (ch == L'l') {
						ch = *(++ptr);
						if ((ch == L'd') || (ch == L'i')) {
							va_format(int64_t);
						} else if ((ch == L'u') || (ch == L'o') || (ch == L'x') || (ch == L'X')) {
							va_format(uint64_t);
						} else if ((ch == L'f') || (ch == L'F') || (ch == L'e') || (ch == L'E') || (ch == L'g') || (ch == L'G') || (ch == L'a') || (ch == L'A')) {
							int fmtLen = (int)((intptr_t)ptr - (intptr_t)fmt);
							memcpy(format, fmt, fmtLen * sizeof(wchar_t));
							format[fmtLen - 3] = L'L';
							format[fmtLen - 2] = ch;
							format[fmtLen - 1] = 0;
							long double val = va_arg(arglist, long double);
							int resLen = ::swprintf(buffer, 64, format, val);
							if (resLen != EOF) {
								int ofs = m_length;
								setLength(ofs + resLen);
								memcpy(m_data + ofs, buffer, (resLen + 1) * sizeof(wchar_t));
							}
						} else if (ch == L'n') {
							va_store(int64_t);
						} else {
							error = true;
							break;
						}
					} else {
						if ((ch == L'd') || (ch == L'i')) {
							va_format(int32_t);
						} else if ((ch == L'u') || (ch == L'o') || (ch == L'x') || (ch == L'X')) {
							va_format(uint32_t);
						} else if (ch == L'c') {
							va_concat(wchar_t);
						} else if (ch == L's') {
							va_concat(wchar_t*);
						} else if ((ch == L'f') || (ch == L'F') || (ch == L'e') || (ch == L'E') || (ch == L'g') || (ch == L'G') || (ch == L'a') || (ch == L'A')) {
							int fmtLen = (int)((intptr_t)ptr - (intptr_t)fmt);
							memcpy(format, fmt, fmtLen * sizeof(wchar_t));
							format[fmtLen - 3] = ch;
							format[fmtLen - 2] = 0;
							double val = va_arg(arglist, double);
							int resLen = ::swprintf(buffer, 64, format, val);
							if (resLen != EOF) {
								int ofs = m_length;
								setLength(ofs + resLen);
								memcpy(m_data + ofs, buffer, (resLen + 1) * sizeof(wchar_t));
							}
						} else if (ch == L'n') {
							va_store(int32_t);
						} else {
							error = true;
							break;
						}
					}
				} else if (ch == L'j') {
					ch = *(++ptr);
					if ((ch == L'd') || (ch == L'i')) {
						va_format(intmax_t);
					} else if ((ch == L'u') || (ch == L'o') || (ch == L'x') || (ch == L'X')) {
						va_format(uintmax_t);
					} else if (ch == L'n') {
						va_store(intmax_t);
					} else {
						error = true;
						break;
					}
				} else if (ch == L'z') {
					ch = *(++ptr);
					if ((ch == L'd') || (ch == L'i') || (ch == L'u') || (ch == L'o') || (ch == L'x') || (ch == L'X')) {
						va_format(size_t);
					} else if (ch == L'n') {
						va_store(size_t);
					} else {
						error = true;
						break;
					}
				} else if (ch == L't') {
					ch = *(++ptr);
					if ((ch == L'd') || (ch == L'i') || (ch == L'u') || (ch == L'o') || (ch == L'x') || (ch == L'X')) {
						va_format(ptrdiff_t);
					} else if (ch == L'n') {
						va_store(ptrdiff_t);
					} else {
						error = true;
						break;
					}
				} else if (ch == L'L') {
					ch = *(++ptr);
					if ((ch == L'f') || (ch == L'F') || (ch == L'e') || (ch == L'E') || (ch == L'g') || (ch == L'G') || (ch == L'a') || (ch == L'A')) {
						va_format(long double);
					} else {
						error = true;
						break;
					}
				} else {
					if ((ch == L'd') || (ch == L'i')) {
						va_format(int32_t);
					} else if ((ch == L'u') || (ch == L'o') || (ch == L'x') || (ch == L'X')) {
						va_format(uint32_t);
					} else if ((ch == L'f') || (ch == L'F') || (ch == L'e') || (ch == L'E') || (ch == L'g') || (ch == L'G') || (ch == L'a') || (ch == L'A')) {
						va_format(double);
					} else if (ch == L'c') {
						va_concat(char);
					} else if (ch == L's') {
						va_concat(char*);
					} else if (ch == L'p') {
						va_format(void*);
					} else if (ch == L'n') {
						va_store(int32_t);
					} else if (ch == L'@') {
						va_concat(NSString*);
					} else {
						error = true;
						break;
					}
				}
				src = ptr;
			}
		} else {
			ptr++;
		}
	}
	
	if (error) {
		return -1;
	}
	
	if (src != ptr) {
		uint32_t resLen = (uint32_t)((intptr_t)ptr - (intptr_t)src) / sizeof(wchar_t);
		int ofs = m_length;
		setLength(ofs + resLen);
		memcpy(m_data + ofs, src, resLen * sizeof(wchar_t));
	}
	
	return m_length;
}

CString CString::format(const CString& string, ...) {
	if (string.m_length == 0)
		return StringEmpty;

	va_list arglist;
	va_start(arglist, string);

	CString result;
	if (result.vswprintf(string.m_data, arglist) == EOF) {
		va_end(arglist);
		return StringNil;
	}

	va_end(arglist);
	return result;
}

CString CString::format(const wchar_t* string, ...) {
	if (string == NULL)
		return StringNil;

	uint32_t lenf = (uint32_t)wcs_strlen(string, NULL);
	if (lenf == 0)
		return StringEmpty;
	
	va_list arglist;
	va_start(arglist, string);
	CString result;
	if (result.vswprintf(string, arglist) == EOF) {
		va_end(arglist);
		return StringNil;
	}
	va_end(arglist);
	return result;
}

//==============================================================

#include <cipher/uMD5.h>

CString CString::md5() const throw(const char*) {
	uint32_t size = wcs_toutf8_size(m_data);
	char* utf8 = memAlloc(char, utf8, size);
	if (utf8 == NULL)
		throw eOutOfMemory;
		
	struct tMD5 m;
	md5_init(&m);
	md5_update(&m, (uint8_t*)utf8, size);
	md5_final(&m);
	memFree(utf8);
	
	uint8_t* result = (uint8_t*)(m.state);
	
	return CString::format(L"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
							 result[0], result[1], result[2], result[3],
							 result[4], result[5], result[6], result[7],
							 result[8], result[9], result[10], result[11],
							 result[12], result[13], result[14], result[15]
							 );
}

CString CString::toString() const {
	return m_data;
}

CString CString::toJSON() const {
	if (m_data == NULL)
		return L"null";
	
	return CString(L"\"").concat(*this).concat(L"\"");
}


//==============================================================

void uStringInit() {
	
}

void uStringQuit() {
	
}
