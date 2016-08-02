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

#ifndef USTRING_H
#define USTRING_H

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

	void uStringInit();
	void uStringQuit();

#ifdef __cplusplus

	#include <cipher/uCrc.h>

	class String;

	#ifndef Char
		#if defined(__JNI__)
			// All JNI Strings UTF8 Encoded
			#define Char char
		#else
			#define Char wchar_t
		#endif
	#endif

	/**
	 * Get string length
	 * @return length
	 */
	static uint32_t getStringLength(const String& self);

	/**
	 * Set string length without null character
	 * @exception eOutOfMemory
	 */
	static uint32_t setStringLength(String& self, uint32_t length) throw(const char*);

	/**
	 * Java like string class for C++
	 */
	class String {
	private:
		friend uint32_t getStringLength(const String& self);
		friend uint32_t setStringLength(String& self, uint32_t length) throw(const char*);

		inline void create();
		inline void release();

		// UTF16 encoded null terminated string pointer
		wchar_t* m_data = NULL;
		// String characters count without null character
		uint32_t m_length = 0;
		// String memory size with null character
		uint32_t m_size = 0;
		// Aligned string buffer memory size
		uint32_t m_memorySize = 0;

		/**
		 * Set string memory size with null character
		 * @exception eOutOfMemory
		 */
		inline void setSize(uint32_t size) throw(const char*);

		/**
		 * Get string length
		 * @return length
		 */
		inline uint32_t getLength() { return m_length; }

		/**
		 * Set string length without null character
		 * @exception eOutOfMemory
		 */
		inline void setLength(uint32_t length) throw(const char*);

		static inline int swprintf(wchar_t* target, int8_t value);
		static inline int swprintf(wchar_t* target, uint8_t value);
		static inline int swprintf(wchar_t* target, int16_t value);
		static inline int swprintf(wchar_t* target, uint16_t value);
		static inline int swprintf(wchar_t* target, int32_t value);
		static inline int swprintf(wchar_t* target, uint32_t value);
		static inline int swprintf(wchar_t* target, int64_t value);
		static inline int swprintf(wchar_t* target, uint64_t value);
		static inline int swprintf(wchar_t* target, float value);
		static inline int swprintf(wchar_t* target, double value);

		static int getStringNumberFormat(wchar_t* src, uint32_t srcLen, uint32_t* pos, uint32_t* len);
		inline static int64_t wtoll(const wchar_t* data, uint32_t len, uint32_t type) throw(const char*); // eConvert
		inline static long double wtod(const wchar_t* data, uint32_t len, uint32_t type) throw(const char*); // eConvert
	public:
		/**
		 * new String()
		 * @throws eOutOfMemory
 		 */
		void* operator new(size_t) throw(const char*);

		/**
		 * new String[]()
		 * @throws eOutOfMemory
 		 */
		void* operator new[](size_t) throw(const char*);

		void operator delete(void*);
		void operator delete[](void*);

		/*
		friend inline String operator +(const String& src1, const String& src2) {
			return String(src1).concat(src2);
		}
		*/

		Property<String, uint32_t> length;

		/**
		 * Create NULL string
		 */
		String();

		/**
		 * Create NULL string
		 * @param string  must be NULL
		 * @throws eInvalidPointer
		 */
		explicit String(const void* string) throw(const char*);

		/**
		 * Create copy of string
		 * @param string  source string
		 * @throws eOutOfMemory
		 */
		String(const String& string) throw(const char*);
		#if defined(__IOS__)
			String(const NSString* string) throw(const char*); // eOutOfMemory
		#endif
		String(const char* string) throw(const char*); // eOutOfMemory
		explicit String(const char* string, uint32_t length) throw(const char*); // eOutOfMemory
		String(const wchar_t* string) throw(const char*); // eOutOfMemory
		explicit String(const wchar_t* string, uint32_t length) throw(const char*); // eOutOfMemory
		explicit String(const char character) throw(const char*); // eOutOfMemory
		explicit String(const wchar_t character) throw(const char*); // eOutOfMemory
		explicit String(bool value) throw(const char*); // eOutOfMemory
		String(int8_t value) throw(const char*); // eOutOfMemory
		String(uint8_t value) throw(const char*); // eOutOfMemory
		String(int16_t value) throw(const char*); // eOutOfMemory
		String(uint16_t value) throw(const char*); // eOutOfMemory
		String(int32_t value) throw(const char*); // eOutOfMemory
		String(uint32_t value) throw(const char*); // eOutOfMemory
		String(int64_t value) throw(const char*); // eOutOfMemory
		String(uint64_t value) throw(const char*); // eOutOfMemory
		String(float value) throw(const char*); // eOutOfMemory
		String(double value) throw(const char*); // eOutOfMemory
		~String();

		static inline String valueOf(char value) throw(const char*) { return String(value); } // eOutOfMemory
		static inline String valueOf(wchar_t value) throw(const char*) { return String(value); } // eOutOfMemory
		static inline String valueOf(bool value) throw(const char*) { return String(value); } // eOutOfMemory
		static inline String valueOf(int8_t value) throw(const char*) { return String(value); } // eOutOfMemory
		static inline String valueOf(uint8_t value) throw(const char*) { return String(value); } // eOutOfMemory
		static inline String valueOf(int16_t value) throw(const char*) { return String(value); } // eOutOfMemory
		static inline String valueOf(uint16_t value) throw(const char*) { return String(value); } // eOutOfMemory
		static inline String valueOf(int32_t value) throw(const char*) { return String(value); } // eOutOfMemory
		static inline String valueOf(uint32_t value) throw(const char*) { return String(value); } // eOutOfMemory
		static inline String valueOf(int64_t value) throw(const char*) { return String(value); } // eOutOfMemory
		static inline String valueOf(uint64_t value) throw(const char*) { return String(value); } // eOutOfMemory
		static inline String valueOf(float value) throw(const char*) { return String(value); } // eOutOfMemory
		static inline String valueOf(double value) throw(const char*) { return String(value); } // eOutOfMemory

		String& operator =(const void* string) throw(const char*); // eOutOfMemory
		String& operator =(const String& string) throw(const char*); // eOutOfMemory
		#if defined(__IOS__)
			String& operator =(const NSString* string) throw(const char*); // eOutOfMemory
		#endif
		String& operator =(const char* string) throw(const char*); // eOutOfMemory
		String& operator =(const wchar_t* string) throw(const char*); // eOutOfMemory
		String& operator =(const char character) throw(const char*); // eOutOfMemory
		String& operator =(const wchar_t character) throw(const char*); // eOutOfMemory
		inline String& operator =(bool value) throw(const char*); // eOutOfMemory
		String& operator =(int8_t value) throw(const char*); // eOutOfMemory
		String& operator =(uint8_t value) throw(const char*); // eOutOfMemory
		String& operator =(int16_t value) throw(const char*); // eOutOfMemory
		String& operator =(uint16_t value) throw(const char*); // eOutOfMemory
		String& operator =(int32_t value) throw(const char*); // eOutOfMemory
		String& operator =(uint32_t value) throw(const char*); // eOutOfMemory
		String& operator =(int64_t value) throw(const char*); // eOutOfMemory
		String& operator =(uint64_t value) throw(const char*); // eOutOfMemory
		String& operator =(float value) throw(const char*); // eOutOfMemory
		String& operator =(double value) throw(const char*); // eOutOfMemory

		inline String& operator +=(const String& string) throw(const char*) { return this->concat(string); }; // eOutOfMemory
		#if defined(__IOS__)
			inline String& operator +=(const NSString* string) throw(const char*) { return this->concat(string); }; // eOutOfMemory
		#endif
		inline String& operator +=(const char* string) throw(const char*) { return this->concat(string); }; // eOutOfMemory
		inline String& operator +=(const wchar_t* string) throw(const char*) { return this->concat(string); }; // eOutOfMemory
		inline String& operator +=(const char character) throw(const char*) { return this->concat(character); }; // eOutOfMemory
		inline String& operator +=(const wchar_t character) throw(const char*) { return this->concat(character); }; // eOutOfMemory
		inline String& operator +=(bool value) throw(const char*) { return this->concat(value); }; // eOutOfMemory
		inline String& operator +=(int8_t value) throw(const char*) { return this->concat(value); }; // eOutOfMemory
		inline String& operator +=(uint8_t value) throw(const char*) { return this->concat(value); }; // eOutOfMemory
		inline String& operator +=(int16_t value) throw(const char*) { return this->concat(value); }; // eOutOfMemory
		inline String& operator +=(uint16_t value) throw(const char*) { return this->concat(value); }; // eOutOfMemory
		inline String& operator +=(int32_t value) throw(const char*) { return this->concat(value); }; // eOutOfMemory
		inline String& operator +=(uint32_t value) throw(const char*) { return this->concat(value); }; // eOutOfMemory
		inline String& operator +=(int64_t value) throw(const char*) { return this->concat(value); }; // eOutOfMemory
		inline String& operator +=(uint64_t value) throw(const char*) { return this->concat(value); }; // eOutOfMemory
		inline String& operator +=(float value) throw(const char*) { return this->concat(value); }; // eOutOfMemory
		inline String& operator +=(double value) throw(const char*) { return this->concat(value); }; // eOutOfMemory

		#if defined(__IOS__)
			explicit operator NSString*() const;
		#endif
		explicit inline operator wchar_t*() const { return this->m_data; };
		explicit operator bool() const;
		explicit operator int8_t() const throw(const char*); // eConvert
		explicit operator uint8_t() const;
		explicit operator int16_t() const;
		explicit operator uint16_t() const;
		explicit operator int32_t() const;
		explicit operator uint32_t() const;
		explicit operator int64_t() const;
		explicit operator uint64_t() const;
		explicit operator float() const;
		explicit operator double() const;

		inline double operator +() const { return this->operator double(); }
		inline double operator -() const { return -(this->operator double()); }

		//friend int8_t operator +(int8_t value1, const String& value2);

/*
		bool operator ==(const String& src) const;
		bool operator ==(const wchar_t* src) const;
		bool operator ==(const char* src) const;

		inline bool operator !=(const String& src) const { return !(this->operator==(src)); }
		inline bool operator !=(const wchar_t* src) const { return !(this->operator==(src)); }
		inline bool operator !=(const char* src) const { return !(this->operator==(src)); }

		bool operator <(const String& src) const;
		bool operator <(const wchar_t* src) const;
		bool operator >(const String& src) const;
		bool operator >(const wchar_t* src) const;
		bool operator <=(const String& src) const;
		bool operator <=(const wchar_t* src) const;
		bool operator >=(const String& src) const;
		bool operator >=(const wchar_t* src) const;
*/

		String& concat(const String& string) throw(const char*); // eOutOfMemory
		#if defined(__IOS__)
			String& concat(const NSString* string) throw(const char*); // eOutOfMemory
		#endif
		// Convert value to String
		// Throw eOutOfMemory
		String& concat(const char* string) throw(const char*); // eOutOfMemory
		String& concat(const wchar_t* string) throw(const char*); // eOutOfMemory
		String& concat(const char character) throw(const char*); // eOutOfMemory
		String& concat(const wchar_t character) throw(const char*); // eOutOfMemory
		String& concat(const bool value) throw(const char*); // eOutOfMemory
		String& concat(const int8_t value) throw(const char*); // eOutOfMemory
		String& concat(const uint8_t value) throw(const char*); // eOutOfMemory
		String& concat(const int16_t value) throw(const char*); // eOutOfMemory
		String& concat(const uint16_t value) throw(const char*); // eOutOfMemory
		String& concat(const int32_t value) throw(const char*); // eOutOfMemory
		String& concat(const uint32_t value) throw(const char*); // eOutOfMemory
		String& concat(const int64_t value) throw(const char*); // eOutOfMemory
		String& concat(const uint64_t value) throw(const char*); // eOutOfMemory
		String& concat(const float value) throw(const char*); // eOutOfMemory
		String& concat(const double value) throw(const char*); // eOutOfMemory
/*
		bool equals(const String& string) const;
		bool equals(const wchar_t* string) const;

		int compareTo(const String& string) const;
		int compareTo(const wchar_t* string) const;

		String& toLowerCase();
		String& toUpperCase();

		bool equalsIgnoreCase(const String& string) const;
		bool equalsIgnoreCase(const wchar_t* string) const;

		int compareToIgnoreCase(const String& string) const;
		int compareToIgnoreCase(const wchar_t* string) const;

		bool startsWith(const String& string, uint32_t start = 0) const;
		bool startsWith(const wchar_t* string, uint32_t start = 0) const;

		bool endsWith(const String& string) const;
		bool endsWith(const wchar_t* string) const;

		int indexOf(wchar_t character) const;
		int indexOf(const String& string) const;
		int indexOf(const wchar_t* string) const;

		int lastIndexOf(wchar_t character) const;
		int lastIndexOf(const String& string) const;
		int lastIndexOf(const wchar_t* string) const;

		String substring(int start, int end = -1) const;

		String replace(const String& target, const String& replacement) const;
		String replace(const wchar_t* target, const wchar_t* replacement) const;

		wchar_t charAt(int index) const;

		inline bool isEmpty() const { return (strLength == 0); }

		bool isNumber() const;

		inline uint32_t length() { return strLength; }

		String toString() const;
		#if defined(__IOS__)
			NSString* toNSString() const;
		#endif
		#if defined(__JNI__)
			inline char* toChar() const { return lpData; }
		#else
			inline wchar_t* toChar() const { return lpData; }
		#endif

		bool matches(const String& regularExpression) const;
		bool regionMatches(bool ignoreCase, int thisStart, const String& string, int start, int length) const;
		bool regionMatches(int thisStart, const String& string, int start, int length) const;

		String replaceAll(const String& regularExpression, const String& replacement) const;
		String replaceFirst(const String& regularExpression, const String& replacement) const;

		String& trim();

		inline uint32_t hashCode() const {
			if (lpData == NULL) {
				return 0;
			}
			return mmcrc32(0xFFFFFFFF, lpData, strSize);
		}

		String md5() const;

		//List<String> split(const String& regularExpression, uint32_t limit = 0) const;

		static String format(const char* string, ...);
		static String format(const wchar_t* string, ...);
		static String format(const String& string, ...);

		//int vsprintf(const char* format, va_list arglist);
		//int vswprintf(const wchar_t* format, va_list arglist);
		*/
	};

#endif

#ifdef __cplusplus
}
#endif

#endif //ANDROID_USTRING_H
