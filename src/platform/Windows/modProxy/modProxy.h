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

#ifndef MODPROXY_H
#define MODPROXY_H

#include <stdint.h>

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#elseif BUILD_TEST
    #define DLL_EXPORT
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

// exported

#ifdef __cplusplus
}
#endif

#endif
