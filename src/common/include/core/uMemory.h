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

#ifndef JAPPSY_UMEMORY_H
#define JAPPSY_UMEMORY_H

#include <platform.h>

#ifdef DEBUG
	#define MEMORY_LOG	0			// Log Memory Operations
#else
	#define MEMORY_LOG	0
#endif
#define MM_VIRTUAL	0			// Use Virtual Memory Caching

#ifdef __cplusplus
extern "C" {
#endif

void* mmalloc(size_t dwSize);
#define mmcalloc(dwSize, dwCount)	mmalloc((dwSize) * (dwCount))
void* mmrealloc(void* mem, size_t dwSize);
void mmfree(void* mem);

void atomic_bzero(void* dst, size_t size);
void atomic_memset(void* dst, uint8_t val, size_t size);
void atomic_memread(void* dst, const void* src, size_t size);
void atomic_memwrite(void* dst, const void* src, size_t size);
void atomic_memcpy(void* dst, const void* src, size_t size);
void atomic_memmove(void* dst, const void* src, size_t size);

uint64_t mmStats(uint32_t* stats, int count);
void mmCleanup();

void mmInit();
void mmQuit();

#ifdef DEBUG
    void memLogAlloc(const char* location, const char* type, const char* var, const void* ptr, size_t size);
    void memLogRealloc(const char* location, const char* type, const char* varNew, const void* ptrNew, const char* varOld, const void* ptrOld, size_t sizeNew);
    void memLogFree(const char* var, const void* ptr);
    void memLogNew(const char* location, const char* var, const char* type, const void* ptr, size_t size);
    void memLogDelete(const char* var, const void* ptr);
    void memLogStats(uint32_t* count, uint32_t* mallocCount, uint32_t* newCount, size_t* size);
    void memLogCallFunction(const char* location);
    void memLogRetFunction();
#endif

#ifndef MM_T
    #define MM_T(...)        MM_T_(__VA_ARGS__)
#endif

#ifndef MM_T_
    #define MM_T_(...)       #__VA_ARGS__
#endif

#ifndef MM_LOC
    #define MM_LOC         MM_T(__FILE__) "(" MM_T(__LINE__) ")"
#endif

#ifdef DEBUG

    #define memAlloc(type,var,size) \
    		(type*)mmalloc(size); \
    		memLogAlloc(MM_LOC, MM_T(type), MM_T(var), var, size)

    #define memRealloc(type,varNew,varOld,size) \
            (type*)mmrealloc(varOld, size); \
            memLogRealloc(MM_LOC, MM_T(type), MM_T(varNew), varNew, MM_T(varOld), varOld, size)

    #define memFree(var) \
            memLogFree(MM_T(var), var); \
            mmfree(var)

    #define memNew(var,type, ...) \
            new type, ## __VA_ARGS__; \
            memLogNew(MM_LOC, MM_T(var), MM_T(type, ## __VA_ARGS__), var, sizeof(*var))

    #define memDelete(var) \
            memLogDelete(MM_T(var), var); \
            delete var

	#define memAllocNew(type,var,size) \
			(type*)mmalloc(size); \
			memLogNew(MM_LOC, MM_T(var), MM_T(type), ((uint8_t*)var) + 1, size)
	
	#define memFreeNew(var) \
			memLogDelete(MM_T(var), ((uint8_t*)var) + 1); \
			mmfree(var)

	#define memCall(func) \
            memLogCallFunction(MM_LOC); \
            func \
            memLogRetFunction();

    #define memCallRet(var, func) \
            memLogCallFunction(MM_LOC); \
            var = func; \
            memLogRetFunction();

    #define memCallStart() \
            memLogCallFunction(MM_LOC);

    #define memCallEnd() \
            memLogRetFunction();

#else

    #define memAlloc(type,var,size) \
        (type*)mmalloc(size);

    #define memRealloc(type,varNew,varOld,size) \
        (type*)mmrealloc(varOld, size)

    #define memFree(var) \
        mmfree(var)

	#define memNew(var,type, ...) \
		new type, ## __VA_ARGS__
	
    #define memDelete(var) \
        delete var

	#define memAllocNew(type,var,size) \
		(type*)mmalloc(size);

	#define memFreeNew(var) \
		mmfree(var)
	
    #define memCall(func) \
        func

    #define memCallRet(var, func) \
        var = func;

    #define memCallStart()

    #define memCallEnd()

#endif

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_UMEMORY_H
