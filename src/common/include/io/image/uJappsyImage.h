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

#ifndef ANDROID_UJAPPSYIMAGE_H
#define ANDROID_UJAPPSYIMAGE_H

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

bool is_jappsyImage(const void* ptr, uint32_t insize, char** error);
bool jappsyImage_getSize(const void* ptr, uint32_t insize, uint32_t* lpWidth, uint32_t* lpHeight, char** error);
void* jappsyImage_getPixels(const void* ptr, uint32_t insize, uint32_t outWidth, uint32_t outHeight, int32_t x1, int32_t y1, int32_t x2, int32_t y2, char** error);

#ifdef __cplusplus
}
#endif

#endif //ANDROID_UJAPPSYIMAGE_H
