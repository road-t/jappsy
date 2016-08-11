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

#include "uCipher.h"
#if defined(__IOS__)
	#include <libz/zlib.h>
#else
	#include <zlib.h>
#endif
#include "../core/uMemory.h"

#define GZIP_MAGIC 0x8b1f

bool is_gzip(const void* in, uint32_t insize) {
    if ((in == NULL) || (insize < 2))
        return false;

    return (*(uint16_t*)in == GZIP_MAGIC);
}

void* gzip_encode_fast(const void* in, uint32_t insize, uint32_t* outsize) {
    uint32_t outSize = 0;
    uint8_t* outBuffer = 0;

    if (in != 0) {
        z_stream gzip;
        gzip.zalloc = Z_NULL;
        gzip.zfree = Z_NULL;
        gzip.opaque = Z_NULL;
        gzip.total_out = 0;
        gzip.next_in = (Bytef*)in;
        gzip.avail_in = insize;

        int res = deflateInit2(&gzip, Z_BEST_SPEED, Z_DEFLATED, (15+16), 8, Z_DEFAULT_STRATEGY);
        if (res == Z_OK) {
            uint32_t gzipBufferSize = insize + (insize / 100) + 12;
            Bytef* gzipBuffer = memAlloc(Bytef, gzipBuffer, gzipBufferSize);

            do {
                gzip.next_out = gzipBuffer + gzip.total_out;
                gzip.avail_out = (uInt)(gzipBufferSize - gzip.total_out);
                res = deflate(&gzip, Z_FINISH);
            } while (res == Z_OK);

            deflateEnd(&gzip);

            if ((res != Z_STREAM_END) || (gzip.total_out > gzip.total_in)) {
                memFree(gzipBuffer);
            } else {
                outSize = (uint32_t)gzip.total_out;
                outBuffer = gzipBuffer;
            }
        }
    }

    if (outBuffer != 0)
        *outsize = outSize;
    return outBuffer;
}

void* gzip_encode(const void* in, uint32_t insize, uint32_t* outsize) {
    uint32_t outSize = 0;
    uint8_t* outBuffer = 0;

    if (in != 0) {
        z_stream gzip;
        gzip.zalloc = Z_NULL;
        gzip.zfree = Z_NULL;
        gzip.opaque = Z_NULL;
        gzip.total_out = 0;
        gzip.next_in = (Bytef*)in;
        gzip.avail_in = insize;

        int res = deflateInit2(&gzip, Z_BEST_COMPRESSION, Z_DEFLATED, (15+16), 8, Z_DEFAULT_STRATEGY);
        if (res == Z_OK) {
            uint32_t gzipBufferSize = insize + (insize / 100) + 12;
            Bytef* gzipBuffer = memAlloc(Bytef, gzipBuffer, gzipBufferSize);

            do {
                gzip.next_out = gzipBuffer + gzip.total_out;
                gzip.avail_out = (uInt)(gzipBufferSize - gzip.total_out);
                res = deflate(&gzip, Z_FINISH);
            } while (res == Z_OK);

            deflateEnd(&gzip);

            if ((res != Z_STREAM_END) || (gzip.total_out > gzip.total_in)) {
                memFree(gzipBuffer);
            } else {
                outSize = (uint32_t)gzip.total_out;
                outBuffer = gzipBuffer;
            }
        }
    }

    if (outBuffer != 0)
        *outsize = outSize;
    return outBuffer;
}

void* gzip_decode(const void* in, uint32_t insize, uint32_t* outsize) {
    uint32_t outSize = 0;
    uint8_t* outBuffer = 0;

    if (in != 0) {
        z_stream gzip;
        gzip.zalloc = Z_NULL;
        gzip.zfree = Z_NULL;
        gzip.opaque = Z_NULL;
        gzip.total_out = 0;
        gzip.next_in = (Bytef*)in;
        gzip.avail_in = insize;

        int res = inflateInit2(&gzip, (15+16));
        if (res == Z_OK) {
            uint32_t gzipBufferSize = insize;
            if (gzipBufferSize < 1024) gzipBufferSize = 1024;
            Bytef* gzipBuffer = memAlloc(Bytef, gzipBuffer, gzipBufferSize);

            do {
                if ((gzipBufferSize - gzip.total_out) < 1024) {
                    gzipBufferSize += 1024;
                    Bytef* gzipNewBuffer = memRealloc(Bytef, gzipNewBuffer, gzipBuffer, gzipBufferSize);
                    if (gzipNewBuffer != 0) {
                        gzipBuffer = gzipNewBuffer;
                    } else {
                        break;
                    }
                }

                gzip.next_out = gzipBuffer + gzip.total_out;
                gzip.avail_out = (uInt)(gzipBufferSize - gzip.total_out);
                res = inflate(&gzip, Z_FINISH);
            } while (res == Z_OK);

            inflateEnd(&gzip);

            if (res != Z_STREAM_END) {
                memFree(gzipBuffer);
            } else {
                outSize = (uint32_t)gzip.total_out;
                outBuffer = gzipBuffer;
            }
        }
    }

    if (outBuffer != 0)
        *outsize = outSize;
    return outBuffer;
}

const char base64_encode_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const uint8_t base64_decode_table[128] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,62,0,0,0,63,52,53,54,55,56,57,58,59,60,61,0,0,0,0,0,0,
        0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,
        0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,0,0,0,0,0
};

