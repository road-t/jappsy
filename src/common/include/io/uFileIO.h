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

#ifndef ANDROID_UFILEIO_H
#define ANDROID_UFILEIO_H

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__JNI__)

extern const char* clsFileDescriptor;

#endif

bool fio_begin(const int fd, off_t* restore, off_t* size, char** error);
bool fio_end(const int fd, off_t* restore, char** error);
bool fio_readFully(const int fd, uint8_t* buffer, int len, char** error);
bool fio_writeFully(const int fd, uint8_t* buffer, int len, char** error);
bool fio_seek(const int fd, uint32_t ofs, char** error);
bool fio_flush(const int fd, char** error);
bool fio_truncate(const int fd, uint32_t size, char** error);

void* fio_readFile(const int fd, uint32_t* size, char** error);

#ifdef __cplusplus
}
#endif

#endif //ANDROID_UFILEIO_H
