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

#include "uMD5.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void md5_init(struct tMD5* md5) {
    md5->count = 0;
    md5->state[0] = 0x67452301;
    md5->state[1] = 0xefcdab89;
    md5->state[2] = 0x98badcfe;
    md5->state[3] = 0x10325476;
    memset(md5->part, 0, 64);
}

#define F(x,y,z) ((((y) ^ (z)) & (x)) ^ (z))
#define G(x,y,z) ((((x) ^ (y)) & (z)) ^ (y))
#define H(x,y,z) ((x) ^ (y) ^ (z))
#define I(x,y,z) (((~(z)) | (x)) ^ (y))

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define FF(a, b, c, d, x, s, ac) { 							\
 (a) += F ((b), (c), (d)) + (x) + (uint32_t)(ac);	\
 (a) = ROTATE_LEFT ((a), (s)); (a) += (b); }

#define GG(a, b, c, d, x, s, ac) {							\
 (a) += G ((b), (c), (d)) + (x) + (uint32_t)(ac);	\
 (a) = ROTATE_LEFT ((a), (s)); (a) += (b); }

#define HH(a, b, c, d, x, s, ac) {							\
 (a) += H ((b), (c), (d)) + (x) + (uint32_t)(ac);	\
 (a) = ROTATE_LEFT ((a), (s)); (a) += (b); }

#define II(a, b, c, d, x, s, ac) {							\
 (a) += I ((b), (c), (d)) + (x) + (uint32_t)(ac);	\
 (a) = ROTATE_LEFT ((a), (s)); (a) += (b); }

