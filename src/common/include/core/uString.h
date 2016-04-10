/*
 * Created by VipDev on 03.04.16.
 *
 * Copyright (C) 2016 The Jappsy Open Source Project
 *
 * Project Web Page http://jappsy.com
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

#ifndef ANDROID_USTRING_H
#define ANDROID_USTRING_H

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t utf8_strlen(const char* src, uint32_t* strsize);
uint32_t utf8_strlen_nzt(const char* src, uint32_t srcsize, uint32_t* strsize);
uint32_t utf8_toutf16_size(const char* src);
uint32_t utf8_toutf16(const char* src, uint16_t* dst, uint32_t dstsize);
uint32_t utf8_toutf32(const char* src, uint32_t* dst, uint32_t dstsize);
uint32_t utf16_strlen(const uint16_t* src, uint32_t* strsize);
uint32_t utf16_toutf8_size(const uint16_t* src);
uint32_t utf16_toutf8(const uint16_t* src, char* dst, uint32_t dstsize);
uint32_t utf16_toutf32(const uint16_t* src, uint32_t* dst, uint32_t dstsize);
uint32_t utf32_strlen(const uint32_t* src, uint32_t* strsize);
uint32_t utf32_toutf8_size(const uint32_t* src);
uint32_t utf32_toutf8(const uint32_t* src, char* dst, uint32_t dstsize);
uint32_t utf32_toutf16_size(const uint32_t* src);
uint32_t utf32_toutf16(const uint32_t* src, uint16_t* dst, uint32_t dstsize);

#if __WCHAR_MAX__ > 0x10000
    #define wcs_strlen(src, sizeptr)	utf32_strlen((uint32_t*)(src), (sizeptr))
    #define wcs_toutf8_size(src)		utf32_toutf8_size((uint32_t*)(src))
    #define wcs_toutf8(src, dst, size)	utf32_toutf8((uint32_t*)(src), (dst), (size))
    #define utf8_towcs_size(src)		(utf8_strlen((src), 0) * 4)
    #define utf8_towcs(src, dst, size)	utf8_toutf32((src), (uint32_t*)(dst), (size))
#else
    #define wcs_strlen(src, sizeptr)	utf16_strlen((uint16_t*)(src), (sizeptr))
    #define wcs_toutf8_size(src)		utf16_toutf8_size((uint16_t*)(src))
    #define wcs_toutf8(src, dst, size)	utf16_toutf8((uint16_t*)(src), (dst), (size))
    #define utf8_towcs_size(src)		utf8_toutf16_size((src))
    #define utf8_towcs(src, dst, size)	utf8_toutf16((src), (uint16_t*)(dst), (size))
#endif

void uStringInit();
void uStringQuit();

#ifdef __cplusplus
/*
#include <cipher/uCrc.h>

class String;

extern String StringTrue;
extern String StringFalse;
extern String StringNan;
extern String StringInfinite;
extern String StringNegInfinite;
extern String StringEmpty;
extern String StringNull;

#ifndef Char
    #if defined(__JNI__)
        #define Char char
    #else
        #define Char wchar_t
    #endif
#endif

class String {
private:
    Char* lpData = NULL;

    uint32_t strLength = 0;
    uint32_t strSize = 0;
    uint32_t memSize = 0;

    bool resetLength(uint32_t length);
    bool setLength(uint32_t length);

public:
    friend inline String operator +(const String& src1, const String& src2) {
        return String(src1).concat(src2);
    }

    String();
    ~String();

    String(const String& string);
    #if defined(__IOS__)
        String(const NSString* string);
    #endif
    String(const char* string);
    explicit String(const char* string, uint32_t length);
    String(const wchar_t* string);
    explicit String(const wchar_t* string, uint32_t length);
    explicit String(const char character);
    explicit String(const wchar_t character);
    explicit String(bool value);
    String(int8_t value);
    String(uint8_t value);
    String(int16_t value);
    String(uint16_t value);
    String(int32_t value);
    String(uint32_t value);
    String(int64_t value);
    String(uint64_t value);
    String(float value);
    String(double value);

    static inline String valueOf(char value) { return String(value); }
    static inline String valueOf(wchar_t value) { return String(value); }
    static inline String valueOf(bool value) { return String(value); }
    static inline String valueOf(int8_t value) { return String(value); }
    static inline String valueOf(uint8_t value) { return String(value); }
    static inline String valueOf(int16_t value) { return String(value); }
    static inline String valueOf(uint16_t value) { return String(value); }
    static inline String valueOf(int32_t value) { return String(value); }
    static inline String valueOf(uint32_t value) { return String(value); }
    static inline String valueOf(int64_t value) { return String(value); }
    static inline String valueOf(uint64_t value) { return String(value); }
    static inline String valueOf(float value) { return String(value); }
    static inline String valueOf(double value) { return String(value); }

    String& operator =(const String& string);
    #if defined(__IOS__)
        String& operator =(const NSString* string);
    #endif
    String& operator =(const char* string);
    String& operator =(const wchar_t* string);
    String& operator =(const char character);
    String& operator =(const wchar_t character);
    String& operator =(bool value);
    String& operator =(int8_t value);
    String& operator =(uint8_t value);
    String& operator =(int16_t value);
    String& operator =(uint16_t value);
    String& operator =(int32_t value);
    String& operator =(uint32_t value);
    String& operator =(int64_t value);
    String& operator =(uint64_t value);
    String& operator =(float value);
    String& operator =(double value);

    String& operator +=(const String& string);
    #if defined(__IOS__)
        String& operator +=(const NSString* string);
    #endif
    String& operator +=(const char* string);
    String& operator +=(const wchar_t* string);
    String& operator +=(const char character);
    String& operator +=(const wchar_t character);
    String& operator +=(bool value);
    String& operator +=(int8_t value);
    String& operator +=(uint8_t value);
    String& operator +=(int16_t value);
    String& operator +=(uint16_t value);
    String& operator +=(int32_t value);
    String& operator +=(uint32_t value);
    String& operator +=(int64_t value);
    String& operator +=(uint64_t value);
    String& operator +=(float value);
    String& operator +=(double value);

    #if defined(__IOS__)
        operator NSString*() const;
        explicit operator Char*() const;
    #else
        operator Char*() const;
    #endif
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

    String& concat(const String& string);
    #if defined(__IOS__)
        String& concat(const NSString* string);
    #endif
    String& concat(const char* string);
    String& concat(const wchar_t* string);
    String& concat(const char character);
    String& concat(const wchar_t character);
    String& concat(const bool value);
    String& concat(const int8_t value);
    String& concat(const uint8_t value);
    String& concat(const int16_t value);
    String& concat(const uint16_t value);
    String& concat(const int32_t value);
    String& concat(const uint32_t value);
    String& concat(const int64_t value);
    String& concat(const uint64_t value);
    String& concat(const float value);
    String& concat(const double value);

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
};
*/
#endif

#ifdef __cplusplus
}
#endif

#endif //ANDROID_USTRING_H