bool is_base64(const char* text) {
    if (text == 0) return false;
    int32_t len = wcs_strlen(text, NULL);

    while ((len > 0) && (text[--len] == '=')) { };
    len++;

    while (--len >= 0) {
        if (strrchr(base64_encode_table, text[len]) == 0) break;
    }

    return (len < 0);
}

char* base64_encode(const void* in, uint32_t insize, uint32_t* outsize) {
    uint32_t size = (uint32_t)div(insize + 2, 3).quot * 4;
    char* out = memAlloc(char,out,(size+1)); // zero termitaned string
    register char* outBuf = out;
    register uint8_t* inBuf = (uint8_t*)in;
    if (outsize) *outsize = size;
    while (insize > 0) {
        int n = (insize > 3) ? 3 : insize;
        uint32_t w = 0;
        int i = 0;
        for (; i < n; i++) {
            uint32_t b = inBuf[i];
            w += (b << ((2 - i) * 8));
        }
        outBuf[0] = base64_encode_table[(w >> 18) & 0x3f];
        outBuf[1] = base64_encode_table[(w >> 12) & 0x3f];
        outBuf[2] = base64_encode_table[(w >> 6) & 0x3f];
        outBuf[3] = base64_encode_table[w & 0x3f];
        if (n != 3) {
            for (i = n + 1; i < 4; i++) {
                outBuf[i] = L'=';
            }
        }
        insize -= n;
        inBuf += 3;
        outBuf += 4;
    }
    *outBuf = 0;
    return out;
}

void* base64_decode(const char* in, uint32_t insize, uint32_t* outsize) {
    while (in[insize-1] == '=') insize--;
    uint32_t size = (((insize * 3) << 1) >> 3);
    uint32_t bufsize = ((insize + 3) >> 2) * 3;
    void* out = memAlloc(void,out,bufsize + 1);
    register uint8_t* outBuf = (uint8_t*)out;
    register uint8_t* inBuf = (uint8_t*)in;
    if (outsize) *outsize = size;
    while (insize > 0) {
        int n = (insize > 4) ? 4 : insize;
        uint32_t w = 0;
        int i = 0;
        for (; i < n; i++) {
            w |= (uint32_t)base64_decode_table[*inBuf] << ((3 - i) * 6);
            inBuf++;
        }
        outBuf[0] = (uint8_t)((w >> 16) & 0xFF);
        outBuf[1] = (uint8_t)((w >> 8) & 0xFF);
        outBuf[2] = (uint8_t)(w & 0xFF);
        insize -= n;
        outBuf += 3;
    }
    *outBuf = 0;
    return out;
}

bool is_base64_unicode(const wchar_t* text) {
    if (text == 0) return false;
    int32_t len = wcs_strlen(text, NULL);

    while ((len > 0) && (text[--len] == '=')) { };
    len++;

    while (--len >= 0) {
        wchar_t ch = text[len];
        if (ch >= 0x100) break;
        if (strrchr(base64_encode_table, (char)ch) == 0) break;
    }

    return (len < 0);
}

wchar_t* base64_encode_unicode(const void* in, uint32_t insize, uint32_t* outsize) {
    uint32_t size = (uint32_t)div(insize + 2, 3).quot * 4;
    wchar_t* out = memAlloc(wchar_t,out,(size+1) * 2); // zero termitaned string
    register wchar_t* outBuf = out;
    register uint8_t* inBuf = (uint8_t*)in;
    if (outsize) *outsize = size;
    while (insize > 0) {
        int n = (insize > 3) ? 3 : insize;
        uint32_t w = 0;
        int i = 0;
        for (; i < n; i++) {
            uint32_t b = inBuf[i];
            w += (b << ((2 - i) * 8));
        }
        outBuf[0] = (wchar_t)base64_encode_table[(w >> 18) & 0x3f];
        outBuf[1] = (wchar_t)base64_encode_table[(w >> 12) & 0x3f];
        outBuf[2] = (wchar_t)base64_encode_table[(w >> 6) & 0x3f];
        outBuf[3] = (wchar_t)base64_encode_table[w & 0x3f];
        if (n != 3) {
            for (i = n + 1; i < 4; i++) {
                outBuf[i] = L'=';
            }
        }
        insize -= n;
        inBuf += 3;
        outBuf += 4;
    }
    *outBuf = 0;
    return out;
}

void* base64_decode_unicode(const wchar_t* in, uint32_t insize, uint32_t* outsize) {
    while (in[insize-1] == '=') insize--;
    uint32_t size = (((insize * 3) << 1) >> 3);
    uint32_t bufsize = ((insize + 3) >> 2) * 3;
    void* out = memAlloc(void,out,bufsize + 1);
    register uint8_t* outBuf = (uint8_t*)out;
    register uint16_t* inBuf = (uint16_t*)in;
    if (outsize) *outsize = size;
    while (insize > 0) {
        int n = (insize > 4) ? 4 : insize;
        uint32_t w = 0;
        int i = 0;
        for (; i < n; i++) {
            w |= (uint32_t)base64_decode_table[*inBuf] << ((3 - i) * 6);
            inBuf++;
        }
        outBuf[0] = (uint8_t)((w >> 16) & 0xFF);
        outBuf[1] = (uint8_t)((w >> 8) & 0xFF);
        outBuf[2] = (uint8_t)(w & 0xFF);
        insize -= n;
        outBuf += 3;
    }
    *outBuf = 0;
    return out;
}
