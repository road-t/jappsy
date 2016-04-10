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
            int skip = 0;
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

void uStringInit() {

}

void uStringQuit() {

}

#ifdef __cplusplus
}
#endif
