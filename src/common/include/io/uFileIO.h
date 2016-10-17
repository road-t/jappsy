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

#ifndef JAPPSY_UFILEIO_H
#define JAPPSY_UFILEIO_H

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__JNI__)

extern const char* clsFileDescriptor;

#endif

int fio_create(const char* path, const char** error);
int fio_createNew(const char* path, const char** error);
int fio_open(const char* path, const char** error);
bool fio_close(const int fd, const char** error);

bool fio_begin(const int fd, off_t* restore, off_t* size, const char** error);
bool fio_end(const int fd, off_t* restore, const char** error);
bool fio_readFully(const int fd, uint8_t* buffer, size_t len, const char** error);
bool fio_writeFully(const int fd, uint8_t* buffer, size_t len, const char** error);
bool fio_seek(const int fd, uint32_t ofs, const char** error);
bool fio_flush(const int fd, const char** error);
bool fio_truncate(const int fd, uint32_t size, const char** error);

void* fio_readFile(const int fd, uint32_t* size, const char** error);

bool fio_setModification(const int fd, uint64_t time, const char** error);
bool fio_getModification(const int fd, uint64_t* time, const char** error);

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_UFILEIO_H