void md5_transform(struct tMD5* md5, uint32_t* x) {
    uint32_t a = md5->state[0], b = md5->state[1], c = md5->state[2], d = md5->state[3];

    //__android_log_print(ANDROID_LOG_INFO, "JNI", "MD5 Transform S - %08X %08X %08X %08X", a, b, c, d);
    //__android_log_print(ANDROID_LOG_INFO, "JNI", "MD5 Transform P - %08X %08X %08X %08X", x[0], x[1], x[2], x[3]);

    FF (a, b, c, d, x[ 0], 7, 0xd76aa478); FF (d, a, b, c, x[ 1], 12, 0xe8c7b756);
    FF (c, d, a, b, x[ 2], 17, 0x242070db); FF (b, c, d, a, x[ 3], 22, 0xc1bdceee);
    FF (a, b, c, d, x[ 4], 7, 0xf57c0faf); FF (d, a, b, c, x[ 5], 12, 0x4787c62a);
    FF (c, d, a, b, x[ 6], 17, 0xa8304613); FF (b, c, d, a, x[ 7], 22, 0xfd469501);
    FF (a, b, c, d, x[ 8], 7, 0x698098d8); FF (d, a, b, c, x[ 9], 12, 0x8b44f7af);
    FF (c, d, a, b, x[10], 17, 0xffff5bb1); FF (b, c, d, a, x[11], 22, 0x895cd7be);
    FF (a, b, c, d, x[12], 7, 0x6b901122); FF (d, a, b, c, x[13], 12, 0xfd987193);
    FF (c, d, a, b, x[14], 17, 0xa679438e); FF (b, c, d, a, x[15], 22, 0x49b40821);
    GG (a, b, c, d, x[ 1], 5, 0xf61e2562); GG (d, a, b, c, x[ 6], 9, 0xc040b340);
    GG (c, d, a, b, x[11], 14, 0x265e5a51); GG (b, c, d, a, x[ 0], 20, 0xe9b6c7aa);
    GG (a, b, c, d, x[ 5], 5, 0xd62f105d); GG (d, a, b, c, x[10], 9, 0x02441453);
    GG (c, d, a, b, x[15], 14, 0xd8a1e681); GG (b, c, d, a, x[ 4], 20, 0xe7d3fbc8);
    GG (a, b, c, d, x[ 9], 5, 0x21e1cde6); GG (d, a, b, c, x[14], 9, 0xc33707d6);
    GG (c, d, a, b, x[ 3], 14, 0xf4d50d87); GG (b, c, d, a, x[ 8], 20, 0x455a14ed);
    GG (a, b, c, d, x[13], 5, 0xa9e3e905); GG (d, a, b, c, x[ 2], 9, 0xfcefa3f8);
    GG (c, d, a, b, x[ 7], 14, 0x676f02d9); GG (b, c, d, a, x[12], 20, 0x8d2a4c8a);
    HH (a, b, c, d, x[ 5], 4, 0xfffa3942); HH (d, a, b, c, x[ 8], 11, 0x8771f681);
    HH (c, d, a, b, x[11], 16, 0x6d9d6122); HH (b, c, d, a, x[14], 23, 0xfde5380c);
    HH (a, b, c, d, x[ 1], 4, 0xa4beea44); HH (d, a, b, c, x[ 4], 11, 0x4bdecfa9);
    HH (c, d, a, b, x[ 7], 16, 0xf6bb4b60); HH (b, c, d, a, x[10], 23, 0xbebfbc70);
    HH (a, b, c, d, x[13], 4, 0x289b7ec6); HH (d, a, b, c, x[ 0], 11, 0xeaa127fa);
    HH (c, d, a, b, x[ 3], 16, 0xd4ef3085); HH (b, c, d, a, x[ 6], 23, 0x04881d05);
    HH (a, b, c, d, x[ 9], 4, 0xd9d4d039); HH (d, a, b, c, x[12], 11, 0xe6db99e5);
    HH (c, d, a, b, x[15], 16, 0x1fa27cf8); HH (b, c, d, a, x[ 2], 23, 0xc4ac5665);
    II (a, b, c, d, x[ 0], 6, 0xf4292244); II (d, a, b, c, x[ 7], 10, 0x432aff97);
    II (c, d, a, b, x[14], 15, 0xab9423a7); II (b, c, d, a, x[ 5], 21, 0xfc93a039);
    II (a, b, c, d, x[12], 6, 0x655b59c3); II (d, a, b, c, x[ 3], 10, 0x8f0ccc92);
    II (c, d, a, b, x[10], 15, 0xffeff47d); II (b, c, d, a, x[ 1], 21, 0x85845dd1);
    II (a, b, c, d, x[ 8], 6, 0x6fa87e4f); II (d, a, b, c, x[15], 10, 0xfe2ce6e0);
    II (c, d, a, b, x[ 6], 15, 0xa3014314); II (b, c, d, a, x[13], 21, 0x4e0811a1);
    II (a, b, c, d, x[ 4], 6, 0xf7537e82); II (d, a, b, c, x[11], 10, 0xbd3af235);
    II (c, d, a, b, x[ 2], 15, 0x2ad7d2bb); II (b, c, d, a, x[ 9], 21, 0xeb86d391);
    md5->state[0] += a; md5->state[1] += b; md5->state[2] += c; md5->state[3] += d;

    //__android_log_print(ANDROID_LOG_INFO, "JNI", "MD5 Transform S - %08X %08X %08X %08X", a, b, c, d);
}

void md5_update(struct tMD5* md5, uint8_t* buffer, uint32_t size) {
    uint32_t index = (((uint32_t)(md5->count)) >> 3) & 0x3F; // bytes mod 64
    int64_t sizeBits = size; sizeBits <<= 3;
    md5->count += sizeBits;
    uint32_t partLen = index; partLen ^= 0x3F; ++partLen;

    //__android_log_print(ANDROID_LOG_INFO, "JNI", "MD5 - Size: %d, Index: %d, Bits: %lld, Fill: %d", size, index, sizeBits, partLen);

    if (size >= partLen) {
        memcpy((md5->part)+index, buffer, partLen); buffer += partLen;
        md5_transform(md5, (uint32_t*)(md5->part));
        size -= partLen; index = size >> 6; size &= 0x3F;
        while (index > 0) {
            //__android_log_print(ANDROID_LOG_INFO, "JNI", "MD5 Loop - Size: %d, Index: %d", size, index);

            md5_transform(md5, (uint32_t*)buffer); buffer += 64; --index;
        }
    }
    if (size > 0)
        memcpy((md5->part) + index, buffer, size);
}

