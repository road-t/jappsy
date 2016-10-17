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

#ifndef JAPPSY_USTRING_H
#define JAPPSY_USTRING_H

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * Get string length
	 * @param			char*		src			UTF8 encoded null terminated source string
	 * @param	[out]	uint32_t*	strsize		source string memory size with null character
	 * @return			uint32_t				source string length without null character
	 */
	uint32_t utf8_strlen(const char* src, uint32_t* strsize);

	/**
	 * Get string length
	 * @param			char*		src			UTF8 encoded null terminated source string
	 * @param			uint32_t	srcsize		source string memory size
	 * @param	[out]	uint32_t*	strsize		source string memory size without null character
	 * @return			uint32_t				source string length without null character
	 */
	uint32_t utf8_strlen_nzt(const char* src, uint32_t srcsize, uint32_t* strsize);

	/**
	 * Calculate memory size required for string encoding from UTF8 to UTF16
	 * @param			char*		src			UTF8 encoded null terminated source string
	 * @return			uint32_t				UTF16 encoded string memory size with null character
	 */
	uint32_t utf8_toutf16_size(const char* src);

	/**
	 * Convert string
	 * @param			char*		src			UTF8 encoded null terminated source string
	 * @param	[out]	uint16_t*	dst			UTF16 encoded null terminated destination string
	 * @param			uint32_t	dstsize		UTF16 destination string buffer memory size with null character
	 * @return			uint32_t				UTF16 destination string used memory size with null character
	 */
	uint32_t utf8_toutf16(const char* src, uint16_t* dst, uint32_t dstsize);
	
	/**
	 * Convert string
	 * @param			char*		src			UTF8 encoded null terminated source string
	 * @param	[out]	uint32_t*	dst			UTF32 encoded null terminated destination string
	 * @param			uint32_t	dstsize		UTF32 destination string buffer memory size with null character
	 * @return			uint32_t				UTF32 destination string used memory size with null character
	 */
	uint32_t utf8_toutf32(const char* src, uint32_t* dst, uint32_t dstsize);

	/**
	 * Get string length
	 * @param			uint16_t*	src			UTF16 encoded null terminated source string
	 * @param	[out]	uint32_t*	strsize		source string memory size with null character
	 * @return			uint32_t				source string length without null character
	 */
	uint32_t utf16_strlen(const uint16_t* src, uint32_t* strsize);

	/**
	 * Calculate memory size required for string encoding from UTF16 to UTF8
	 * @param			uint16_t*	src			UTF16 encoded null terminated source string
	 * @return			uint32_t				UTF8 encoded string memory size with null character
	 */
	uint32_t utf16_toutf8_size(const uint16_t* src);

	/**
	 * Convert string
	 * @param			uint16_t*	src			UTF16 encoded null terminated source string
	 * @param	[out]	char*		dst			UTF8 encoded null terminated destination string
	 * @param			uint32_t	dstsize		UTF8 destination string buffer memory size with null character
	 * @return			uint32_t				UTF8 destination string used memory size with null character
	 */
	uint32_t utf16_toutf8(const uint16_t* src, char* dst, uint32_t dstsize);

	/**
	 * Convert string
	 * @param			uint16_t*	src			UTF16 encoded null terminated source string
	 * @param	[out]	uint32_t*	dst			UTF32 encoded null terminated destination string
	 * @param			uint32_t	dstsize		UTF32 destination string buffer memory size with null character
	 * @return			uint32_t				UTF32 destination string used memory size with null character
	 */
	uint32_t utf16_toutf32(const uint16_t* src, uint32_t* dst, uint32_t dstsize);

	/**
	 * Get string length
	 * @param			uint32_t*	src			UTF32 encoded null terminated source string
	 * @param	[out]	uint32_t*	strsize		source string memory size with null character
	 * @return			uint32_t				source string length without null character
	 */
	uint32_t utf32_strlen(const uint32_t* src, uint32_t* strsize);

	/**
	 * Calculate memory size required for string encoding from UTF32 to UTF8
	 * @param			uint32_t*	src			UTF32 encoded null terminated source string
	 * @return			uint32_t				UTF8 encoded string memory size with null character
	 */
	uint32_t utf32_toutf8_size(const uint32_t* src);

	/**
	 * Convert string
	 * @param			uint32_t*	src			UTF32 encoded null terminated source string
	 * @param	[out]	char*		dst			UTF8 encoded null terminated destination string
	 * @param			uint32_t	dstsize		UTF8 destination string buffer memory size with null character
	 * @return			uint32_t				UTF8 destination string used memory size with null character
	 */
	uint32_t utf32_toutf8(const uint32_t* src, char* dst, uint32_t dstsize);

	/**
	 * Calculate memory size required for string encoding from UTF32 to UTF16
	 * @param			uint32_t*	src			UTF32 encoded null terminated source string
	 * @return			uint32_t				UTF16 encoded string memory size with null character
	 */
	uint32_t utf32_toutf16_size(const uint32_t* src);

	/**
	 * Convert string
	 * @param			uint32_t*	src			UTF32 encoded null terminated source string
	 * @param	[out]	uint16_t*	dst			UTF16 encoded null terminated destination string
	 * @param			uint32_t	dstsize		UTF16 destination string buffer memory size with null character
	 * @return			uint32_t				UTF16 destination string used memory size with null character
	 */
	uint32_t utf32_toutf16(const uint32_t* src, uint16_t* dst, uint32_t dstsize);

	#if __WCHAR_MAX__ > 0x10000
		/**
		 * Get string length
		 * @param			wchar_t*	src			Unicode null terminated source string
		 * @param	[out]	uint32_t*	strsize		source string memory size with null character
		 * @return			uint32_t				source string length without null character
		 */
		#define wcs_strlen(src, sizeptr)	utf32_strlen((uint32_t*)(src), (sizeptr))

		/**
		 * Calculate memory size required for string encoding from Unicode to UTF8
		 * @param			wchar_t*	src			Unicode null terminated source string
		 * @return			uint32_t				UTF8 encoded string memory size with null character
		 */
		#define wcs_toutf8_size(src)		utf32_toutf8_size((uint32_t*)(src))

		/**
		 * Convert string
		 * @param			wchar_t*	src			Unicode null terminated source string
		 * @param	[out]	uint8_t*	dst			UTF8 encoded null terminated destination string
		 * @param			uint32_t	dstsize		UTF8 destination string buffer memory size with null character
		 * @return			uint32_t				UTF8 destination string used memory size with null character
		 */
		#define wcs_toutf8(src, dst, size)	utf32_toutf8((uint32_t*)(src), (dst), (size))

		/**
		 * Calculate memory size required for string encoding from UTF8 to Unicode
		 * @param			char*		src			UTF8 encoded null terminated source string
		 * @return			uint32_t				Unicode string memory size with null character
		 */
		#define utf8_towcs_size(src)		(utf8_strlen((src), 0) * 4)

		/**
		 * Convert string
		 * @param			char*		src			UTF8 encoded null terminated source string
		 * @param	[out]	wchar_t*	dst			Unicode null terminated destination string
		 * @param			uint32_t	dstsize		destination string buffer memory size with null character
		 * @return			uint32_t				destination string used memory size with null character
		 */
		#define utf8_towcs(src, dst, size)	utf8_toutf32((src), (uint32_t*)(dst), (size))
	#else
		/**
		 * Get string length
		 * @param			wchar_t*	src			Unicode null terminated source string
		 * @param	[out]	uint32_t*	strsize		source string memory size with null character
		 * @return			uint32_t				source string length without null character
		 */
		#define wcs_strlen(src, sizeptr)	utf16_strlen((uint16_t*)(src), (sizeptr))

		/**
		 * Calculate memory size required for string encoding from Unicode to UTF8
		 * @param			wchar_t*	src			Unicode null terminated source string
		 * @return			uint32_t				UTF8 encoded string memory size with null character
		 */
		#define wcs_toutf8_size(src)		utf16_toutf8_size((uint16_t*)(src))

		/**
		 * Convert string
		 * @param			wchar_t*	src			Unicode null terminated source string
		 * @param	[out]	uint8_t*	dst			UTF8 encoded null terminated destination string
		 * @param			uint32_t	dstsize		UTF8 destination string buffer memory size with null character
		 * @return			uint32_t				UTF8 destination string used memory size with null character
		 */
		#define wcs_toutf8(src, dst, size)	utf16_toutf8((uint16_t*)(src), (dst), (size))

		/**
		 * Calculate memory size required for string encoding from UTF8 to Unicode
		 * @param			char*		src			UTF8 encoded null terminated source string
		 * @return			uint32_t				Unicode string memory size with null character
		 */
		#define utf8_towcs_size(src)		utf8_toutf16_size((src))

		/**
		 * Convert string
		 * @param			char*		src			UTF8 encoded null terminated source string
		 * @param	[out]	wchar_t*	dst			Unicode null terminated destination string
		 * @param			uint32_t	dstsize		destination string buffer memory size with null character
		 * @return			uint32_t				destination string used memory size with null character
		 */
		#define utf8_towcs(src, dst, size)	utf8_toutf16((src), (uint16_t*)(dst), (size))
	#endif

	#include <wchar.h>

	#ifndef wcslwr
		#define STRING_WCSLWR

		/**
		 * Convert string to lower case
		 * @param			wchar_t*	s			Unicode null terminated string
		 * @return			wchar_t*				source string pointer
		 * @see {@link setlocale(int, char*) setlocale}
		 * @see {@link #setlocale(int, char*) setlocale} setlocale(C_CTYPE, "en_EN.UTF-8")
		 */
		wchar_t* wcslwr(wchar_t* s);

		/**
		 * Convert string to upper case
		 * @param			wchar_t*	s			Unicode null terminated string
		 * @return			wchar_t*				source string pointer
		 */
		wchar_t* wcsupr(wchar_t* s);
	#endif
	
	int utf8_cmpi(const char* s1, const char* s2);
	int wcs_cmpi(const wchar_t* s1, const wchar_t* s2);

	void uStringInit();
	void uStringQuit();

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

	#include <cipher/uCrc.h>
	#include <core/uMemory.h>
	#include <core/uAtomic.h>
	#include <data/uObject.h>

