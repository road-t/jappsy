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
                else if (ch <= 0xFFFFF) { // 4 byte UTF16
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
                else if (ch <= 0xFFFFF) { // 4 byte UTF16
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

#include <core/uMemory.h>

#define StringTrue L"true"
#define StringFalse L"false"
#define StringNan L"NAN"
#define StringInfinite L"INFINITE"
#define StringNegInfinite L"-INFINITE"
#define StringEmpty L""
#define StringNil NULL
#define StringNull L"(null)"

void String::create() {
	this->length.initialize(this, &(this->getLength), &(this->setLength));
}

void String::release() {
}

void* String::operator new(size_t size) throw(const char*) {
    void *p = memAlloc(void, p, size);
    if (!p) throw "Error: Out of memory ( String::operator new() )";
    return p;
}

void* String::operator new[](size_t size) throw(const char*) {
    void *p = memAlloc(void, p, size);
    if (!p) throw "Error: Out of memory ( String::operator new[]() )";
    return p;
}

void String::operator delete(void *p) {
    memFree(p);
}

void String::operator delete[](void *p) {
    memFree(p);
}

#define STRING_BLOCK_SIZE	32

uint32_t String::setSize(uint32_t size) throw(const char*) {
	uint32_t newSize = size;
	uint32_t newLength = (newSize / sizeof(wchar_t)) - 1;

	if (newLength < 0) {
		if (this->m_data != NULL) {
			memFree(this->m_data);
			this->m_data = NULL;
		}
		this->m_length = 0;
		this->m_size = 0;
		this->m_memorySize = 0;
		return 0;
	}

    uint32_t newMemSize = newSize - (newSize % STRING_BLOCK_SIZE) + STRING_BLOCK_SIZE;
    if (this->m_memorySize != newMemSize) {
        wchar_t* newString = memRealloc(wchar_t, newString, this->m_data, newMemSize);
        if (newString) {
            this->m_data = newString;
            this->m_data[newLength] = 0;
            this->m_length = newLength;
            this->m_size = newSize;
            this->m_memorySize = newMemSize;
            return newSize;
        } else {
			throw "Error: Out of memory ( String::setSize() )";
        }
    }

	this->m_data[newLength] = 0;
	this->m_length = newLength;
	this->m_size = newSize;
	return newSize;
}

uint32_t String::setLength(uint32_t length) throw(const char*) {
    uint32_t newSize = (length + 1) * sizeof(wchar_t);
    uint32_t newMemSize = newSize - (newSize % STRING_BLOCK_SIZE) + STRING_BLOCK_SIZE;
    if (this->m_memorySize != newMemSize) {
        wchar_t* newString = memRealloc(wchar_t, newString, this->m_data, newMemSize);
        if (newString) {
            this->m_data = newString;
            this->m_data[length] = 0;
            this->m_length = length;
            this->m_size = newSize;
            this->m_memorySize = newMemSize;
            return length;
        } else {
			throw "Error: Out of memory ( String::setLength() )";
        }
    }

	this->m_data[length] = 0;
	this->m_length = length;
	this->m_size = newSize;
	return length;
}

String::String() {
	this->create();
}

String::String(const void* string) throw(const char*) {
	this->create();
	if (string != NULL) {
		throw "Error: Invalid pointer ( String::String(void*) )";
	}
}

String& String::operator =(const void* string) throw(const char*) {
	if (string != NULL) {
		throw "Error: Invalid pointer ( String::String(void*) )";
	}
	this->setSize(0);
	return *this;
}

String::String(const String& string) throw(const char*) {
	this->create();
	if (string.m_size > 0) {
		this->setSize(string.m_size);
		memcpy(this->m_data, string.m_data, this->m_size);
	}
}

String& String::operator =(const String& string) throw(const char*) {
	this->setSize(string.m_size);
	if (string.m_size > 0)
		memcpy(this->m_data, string.m_data, this->m_size);
	return *this;
}

String& String::concat(const String& string) throw(const char*) {
	uint32_t length = string.m_length;
	if (length != 0) {
		uint32_t prevLength = this->m_length;
		setLength(prevLength + length);
		memcpy(this->m_data + prevLength, string.m_data, string.m_size);
	}
    return *this;
}

String& String::operator +=(const String& string) throw(const char*) {
    if (this->m_data == NULL)
        return this->operator=(string);
    else
		return this->concat(string);
}

#if defined(__IOS__)
	String::String(const NSString* string) throw(const char*) {
		this->create();
		if (string != nil) {
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

	String& String::operator =(const NSString* string) throw(const char*) {
		if (string != nil) {
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

	String& String::concat(const NSString* string) throw(const char*) {
		if (string != nil) {
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

	String& String::operator +=(const NSString* string) throw(const char*) {
		if (this->m_data == NULL)
			return this->operator=(string);
		else
			return this->concat(string);
	}

#endif

String::String(const char* string) throw(const char*) {
	this->create();
	if (string != NULL) {
		uint32_t newSize = 0;
		uint32_t length = utf8_strlen(string, &newSize);
		if (length > 0) {
			this->setSize(newSize);
			utf8_towcs(string, this->m_data, this->m_size);
		} else {
			this->setLength(0);
		}
	}
}

String& String::operator =(const char* string) throw(const char*) {
	if (string != NULL) {
		uint32_t newSize = 0;
		uint32_t length = utf8_strlen(string, &newSize);
		if (length > 0) {
			this->setSize(newSize);
			utf8_towcs(string, this->m_data, this->m_size);
		} else {
			this->setLength(0);
		}
	} else {
		this->setSize(0);
	}
	return *this;
}

String& String::concat(const char* string) throw(const char*) {
    if (string != NULL) {
        uint32_t prevLength = this->m_length;
        uint32_t strSize;
        uint32_t length = utf8_strlen(string, &strSize);
        if (length > 0) {
			this->setLength(prevLength + length);
            utf8_towcs(string, this->m_data + prevLength, strSize);
        }
    }
    return *this;
}

String& String::operator +=(const char* string) throw(const char*) {
	if (this->m_data == NULL)
		return this->operator=(string);
	else
		return this->concat(string);
}

String::String(const char* string, uint32_t length) throw(const char*) {
	this->create();
	if (string != NULL) {
		this->setLength(length);
		if (length > 0) {
			wchar_t* dst = this->m_data;
			char* src = (char*)string;
			do {
				(*dst) = (wchar_t)(*src);
				length--;
			} while (length > 0);
		}
	}
}

String::String(const wchar_t* string) throw(const char*) {
	this->create();
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

String& String::operator =(const wchar_t* string) throw(const char*) {
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

String& String::concat(const wchar_t* string) throw(const char*) {
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

String& String::operator +=(const wchar_t* string) throw(const char*) {
	if (this->m_data == NULL)
		return this->operator=(string);
	else
		return this->concat(string);
}

String::String(const wchar_t* string, uint32_t length) throw(const char*) {
	this->create();
	if (string != NULL) {
		this->setLength(length);
		if (length > 0)
			memcpy(this->m_data, string, length * sizeof(wchar_t));
	}
}

String::String(const char character) throw(const char*) {
	this->create();
	this->setLength(1);
	this->m_data[0] = (wchar_t)character;
}

String& String::operator =(const char character) throw(const char*) {
	this->setLength(1);
	this->m_data[0] = (wchar_t)character;
	return *this;
}

String& String::concat(const char character) throw(const char*) {
    uint32_t prevLength = this->m_length;
    this->setLength(prevLength + 1);
	this->m_data[prevLength] = (wchar_t)character;
    return *this;
}

String& String::operator +=(const char character) throw(const char*) {
	if (this->m_data == NULL)
		return this->operator=(character);
	else
		return this->concat(character);
}

String::String(const wchar_t character) throw(const char*) {
	this->create();
	this->setLength(1);
	this->m_data[0] = character;
}

String& String::operator =(const wchar_t character) throw(const char*) {
	this->setLength(1);
	this->m_data[0] = character;
	return *this;
}

String& String::concat(const wchar_t character) throw(const char*) {
    uint32_t prevLength = this->m_length;
    this->setLength(prevLength + 1);
	this->m_data[prevLength] = character;
    return *this;
}

String& String::operator +=(const wchar_t character) throw(const char*) {
	if (this->m_data == NULL)
		return this->operator=(character);
	else
		return this->concat(character);
}

String::String(bool value) throw(const char*) {
	this->create();
	this->operator =(value);
}

String& String::operator =(bool value) throw(const char*) {
	if (value)
		this->operator =(StringTrue);
	else
		this->operator =(StringFalse);
	return *this;
}

int String::swprintf(wchar_t* target, int8_t value) {
	return ::swprintf(target, 4, L"%hhd", value);
}

String::String(int8_t value) throw(const char*) {
	this->create();
    this->setLength(4);
	int length = String::swprintf(this->m_data, value);
	if (length != 4) {
		this->setLength(static_cast<uint32_t>(length));
    }
}

String& String::operator =(int8_t value) throw(const char*) {
    this->setLength(4);
	int length = String::swprintf(this->m_data, value);
	if (length != 4) {
		this->setLength(static_cast<uint32_t>(length));
    }
    return *this;
}

int String::swprintf(wchar_t* target, uint8_t value) {
	return ::swprintf(target, 3, L"%hhu", value);
}

String::String(uint8_t value) throw(const char*) {
	this->create();
    this->setLength(3);
	int length = String::swprintf(this->m_data, value);
	if (length != 3) {
		this->setLength(static_cast<uint32_t>(length));
    }
}

String& String::operator =(uint8_t value) throw(const char*) {
    this->setLength(3);
	int length = String::swprintf(this->m_data, value);
	if (length != 3) {
		this->setLength(static_cast<uint32_t>(length));
    }
    return *this;
}

int String::swprintf(wchar_t* target, int16_t value) {
	return ::swprintf(target, 6, L"%hd", value);
}

String::String(int16_t value) throw(const char*) {
	this->create();
    this->setLength(6);
	int length = String::swprintf(this->m_data, value);
	if (length != 6) {
		this->setLength(static_cast<uint32_t>(length));
    }
}

String& String::operator =(int16_t value) throw(const char*) {
    this->setLength(6);
	int length = String::swprintf(this->m_data, value);
	if (length != 6) {
		this->setLength(static_cast<uint32_t>(length));
    }
    return *this;
}

int String::swprintf(wchar_t* target, uint16_t value) {
	return ::swprintf(target, 5, L"%hu", value);
}

String::String(uint16_t value) throw(const char*) {
	this->create();
    this->setLength(5);
	int length = String::swprintf(this->m_data, value);
	if (length != 5) {
		this->setLength(static_cast<uint32_t>(length));
    }
}

String& String::operator =(uint16_t value) throw(const char*) {
    this->setLength(5);
	int length = String::swprintf(this->m_data, value);
	if (length != 5) {
		this->setLength(static_cast<uint32_t>(length));
    }
    return *this;
}

int String::swprintf(wchar_t* target, int32_t value) {
	return ::swprintf(target, 11, L"%ld", value);
}

String::String(int32_t value) throw(const char*) {
	this->create();
    this->setLength(11);
	int length = String::swprintf(this->m_data, value);
	if (length != 11) {
		this->setLength(static_cast<uint32_t>(length));
    }
}

String& String::operator =(int32_t value) throw(const char*) {
    this->setLength(11);
	int length = String::swprintf(this->m_data, value);
	if (length != 11) {
		this->setLength(static_cast<uint32_t>(length));
    }
    return *this;
}

int String::swprintf(wchar_t* target, uint32_t value) {
	return ::swprintf(target, 10, L"%lu", value);
}

String::String(uint32_t value) throw(const char*) {
	this->create();
    this->setLength(10);
	int length = String::swprintf(this->m_data, value);
	if (length != 10) {
		this->setLength(static_cast<uint32_t>(length));
    }
}

String& String::operator =(uint32_t value) throw(const char*) {
    this->setLength(10);
	int length = String::swprintf(this->m_data, value);
	if (length != 10) {
		this->setLength(static_cast<uint32_t>(length));
    }
    return *this;
}

int String::swprintf(wchar_t* target, int64_t value) {
	return ::swprintf(target, 21, L"%lld", value);
}

String::String(int64_t value) throw(const char*) {
	this->create();
    this->setLength(21);
	int length = String::swprintf(this->m_data, value);
	if (length != 21) {
		this->setLength(static_cast<uint32_t>(length));
    }
}

String& String::operator =(int64_t value) throw(const char*) {
    this->setLength(21);
	int length = String::swprintf(this->m_data, value);
	if (length != 21) {
		this->setLength(static_cast<uint32_t>(length));
    }
    return *this;
}

int String::swprintf(wchar_t* target, uint64_t value) {
	return ::swprintf(target, 20, L"%llu", value);
}

String::String(uint64_t value) throw(const char*) {
	this->create();
    this->setLength(20);
	int length = String::swprintf(this->m_data, value);
	if (length != 20) {
		this->setLength(static_cast<uint32_t>(length));
    }
}

String& String::operator =(uint64_t value) throw(const char*) {
    this->setLength(20);
	int length = String::swprintf(this->m_data, value);
	if (length != 20) {
		this->setLength(static_cast<uint32_t>(length));
    }
    return *this;
}

#ifndef __IOS__
	#include <math.h>
#endif

int String::swprintf(wchar_t* target, float value) {
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

String::String(float value) throw(const char*) {
	this->create();
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
        int length = String::swprintf(this->m_data, value);
        if (length == EOF)
			throw "Error: swprintf ( String::String(float) )";
        else if (length != 23)
			this->setLength(static_cast<uint32_t>(length));
    }
}

String& String::operator =(float value) throw(const char*) {
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
        int length = String::swprintf(this->m_data, value);
        if (length == EOF)
			throw "Error: swprintf ( String::operator =(float) )";
		else if (length != 23)
			this->setLength(static_cast<uint32_t>(length));
    }
    return *this;
}

int String::swprintf(wchar_t* target, double value) {
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

String::String(double value) throw(const char*) {
	this->create();
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
        int length = String::swprintf(this->m_data, value);
		if (length == EOF)
			throw "Error: swprintf ( String::String(double) )";
		else if (length != 31)
			this->setLength(static_cast<uint32_t>(length));
    }
}

String& String::operator =(double value) throw(const char*) {
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
        int length = String::swprintf(this->m_data, value);
		if (length == EOF)
			throw "Error: swprintf ( String::operator =(double) )";
		else if (length != 31)
			this->setLength(static_cast<uint32_t>(length));
    }
    return *this;
}

String::~String() {
	this->release();
}

//==============================================================

void uStringInit() {

}

void uStringQuit() {

}

#ifdef __cplusplus
}
#endif