static uint32_t MD5_PADDING[16] =
        { 0x00000080, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void md5_final(struct tMD5* md5) {
    uint32_t index = (((uint32_t)(md5->count)) >> 3) & 0x3F; // bytes mod 64
    uint32_t padLen = (index < 56) ? (56 - index) : (120 - index);
    int64_t cnt = (md5->count);

    //__android_log_print(ANDROID_LOG_INFO, "JNI", "MD5 Final - Index: %d, Padding: %d, Count: %lld", index, padLen, cnt);

    md5_update(md5, (uint8_t*)MD5_PADDING, padLen);
    md5_update(md5, (uint8_t*)&cnt, 8);
    md5->count = 0;
    memset(md5->part, 0, 64);
}

#if defined(__JNI__)

JNIEXPORT jstring JNICALL
Java_com_jappsy_cipher_MD5_unicode(JNIEnv *env, jclass type, jstring s_) {
    struct tMD5 md5;
    md5_init(&md5);

    jsize size = (*env)->GetStringLength(env, s_) * 2;

    //__android_log_print(ANDROID_LOG_INFO, "JNI", "Size: %d", size);

    const jchar* buffer = (*env)->GetStringCritical(env, s_, NULL);
    if (buffer != NULL) {
        if (size > 0)
            md5_update(&md5, (uint8_t*)buffer, (uint32_t)size);
        (*env)->ReleaseStringCritical(env, s_, buffer);
    }

    md5_final(&md5);

    static jchar res[32];
    uint8_t* md = (uint8_t*)(md5.state);
    int i;
    for (i = 15; i >= 0; i--) {
        uint8_t ch = md[i];
        uint8_t chu = ch >> 4;
        uint8_t chl = (uint8_t)(ch & 0x0F);
        chu = (chu >= 10) ? (uint8_t)(chu + 'a' - 10) : (uint8_t)(chu + '0');
        chl = (chl >= 10) ? (uint8_t)(chl + 'a' - 10) : (uint8_t)(chl + '0');
        res[i * 2] = (jchar)chu;
        res[i * 2 + 1] = (jchar)chl;
    }

    return (*env)->NewString(env, res, 32);
}

JNIEXPORT jstring JNICALL
Java_com_jappsy_cipher_MD5_utf8(JNIEnv *env, jclass type, jstring s_) {
    struct tMD5 md5;
    md5_init(&md5);

    jsize size = (*env)->GetStringUTFLength(env, s_);

    //__android_log_print(ANDROID_LOG_INFO, "JNI", "Size: %d", size);

    const char *buffer = (*env)->GetStringUTFChars(env, s_, 0);
    if (buffer != NULL) {
        if (size > 0)
            md5_update(&md5, (uint8_t*)buffer, (uint32_t)size);
        (*env)->ReleaseStringUTFChars(env, s_, buffer);
    }

    md5_final(&md5);

    jchar res[32];
    uint8_t* md = (uint8_t*)(md5.state);
    int i;
    for (i = 15; i >= 0; i--) {
        uint8_t ch = md[i];
        uint8_t chu = ch >> 4;
        uint8_t chl = (uint8_t)(ch & 0x0F);
        chu = (chu >= 10) ? (uint8_t)(chu + 'a' - 10) : (uint8_t)(chu + '0');
        chl = (chl >= 10) ? (uint8_t)(chl + 'a' - 10) : (uint8_t)(chl + '0');
        res[i * 2] = (jchar)chu;
        res[i * 2 + 1] = (jchar)chl;
    }

    return (*env)->NewString(env, res, 32);
}

#endif

#ifdef __cplusplus
}
#endif
