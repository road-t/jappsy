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

#ifndef JAPPSY_UCIPHER_H
#define JAPPSY_UCIPHER_H

#include "../platform.h"

#ifdef __cplusplus
extern "C" {
#endif

bool is_gzip(const void* in, uint32_t insize);
void* gzip_encode_fast(const void* in, uint32_t insize, uint32_t* outsize);
void* gzip_encode(const void* in, uint32_t insize, uint32_t* outsize);
void* gzip_decode(const void* in, uint32_t insize, uint32_t* outsize);

bool is_base64(const char* text);
char* base64_encode(const void* in, uint32_t insize, uint32_t* outsize);
void* base64_decode(const char* in, uint32_t insize, uint32_t* outsize);

bool is_base64_unicode(const wchar_t* text);
wchar_t* base64_encode_unicode(const void* in, uint32_t insize, uint32_t* outsize);
void* base64_decode_unicode(const wchar_t* in, uint32_t insize, uint32_t* outsize);

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_UCIPHER_H