#if !defined(__OBJC__)
	class NSString;
#endif

	class CString : public CObject {
	public:
	// Content
		// UTF16 encoded null terminated string pointer
		wchar_t* m_data = NULL;
		// JString characters count without null character
		uint32_t m_length = 0;
		// JString memory size with null character
		uint32_t m_size = 0;
		// Aligned string buffer memory size
		uint32_t m_memorySize = 0;

	// Content Resize
		void setSize(uint32_t size) throw(const char*);
		void setLength(uint32_t length) throw(const char*);

	// Destructor
		~CString();
		
	// Constructors
		inline CString() { }
		explicit CString(const void* string) throw(const char*);

		CString(const CString& string) throw(const char*);
#if defined(__IOS__)
		CString(const NSString* string) throw(const char*);
#endif
		CString(const char* string) throw(const char*);
		explicit CString(const char* string, uint32_t size) throw(const char*);
		CString(const wchar_t* string) throw(const char*);
		explicit CString(const wchar_t* string, uint32_t length) throw(const char*);
		explicit CString(const char character) throw(const char*);
		explicit CString(const wchar_t character) throw(const char*);
		explicit CString(bool value) throw(const char*);
		CString(int8_t value) throw(const char*);
		CString(uint8_t value) throw(const char*);
		CString(int16_t value) throw(const char*);
		CString(uint16_t value) throw(const char*);
		CString(int32_t value) throw(const char*);
		CString(uint32_t value) throw(const char*);
		CString(int64_t value) throw(const char*);
		CString(uint64_t value) throw(const char*);
		CString(float value) throw(const char*);
		CString(double value) throw(const char*);

	// Setters
		CString& operator =(const void* string) throw(const char*);
		CString& operator =(const CString& string) throw(const char*);
		#if defined(__IOS__)
			CString& operator =(const NSString* string) throw(const char*);
		#endif
		CString& operator =(const char* string) throw(const char*);
		CString& operator =(const wchar_t* string) throw(const char*);
		CString& operator =(const char character) throw(const char*);
		CString& operator =(const wchar_t character) throw(const char*);
		CString& operator =(bool value) throw(const char*);
		CString& operator =(int8_t value) throw(const char*);
		CString& operator =(uint8_t value) throw(const char*);
		CString& operator =(int16_t value) throw(const char*);
		CString& operator =(uint16_t value) throw(const char*);
		CString& operator =(int32_t value) throw(const char*);
		CString& operator =(uint32_t value) throw(const char*);
		CString& operator =(int64_t value) throw(const char*);
		CString& operator =(uint64_t value) throw(const char*);
		CString& operator =(float value) throw(const char*);
		CString& operator =(double value) throw(const char*);

	// Concatenaters
		CString& concat(const CString& string) throw(const char*);
#if defined(__IOS__)
		CString& concat(const NSString* string) throw(const char*);
#endif
		CString& concat(const char* string) throw(const char*);
		CString& concat(const wchar_t* string) throw(const char*);
		CString& concat(const char character) throw(const char*);
		CString& concat(const wchar_t character) throw(const char*);
		CString& concat(const bool value) throw(const char*);
		CString& concat(const int8_t value) throw(const char*);
		CString& concat(const uint8_t value) throw(const char*);
		CString& concat(const int16_t value) throw(const char*);
		CString& concat(const uint16_t value) throw(const char*);
		CString& concat(const int32_t value) throw(const char*);
		CString& concat(const uint32_t value) throw(const char*);
		CString& concat(const int64_t value) throw(const char*);
		CString& concat(const uint64_t value) throw(const char*);
		CString& concat(const float value) throw(const char*);
		CString& concat(const double value) throw(const char*);

		CString& concatPath(const CString& string) throw(const char*);

		inline CString& operator +=(const CString& string) throw(const char*) { return this->concat(string); };
#if defined(__IOS__)
		inline CString& operator +=(const NSString* string) throw(const char*) { return this->concat(string); };
#endif
		inline CString& operator +=(const char* string) throw(const char*) { return this->concat(string); };
		inline CString& operator +=(const wchar_t* string) throw(const char*) { return this->concat(string); };
		inline CString& operator +=(const char character) throw(const char*) { return this->concat(character); };
		inline CString& operator +=(const wchar_t character) throw(const char*) { return this->concat(character); };
		inline CString& operator +=(bool value) throw(const char*) { return this->concat(value); };
		inline CString& operator +=(int8_t value) throw(const char*) { return this->concat(value); };
		inline CString& operator +=(uint8_t value) throw(const char*) { return this->concat(value); };
		inline CString& operator +=(int16_t value) throw(const char*) { return this->concat(value); };
		inline CString& operator +=(uint16_t value) throw(const char*) { return this->concat(value); };
		inline CString& operator +=(int32_t value) throw(const char*) { return this->concat(value); };
		inline CString& operator +=(uint32_t value) throw(const char*) { return this->concat(value); };
		inline CString& operator +=(int64_t value) throw(const char*) { return this->concat(value); };
		inline CString& operator +=(uint64_t value) throw(const char*) { return this->concat(value); };
		inline CString& operator +=(float value) throw(const char*) { return this->concat(value); };
		inline CString& operator +=(double value) throw(const char*) { return this->concat(value); };
		
		
		static inline CString valueOf(char value) throw(const char*) { return CString(value); }
		static inline CString valueOf(wchar_t value) throw(const char*) { return CString(value); }
		static inline CString valueOf(bool value) throw(const char*) { return CString(value); }
		static inline CString valueOf(int8_t value) throw(const char*) { return CString(value); }
		static inline CString valueOf(uint8_t value) throw(const char*) { return CString(value); }
		static inline CString valueOf(int16_t value) throw(const char*) { return CString(value); }
		static inline CString valueOf(uint16_t value) throw(const char*) { return CString(value); }
		static inline CString valueOf(int32_t value) throw(const char*) { return CString(value); }
		static inline CString valueOf(uint32_t value) throw(const char*) { return CString(value); }
		static inline CString valueOf(int64_t value) throw(const char*) { return CString(value); }
		static inline CString valueOf(uint64_t value) throw(const char*) { return CString(value); }
		static inline CString valueOf(float value) throw(const char*) { return CString(value); }
		static inline CString valueOf(double value) throw(const char*) { return CString(value); }

	// Type Converters
		#if defined(__IOS__)
			operator NSString*() const;
		#endif
		explicit inline operator wchar_t*() const { return this->m_data; };
		explicit operator bool() const;
		explicit operator int8_t() const;
		explicit operator uint8_t() const;
		explicit operator int16_t() const;
		explicit operator uint16_t() const;
		explicit operator int32_t() const;
		explicit operator uint32_t() const;
		explicit operator int64_t() const;
		explicit operator uint64_t() const;
		explicit operator float() const;
		explicit operator double() const;

	// Sign support
		inline double operator +() const { return this->operator double(); }
		inline double operator -() const { return -(this->operator double()); }

	// A + B support
		friend inline CString operator +(const CString& src1, const CString& src2) { return CString(src1).concat(src2);	}
		
	// Comparators
		bool equals(const CString& string) const;
		bool equals(const wchar_t* string) const;
		bool equals(const char* string) const;

		inline bool operator ==(const CString& string) const { return this->equals(string); }
		inline bool operator ==(const wchar_t* string) const { return this->equals(string); }
		inline bool operator ==(const char* string) const { return this->equals(string); }

		inline bool operator !=(const CString& string) const { return !(this->equals(string)); }
		inline bool operator !=(const wchar_t* string) const { return !(this->equals(string)); }
		inline bool operator !=(const char* string) const { return !(this->equals(string)); }

		int compareTo(const CString& string) const;
		int compareTo(const wchar_t* string) const;

		inline bool operator <(const CString& string) const { return (this->compareTo(string) < 0); }
		inline bool operator <(const wchar_t* string) const { return (this->compareTo(string) < 0); }
		inline bool operator >(const CString& string) const { return (this->compareTo(string) > 0); }
		inline bool operator >(const wchar_t* string) const { return (this->compareTo(string) > 0); }
		inline bool operator <=(const CString& string) const { return (this->compareTo(string) <= 0); }
		inline bool operator <=(const wchar_t* string) const { return (this->compareTo(string) <= 0); }
		inline bool operator >=(const CString& string) const { return (this->compareTo(string) >= 0); }
		inline bool operator >=(const wchar_t* string) const { return (this->compareTo(string) >= 0); }

	// Upper/Lower Case
		CString& toLowerCase();
		CString& toUpperCase();

	// Ignorecase Comparators
		bool equalsIgnoreCase(const CString& string) const;
		bool equalsIgnoreCase(const wchar_t* string) const;

		int compareToIgnoreCase(const CString& string) const;
		int compareToIgnoreCase(const wchar_t* string) const;

	// Start/End Comparators
		bool startsWith(const CString& string, uint32_t start = 0) const;
		bool startsWith(const wchar_t* string, uint32_t start = 0) const;

		bool endsWith(const CString& string) const;
		bool endsWith(const wchar_t* string) const;

	// Search
		int indexOf(wchar_t character) const;
		int indexOf(const CString& string) const;
		int indexOf(const wchar_t* string) const;

		int lastIndexOf(wchar_t character) const;
		int lastIndexOf(const CString& string) const;
		int lastIndexOf(const wchar_t* string) const;

	// Substrings, Chars
		CString substring(int start, int end = -1) const;

		wchar_t& operator [](int index) const throw(const char*); //eOutOfRange
		wchar_t charAt(int index) const throw(const char*); //eOutOfRange

	// Replace
		CString replace(const CString& target, const CString& replacement) const;
		CString replace(const wchar_t* target, const wchar_t* replacement) const;

	// Hash
		inline uint32_t hashCode() const { return (m_data == NULL) ? 0 : mmcrc32(0xFFFFFFFF, m_data, m_size); }
		virtual CString toString() const;
		virtual CString toJSON() const;

		char* toChar(uint32_t* strsize = NULL) const throw(const char*);
		static void freeChar(char* str);

#if defined(__IOS__)
		bool matches(const CString& regularExpression) const;
		bool regionMatches(bool ignoreCase, int thisStart, const CString& string, int start, int length) const;
		bool regionMatches(int thisStart, const CString& string, int start, int length) const;
		
		CString replaceAll(const CString& regularExpression, const CString& replacement) const;
		CString replaceFirst(const CString& regularExpression, const CString& replacement) const;

		//CList<CString> split(const CString& regularExpression, uint32_t limit = 0) const;
#else
	#warning Cross platform JString not complete
#endif
		
		CString& trim();
		
		int vswprintf(const wchar_t* format, va_list arglist);
		
		static CString format(const wchar_t* string, ...);
		
		CString md5() const throw(const char*);

#ifdef DEBUG
		void log() const;
#endif

#if defined(__JNI__)
		jstring toJString(JNIEnv* env);
#endif

	};

#endif

#endif //JAPPSY_USTRING_H
