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

#ifndef JAPPSY_UERROR_H
#define JAPPSY_UERROR_H

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const char* eOK;
extern const char* eOutOfMemory;
extern const char* eNullPointer;
extern const char* eInvalidParams;
extern const char* eInvalidFormat;

extern const char* eIOFileNotOpen;
extern const char* eIOInvalidFile;
extern const char* eIOReadLimit;

extern const char* eInvalidPointer;
extern const char* eConvert;
extern const char* eOutOfRange;

extern const char* eUnknown;

#if defined(__JNI__)

void jniThrow(JNIEnv *env, const char* eString, const char* message);

#endif

bool is_errorPtr(const char* ptr);

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_UERROR_H
