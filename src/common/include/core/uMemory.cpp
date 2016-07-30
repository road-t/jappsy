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

#include "uMemory.h"
#include <core/uAtomic.h>
#include <cipher/uCrc.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__WINNT__)
volatile int32_t mmHeapInit = 0;
volatile int32_t mmHeap = 0;

#define MM_ZERO		0	//HEAP_ZERO_MEMORY

//#define MM_CHECK_LEFT
//#define MM_CHECK_RIGHT
#endif

struct MM_STACK_ITEM {
    void* ptr;
    struct MM_STACK_ITEM* next;
};

#if defined(MM_CHECK_LEFT) || defined(MM_CHECK_RIGHT) || (MM_VIRTUAL != 0)
void mmerror() {
#ifdef DEBUG
    memLogSort();
    memLogStats(0, 0, 0, 0);
#if defined(__WINNT__)
    MessageBoxW(0, L"VirtualMemory Error", L"MemoryManager", MB_OK);
#elif defined(__JNI__)
    LOG("MemoryManager: VirtualMemory Error");
#endif
#endif
}

#define MM_STACK_MAX	20
volatile int32_t mmLock = 0;
struct MM_STACK_ITEM* mmVirtual[MM_STACK_MAX];
struct MM_STACK_ITEM* mmUnused;

static inline uint32_t mmGetIndex(uint32_t mmsize) {
    uint32_t idx = 0;

    while (mmsize > 0) {
        idx++;
        mmsize >>= 1;
    }

    return idx;
}

uint64_t mmStats(uint32_t* stats, int count) {
    AtomicLock(&mmLock);

    uint64_t res = 0;
    int i = 0;

    for (; i < count; i++) {
        stats[i] = 0;
        if (i < MM_STACK_MAX) {
            uint32_t size = (uint32_t)1 << i;
            struct MM_STACK_ITEM* item = (struct MM_STACK_ITEM*)AtomicGetPtr((void**)&(mmVirtual[i]));
            while (item != 0) {
                stats[i]++;
                res += size;
                item = (struct MM_STACK_ITEM*)AtomicGetPtr((void**)&(item->next));
            }
        }
    }

    AtomicUnlock(&mmLock);

    return res;
}

void* mmVirtualAlloc(uint32_t mmsize) {
    if (mmsize == 0)
        return 0;

    uint32_t idx = mmGetIndex(mmsize);

    if (idx >= MM_STACK_MAX) {
#if defined(__WINNT__)
        return VirtualAlloc(0, mmsize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
        void* res = malloc(mmsize);
        if (res != 0)
            memset(res, 0, mmsize);
        return res;
#endif
    } else {
        AtomicLock(&mmLock);

        if (mmVirtual[idx] != 0) {
            struct MM_STACK_ITEM* item = (struct MM_STACK_ITEM*)AtomicGetPtr((void**)&(mmVirtual[idx]));
            (void)AtomicSetPtr((void**)&(mmVirtual[idx]), AtomicGetPtr((void**)&(item->next)));
            void* ptr = AtomicGetPtr(&(item->ptr));
            (void)AtomicSetPtr((void**)&(item->next), AtomicGetPtr((void**)&(mmUnused)));
            (void)AtomicSetPtr((void**)&(mmUnused), item);

            AtomicUnlock(&mmLock);
            memset(ptr, 0, mmsize);
            return ptr;
        } else {
            AtomicUnlock(&mmLock);
            mmsize = (uint32_t)1 << idx;
#if defined(__WINNT__)
            return VirtualAlloc(0, 1 << idx, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
            void* res = malloc(mmsize);
            if (res != 0)
                memset(res, 0, mmsize);
            return res;
#endif
        }
    }
}

void mmVirtualFree(uint32_t* ptr) {
    uint32_t mmsize = *ptr;

    uint32_t idx = mmGetIndex(mmsize);

    if (idx >= MM_STACK_MAX) {
#if defined(__WINNT__)
        if (VirtualFree(ptr, 0, MEM_RELEASE) == 0) {
            mmerror();
        }
#else
        free(ptr);
#endif
    } else {
        AtomicLock(&mmLock);

        struct MM_STACK_ITEM* item = (struct MM_STACK_ITEM*)AtomicGetPtr((void**)&(mmUnused));
        if (item != 0) {
            (void)AtomicSetPtr((void**)&(mmUnused), AtomicGetPtr((void**)&(item->next)));
        } else {
            item = (struct MM_STACK_ITEM*)malloc(sizeof(struct MM_STACK_ITEM));
        }
        (void)AtomicSetPtr((void**)&(item->next), AtomicGetPtr((void**)&(mmVirtual[idx])));
        (void)AtomicSetPtr(&(item->ptr), ptr);
        (void)AtomicSetPtr((void**)&(mmVirtual[idx]), item);

        AtomicUnlock(&mmLock);
    }
}
#endif

void mmCleanup() {
#if defined(MM_CHECK_LEFT) || defined(MM_CHECK_RIGHT) || (MM_VIRTUAL != 0)
    struct MM_STACK_ITEM* item;

    AtomicLock(&mmLock);

    item = (struct MM_STACK_ITEM*)AtomicGetPtr((void**)&(mmUnused));
    while (item != 0) {
        (void)AtomicSetPtr((void**)&(mmUnused), AtomicGetPtr((void**)&(item->next)));
        free(item);
        item = (struct MM_STACK_ITEM*)AtomicGetPtr((void**)&(mmUnused));
    }

    int i = 0;
    for (; i < MM_STACK_MAX; i++) {
        item = (struct MM_STACK_ITEM*)AtomicGetPtr((void**)&(mmVirtual[i]));
        while (item != 0) {
            (void)AtomicSetPtr((void**)&(mmVirtual[i]), AtomicGetPtr((void**)&(item->next)));
            free(AtomicGetPtr(&(item->ptr)));
            free(item);
            item = (struct MM_STACK_ITEM*)AtomicGetPtr((void**)&(mmVirtual[i]));
        }
    }

    AtomicUnlock(&mmLock);
#endif
}

#if defined(__WINNT__) && (defined(MM_CHECK_LEFT) || defined(MM_CHECK_RIGHT))
void* mmvalloc(uint32_t dwSize) {
    if (dwSize == 0)
        return 0;

    uint32_t pageSize = (uint32_t)AtomicCompareExchange(&mmPageSize, 0, 0);
    uint32_t pSize = (dwSize + pageSize - 1); pSize -= pSize % pageSize; pSize += pageSize;
    uint8_t* page = (uint8_t*)VirtualAlloc(0, pSize, MEM_RESERVE, PAGE_NOACCESS);
    if (page != 0) {
        #ifdef MM_CHECK_LEFT
            uint8_t* data = page + pageSize;
        #else
            uint8_t* data = page + pSize - pageSize - dwSize;
            page = data + dwSize;
        #endif
        if (VirtualAlloc(page, pageSize, MEM_COMMIT, PAGE_NOACCESS) == 0) {
            mmerror();
        }
        if (VirtualAlloc(data, dwSize, MEM_COMMIT, PAGE_READWRITE) == 0) {
            mmerror();
        }
        return data;
    } else {
        mmerror();
    }

    return 0;
}
#elif MM_VIRTUAL != 0
void* mmvalloc(uint32_t dwSize) {
    if (dwSize == 0)
        return 0;

    uint32_t* ptr = 0;
    if (dwSize < 2048) {
        ptr = (uint32_t*)malloc(dwSize + 8);
        if (ptr != 0) {
#if defined(__WINNT__)
            ZeroMemory(ptr, dwSize + 8);
#else
            memset(ptr, 0, dwSize + 8);
#endif
        }
    } else {
        ptr = (uint32_t*)mmVirtualAlloc(dwSize + 8);
    }

    if (ptr != 0) {
        *ptr = dwSize;
        *(ptr+1) = mmcrc32(0xFFFFFFFF, ptr, 4);
        return ptr+2;
    } else {
        mmerror();
    }

    return 0;
}
#endif

#if defined(__WINNT__)
#if defined(MM_CHECK_LEFT) || defined(MM_CHECK_RIGHT) || (MM_VIRTUAL != 0)
    static volatile int32_t mmPageSize = 0;
#endif
#endif

void* mmalloc(uint32_t dwSize) {
#if defined(MM_CHECK_LEFT) || defined(MM_CHECK_RIGHT) || (MM_VIRTUAL != 0)
    return mmvalloc(dwSize);
#elif defined(__WINNT__)
    HANDLE hHeap = HANDLE(AtomicCompareExchange(&mmHeap, 0, 0));
    if (hHeap == 0)
        return 0;
    if (dwSize == 0)
        return 0;
    return HeapAlloc(hHeap, MM_ZERO, dwSize);
#else
    return malloc(dwSize);
#endif
}

void* mmrealloc(void* mem, uint32_t dwSize) {
#if defined(__WINNT__) && (defined(MM_CHECK_LEFT) || defined(MM_CHECK_RIGHT))
    if (mem == 0) {
        return mmvalloc(dwSize);
    }
    if (dwSize == 0) {
        if (VirtualFree(mem, 0, MEM_RELEASE) == 0) {
            mmerror();
        }
        return 0;
    }
    void* nmem = mmvalloc(dwSize);
    if (nmem != 0) {
        MEMORY_BASIC_INFORMATION info = {};
        if (VirtualQuery(mem, &info, sizeof(info)) != 0) {
            #ifdef MM_CHECK_LEFT
                uint32_t len = info.RegionSize;
            #else
                uint32_t pageSize = (uint32_t)AtomicCompareExchange(&mmPageSize, 0, 0);
                uint32_t ofs = ((uint32_t)mem) % pageSize;
                uint32_t len = info.RegionSize - ofs;
            #endif
            if (len > dwSize) len = dwSize;
            memcpy(nmem, mem, len);
        } else {
            mmerror();
        }
        mmfree(mem);
        return nmem;
    }
    return 0;
#elif MM_VIRTUAL != 0
    if (mem == 0)
        return mmvalloc(dwSize);

    uint32_t* ptr = (uint32_t*)mem; ptr-=2;
    uint32_t prevSize = *ptr;
    uint32_t crc = mmcrc32(0xFFFFFFFF, ptr, 4);

    if (crc != *(ptr+1)) {
#ifdef DEBUG
#if defined(__IOS__)
        NSLog(@"Corrupted Memory");
#elif defined(__JNI__)
        LOG("Corrupted Memory");
#endif
#endif
#if defined(__WINNT__)
        Sleep(0);
#else
        sleep(0);
#endif
    }
    if (dwSize == 0) {
        if (prevSize < 2048) {
            free(ptr);
        } else {
            mmVirtualFree(ptr);
        }
        return 0;
    }
    uint32_t len = prevSize;
    if (len > dwSize) len = dwSize;
    if (prevSize < 2048) {
        if (dwSize < 2048) {
            uint32_t* nmem = (uint32_t*)realloc(ptr, dwSize + 8);
            if (nmem != 0) {
                *nmem = dwSize;
                *(nmem+1) = mmcrc32(0xFFFFFFFF, nmem, 4);
                return nmem+2;
            }
        } else {
            uint32_t* nmem = (uint32_t*)mmVirtualAlloc(dwSize + 8);
            if (nmem != 0) {
                memcpy(nmem+2, mem, len);
                *nmem = dwSize;
                *(nmem+1) = mmcrc32(0xFFFFFFFF, nmem, 4);
                free(ptr);
                return nmem+2;
            }
        }
    } else {
#if defined(__WINNT__)
        MEMORY_BASIC_INFORMATION info = {};
        if (VirtualQuery(ptr, &info, sizeof(info)) != 0) {
#endif
        if (dwSize < 2048) {
            uint32_t* nmem = (uint32_t*)malloc(dwSize + 8);
            if (nmem != 0) {
                memcpy(nmem+2, mem, len);
                *nmem = dwSize;
                *(nmem+1) = mmcrc32(0xFFFFFFFF, nmem, 4);
                mmVirtualFree(ptr);
                return nmem+2;
            }
        } else {
#if defined(__WINNT__)
            uint32_t pageSize = (uint32_t)AtomicCompareExchange(&mmPageSize, 0, 0);
            uint32_t pSize = (dwSize + 8 + pageSize - 1); pSize -= pSize % pageSize;
            if (pSize == info.RegionSize) {
                *ptr = dwSize;
                *(ptr+1) = mmcrc32(0xFFFFFFFF, ptr, 4);
                return mem;
            }
#else
            if (mmGetIndex(prevSize + 8) == mmGetIndex(dwSize + 8)) {
                *ptr = dwSize;
                *(ptr+1) = mmcrc32(0xFFFFFFFF, ptr, 4);
                return mem;
            }
#endif
            uint32_t* nmem = (uint32_t*)mmVirtualAlloc(dwSize + 8);
            if (nmem != 0) {
                memcpy(nmem+2, mem, len);
                *nmem = dwSize;
                *(nmem+1) = mmcrc32(0xFFFFFFFF, nmem, 4);
                mmVirtualFree(ptr);
                return nmem+2;
            }
        }
#if defined(__WINNT__)
        } else {
            mmerror();
        }
#endif
    }
    return 0;
#elif defined(__WINNT__)
    HANDLE hHeap = HANDLE(AtomicCompareExchange(&mmHeap, 0, 0));
    if (hHeap == 0)
        return 0;
    if (mem == 0) {
        if (dwSize == 0)
            return 0;
        return HeapAlloc(hHeap, MM_ZERO, dwSize);
    }
    if (dwSize == 0) {
        HeapFree(hHeap, 0, mem);
        return 0;
    }
    return HeapReAlloc(hHeap, MM_ZERO, mem, dwSize);
#else
    if (mem == 0) {
        if (dwSize == 0)
            return 0;
        return malloc(dwSize);
    }
    if (dwSize == 0) {
        free(mem);
        return 0;
    }
    return realloc(mem, dwSize);
#endif
}

void mmfree(void* mem) {
    if (mem != 0) {
#if defined(__WINNT__) && (defined(MM_CHECK_LEFT) || defined(MM_CHECK_RIGHT))
        uint32_t pageSize = (uint32_t)AtomicCompareExchange(&mmPageSize, 0, 0);
        uint8_t* page = (uint8_t*)mem;
        #ifdef MM_CHECK_LEFT
            page -= pageSize;
        #else
            page -= (((uint32_t)page) % pageSize);
        #endif
        if (VirtualFree(page, 0, MEM_RELEASE) == 0) {
            mmerror();
        }
#elif MM_VIRTUAL != 0
        uint32_t* ptr = (uint32_t*)mem; ptr-=2;
        uint32_t prevSize = *ptr;
        uint32_t crc = mmcrc32(0xFFFFFFFF, ptr, 4);
        if (crc != *(ptr+1)) {
#ifdef DEBUG
#if defined(__IOS__)
            NSLog(@"Corrupted Memory");
#elif defined(__JNI__)
            LOG("Corrupted Memory");
#endif
#endif
#if defined(__WINNT__)
            Sleep(0);
#else
            sleep(0);
#endif
        }
        if (prevSize < 2048) {
            free(ptr);
        } else {
            mmVirtualFree(ptr);
        }
#elif defined(__WINNT__)
        HANDLE hHeap = HANDLE(AtomicCompareExchange(&mmHeap, 0, 0));
        if (hHeap != 0)
            HeapFree(hHeap, 0, mem);
    #else
        free(ptr);
#endif
    }
}

static volatile int32_t memCount = 0;
static volatile int32_t memAllocCount = 0;
static volatile int32_t memNewCount = 0;
#if MEMORY_LOG == 1
static volatile int32_t memSize = 0;
#endif

struct tMemElement {
    char type[60];
    char var[60];
    void* ptr;
    uint32_t size;
    char loc[MAX_PATH];
    char caller[3][MAX_PATH];
};

#if MEMORY_LOG == 1
static struct tMemElement* memElements = 0;
static uint32_t memElementsCount = 0;
#endif
static volatile int32_t memLockTrigger = 0;

#define memLock() AtomicFastLock(&memLockTrigger)
#define memUnlock() AtomicUnlock(&memLockTrigger)

#if MEMORY_LOG == 1
#define memMaxCallStackSize		256
static int32_t memCallStackCount = -1;
static char memCallStack[memMaxCallStackSize][MAX_PATH];
#endif

void memLogCallFunction(const char* location) {
#if MEMORY_LOG == 1
    memLock();
    strcpy(memCallStack[++memCallStackCount], location);
    memUnlock();
#endif
}

void memLogRetFunction() {
#if MEMORY_LOG == 1
    memLock();
    memCallStackCount--;
    memUnlock();
#endif
}

void memAdd(const char* location, const char* type, const char* var, const void* ptr, uint32_t size) {
#if MEMORY_LOG == 1
    memLock();
    struct tMemElement* el = 0;
    if (memElements == 0) {
        memElements = (struct tMemElement*)mmalloc(sizeof(struct tMemElement)*1024);
        memset(memElements,0,sizeof(struct tMemElement)*1024);
        memElementsCount = 1024;
        el = memElements;
    } else {
        int i = 0;
        for (; i < memElementsCount; i++) {
            if (memElements[i].ptr == 0) {
                el = &(memElements[i]);
                break;
            }
        }
    }
    if (el == 0) {
        struct tMemElement* newElements = (struct tMemElement*)mmalloc((uint32_t)sizeof(struct tMemElement)*(memElementsCount+1024));
        memset(newElements,0,sizeof(struct tMemElement)*(memElementsCount+1024));
        memcpy(newElements,memElements,sizeof(struct tMemElement)*memElementsCount);
        mmfree(memElements);
        memElements = newElements;
        el = &(memElements[memElementsCount]);
        memElementsCount += 1024;
    }
    memset(el,0,sizeof(struct tMemElement));
    strcpy(el->loc, location);
    if (memCallStackCount >= 0) {
        int cnt = memCallStackCount + 1;
        if (cnt > 3) cnt = 3;
        int i = 0;
        for (; i < cnt; i++) {
            strcpy(el->caller[cnt-i-1], memCallStack[memCallStackCount-i]);
        }
    }
    strcpy(el->type, type);
    strcpy(el->var, var);
    el->ptr = (void*)ptr;
    el->size = size;
    memUnlock();
    (void)AtomicAdd(&memSize, size);
#endif
}

void memDel(const char* var, const void* ptr) {
#if MEMORY_LOG == 1
    memLock();
    if (ptr == 0) {
        memUnlock();
        return;
    }

    struct tMemElement* el = 0;
    uint32_t freeCount = 0;
    if (memElements != 0) {
        int i = 0;
        for (; i < memElementsCount; i++) {
            if (memElements[i].ptr != 0) {
                if (memElements[i].ptr == ptr) {
                    el = &(memElements[i]);
                }
            } else {
                freeCount++;
            }
        }
        if (el != 0) freeCount++;
    }
    if (el == 0) {
        if ((freeCount > 0) && (freeCount == memElementsCount)) {
            memElementsCount = 0;
            mmfree(memElements);
            memElements = 0;
        }

        memUnlock();
        return;
    } else {
        (void)AtomicSub(&memSize, el->size);
        memset(el,0,sizeof(struct tMemElement));

        if ((freeCount > 0) && (freeCount == memElementsCount)) {
            memElementsCount = 0;
            mmfree(memElements);
            memElements = 0;
        }
    }
    memUnlock();
#endif
}

void memLogSort() {
#if MEMORY_LOG == 1
    memLock();
    if (memElements != 0) {
        int i = 0; int j = 0;
        while (i < memElementsCount) {
            if (memElements[i].ptr != 0) {
                i++; j = i+1;
            } else if (j < memElementsCount) {
                if (memElements[j].ptr == 0) {
                    j++;
                } else {
                    memcpy(&(memElements[i]),&(memElements[j]),sizeof(struct tMemElement));
                    memset(&(memElements[j]),0,sizeof(struct tMemElement));
                    i++; j++;
                }
            } else {
                break;
            }
        }
    }
    memUnlock();
#endif
}

void memLogAlloc(const char* location, const char* type, const char* var, const void* ptr, uint32_t size) {
    if (size == 0)
        return;

    memAdd(location,type,var,ptr,size);
    (void)AtomicIncrement(&memCount);
    (void)AtomicIncrement(&memAllocCount);
}

void memLogRealloc(const char* location, const char* type, const char* varNew, const void* ptrNew, const char* varOld, const void* ptrOld, uint32_t sizeNew) {
    if (sizeNew == 0) {
        memDel(varOld,ptrOld);
        (void)AtomicDecrement(&memCount);
        (void)AtomicDecrement(&memAllocCount);
    } else if (varOld != 0) {
        if (ptrNew != 0) {
            memDel(varOld,ptrOld);
            memAdd(location,type,varNew,ptrNew,sizeNew);
        }
    } else {
        memAdd(location,type,varNew,ptrNew,sizeNew);
        (void)AtomicIncrement(&memCount);
        (void)AtomicIncrement(&memAllocCount);
    }
}

void memLogFree(const char* var, const void* ptr) {
    memDel(var,ptr);
    (void)AtomicDecrement(&memCount);
    (void)AtomicDecrement(&memAllocCount);
}

void memLogNew(const char* location, const char* var, const char* type, const void* ptr, uint32_t size) {
    memAdd(location,type,var,ptr,size);
    (void)AtomicIncrement(&memCount);
    (void)AtomicIncrement(&memNewCount);
}

void memLogDelete(const char* var, const void* ptr) {
    memDel(var,ptr);
    (void)AtomicDecrement(&memCount);
    (void)AtomicDecrement(&memNewCount);
}

void memLogStats(uint32_t* count, uint32_t* mallocCount, uint32_t* newCount, uint32_t* size) {
    if (count) *count = AtomicGet(&memCount);
    if (mallocCount) *mallocCount = AtomicGet(&memAllocCount);
    if (newCount) *newCount = AtomicGet(&memNewCount);
    if (size)
#if MEMORY_LOG == 1
        *size = AtomicGet(&memSize);
#else
        *size = 0;
#endif

#if MEMORY_LOG == 1
    memLock();

#if defined(__WINNT__)
    HANDLE hFile = CreateFileW(L"memory-dump.log", GENERIC_READ	| GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
#endif

    char buf[2048];
    char buf2[2048];
    char buf3[2048];

#if defined(__JNI__)
    snprintf(buf, 2048, "Memory Usage:: TotalCount: %d / AllocCount: %d / NewCount: %d / Size: %d",
             AtomicGet(&memCount),
             AtomicGet(&memAllocCount),
             AtomicGet(&memNewCount),
             AtomicGet(&memSize));
#else
    sprintf(buf, "Memory Usage:\r\nTotalCount: %d\r\nAllocCount: %d\r\nNewCount: %d\r\nSize: %d\r\n\r\n",
             AtomicGet(&memCount),
             AtomicGet(&memAllocCount),
             AtomicGet(&memNewCount),
             AtomicGet(&memSize));
#endif
#if defined(__WINNT__)
    WriteFile(hFile, buf, strlen(buf), &written, 0);
#elif defined(__IOS__)
    NSLog(@"%@", [NSString stringWithUTF8String:buf]);
#elif defined(__JNI__)
    LOG("%s", buf);
#else
    printf("%s", buf);
#endif

    uint32_t stats[20];
    uint64_t locked = mmStats(stats, 20);
    int i = 0;
    for (; i < 20; i++) {
        uint32_t blockSize = (uint32_t)1 << i;
        if (stats[i] > 0) {
#if defined(__JNI__)
            snprintf(buf, 2048, "Block %d: %d", blockSize, stats[i]);
#else
            sprintf(buf, "Block %d: %d\r\n", blockSize, stats[i]);
#endif
#if defined(__WINNT__)
            WriteFile(hFile, buf, strlen(buf), &written, 0);
#elif defined(__IOS__)
            NSLog(@"%@", [NSString stringWithUTF8String:buf]);
#elif defined(__JNI__)
            LOG("%s", buf);
#else
            printf("%s", buf);
#endif
        }
    }
#if defined(__JNI__)
    snprintf(buf, 2048, "Total %d MB", (uint32_t)(locked / (1024 * 1024)));
#else
    sprintf(buf, "Total %d MB\r\n\r\n", (uint32_t)(locked / (1024 * 1024)));
#endif
#if defined(__WINNT__)
    WriteFile(hFile, buf, strlen(buf), &written, 0);
#elif defined(__IOS__)
    NSLog(@"%@", [NSString stringWithUTF8String:buf]);
#elif defined(__JNI__)
    LOG("%s", buf);
#else
    printf("%s", buf);
#endif

    for (i = 0; i < memCount; i++) {
        struct tMemElement* item = &(memElements[i]);
        if (item != 0) {
            int j = 0;
            while ((j < 3) && (item->caller[j][0] != 0)) {
#if defined(__JNI__)
                snprintf(buf, 2048, "/* %s */\n", item->caller[j]);
#else
                sprintf(buf, "/* %s */\r\n", item->caller[i]);
#endif
#if defined(__WINNT__)
                WriteFile(hFile, buf, strlen(buf), &written, 0);
#elif defined(__IOS__)
                NSLog(@"%@", [NSString stringWithUTF8String:buf]);
#elif defined(__JNI__)
                LOG("%s", buf);
#else
                printf("%s", buf);
#endif
                j++;
            }
#if defined(__JNI__)
            snprintf(buf, 2048, "/* %s */\n%s %s = (%s*)malloc(%d); // 0x%08lx\n",
                     item->loc, item->type, item->var, item->type, item->size, (intptr_t)item->ptr);
#else
            sprintf(buf, "/* %s */\r\n%s %s = (%s*)malloc(%d); // 0x%08x\r\n",
                item->loc, item->type, item->var, item->type, item->size, (uint32_t)item->ptr);
#endif
#if defined(__WINNT__)
            WriteFile(hFile, buf, strlen(buf), &written, 0);
#elif defined(__IOS__)
            NSLog(@"%@", [NSString stringWithUTF8String:buf]);
#elif defined(__JNI__)
            LOG("%s", buf);
#else
            printf("%s", buf);
#endif

            try {
                int left = item->size;
                if (left > 256) left = 256;
                uint8_t* ptr = (uint8_t*)(item->ptr);
                if (ptr != 0) {
                    while (left > 0) {
                        int len = (left >= 32) ? 32 : left;
                        buf2[32] = 0;
                        buf3[0] = 0;
#if defined(__WINNT__)
                        MEMORY_BASIC_INFORMATION info = {};
                        if (VirtualQuery(ptr, &info, sizeof(info)) != 0) {
                            if ((info.State == MEM_COMMIT) && (info.AllocationProtect == PAGE_READWRITE)) {
#endif
                        int k = 0;
                        for (; k < len; k++) {
                            uint8_t ch = *ptr;
                            if ((ch >= 33) && (ch <= 127)) {
                                buf2[k] = *((char*)(&ch));
                            } else {
                                buf2[k] = '.';
                            }

#if defined(__JNI__)
                            snprintf(buf, 2048, " %02X", (uint32_t)ch);
#else
                            sprintf(buf, " %02X", (uint32_t)ch);
#endif
                            strcat(buf3, buf);

                            ptr++;
                        }
                        for (k = len; k < 32; k++)
                            buf2[k] = ' ';
#if defined(__JNI__)
                        snprintf(buf, 2048, "\t// %s |%s\n", buf2, buf3);
#else
                        sprintf(buf, "\t// %s |%s\r\n", buf2, buf3);
#endif
#if defined(__WINNT__)
                        } else {
                            sprintf(buf, "\t// No Access");
                        }
#endif
#if defined(__WINNT__)
                        WriteFile(hFile, buf, strlen(buf), &written, 0);
#elif defined(__IOS__)
                        NSLog(@"%@", [NSString stringWithUTF8String:buf]);
#elif defined(__JNI__)
                        LOG("%s", buf);
#else
                        printf("%s", buf);
#endif
#if defined(__WINNT__)
                        }
#endif
                        left -= len;
                    }
                    strcpy(buf, "\r\n");
#if defined(__WINNT__)
                    WriteFile(hFile, buf, strlen(buf), &written, 0);
#elif defined(__IOS__)
                    NSLog(@"%@", [NSString stringWithUTF8String:buf]);
#elif defined(__JNI__)
#else
                    printf("%s", buf);
#endif
                }
            } catch (...) {
            }
        }
    }
#if defined(__WINNT__)
    CloseHandle(hFile);
}
#endif
    memUnlock();
#endif
}

static int32_t mmInitCounter = 0;

void mmQuit() {
    if (AtomicDecrement(&mmInitCounter) == 1) {
        mmCleanup();
    }
}

#if defined(__WINNT__)
#if defined(MM_CHECK_LEFT) || defined(MM_CHECK_RIGHT) || (MM_VIRTUAL != 0)
    inline HANDLE mmInitHeap() {
        long heapState = AtomicCompareExchange(&mmHeapInit, 1, 0);
        if (heapState < 0)
            return 0;
        if (heapState == 0) {
            void* mem = VirtualAlloc(0, 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (mem != 0) {
                MEMORY_BASIC_INFORMATION info = {};
                if (VirtualQuery(mem, &info, sizeof(info)) != 0) {
                    AtomicExchange(&mmPageSize, info.RegionSize);
                }
                VirtualFree(mem, 0, MEM_RELEASE);
            }
            (void)AtomicExchange(&mmHeapInit, 2);
        } else if (heapState == 1) {
            do {
				Sleep(1);
                heapState = AtomicCompareExchange(&mmHeapInit, 0, 0);
            } while (heapState == 1);
        }
        return 0;
    }
#else
    inline HANDLE mmInitHeap() {
        long heapState = AtomicCompareExchange(&mmHeapInit, 1, 0);
        if (heapState < 0)
            return 0;
        if (heapState == 0) {
            HANDLE hHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0, 0);
            if (hHeap == 0) {
                (void)AtomicExchange(&mmHeapInit, -1);
                return 0;
            }
            (void)AtomicExchange(&mmHeap, long(hHeap));
            (void)AtomicExchange(&mmHeapInit, 2);
            return hHeap;
        } else if (heapState == 1) {
            do {
                Sleep(1);
                heapState = AtomicCompareExchange(&mmHeapInit, 0, 0);
            } while (heapState == 1);
            if (heapState < 0)
                return 0;
        }
        return HANDLE(AtomicCompareExchange(&mmHeap, 0, 0));
    }
#endif
#endif

void mmInit() {
    if (AtomicIncrement(&mmInitCounter) == 0) {
#if defined(__WINNT__)
        (void)AtomicSet(&mmHeapInit, 0);
        (void)AtomicSet(&mmHeap, 0);

		mmInitHeap();
#endif

#if defined(MM_CHECK_LEFT) || defined(MM_CHECK_RIGHT) || (MM_VIRTUAL != 0)
        AtomicUnlock(&mmLock);

        int i = 0;
        for (; i < MM_STACK_MAX; i++) {
            (void)AtomicSetPtr((void**)&(mmVirtual[i]), NULL);
        }

        (void)AtomicSetPtr((void**)&(mmUnused), NULL);
#endif

#if defined(__WINNT__)
        #if defined(MM_CHECK_LEFT) || defined(MM_CHECK_RIGHT) || (MM_VIRTUAL != 0)
            (void)AtomicSet(&mmPageSize, 0);
        #endif
#endif

        (void)AtomicSet(&memCount, 0);
        (void)AtomicSet(&memAllocCount, 0);
        (void)AtomicSet(&memNewCount, 0);
#if MEMORY_LOG == 1
        (void)AtomicSet(&memSize, 0);
        (void)AtomicSetPtr((void**)&memElements, NULL);
        (void)AtomicSet((int32_t*)&memElementsCount, 0);
#endif
        (void)AtomicSet(&memLockTrigger, 0);
#if MEMORY_LOG == 1
        (void)AtomicSet(&memCallStackCount, -1);
#endif
    }
}

#if defined(__JNI__)

JNIEXPORT jobject JNICALL
Java_com_jappsy_core_Memory_alloc(JNIEnv *env, jclass type, jint size) {
    if (size <= 0)
        return NULL;

    void* jniMemory = memAlloc(void, jniMemory, (uint32_t)size);
    if (jniMemory == NULL) {
        jniThrow(env, eOutOfMemory, NULL);
        return NULL;
    }

    jobject directBuffer = env->NewDirectByteBuffer(jniMemory, size);
    jobject memory = env->NewGlobalRef(directBuffer);
    return memory;
}

JNIEXPORT void JNICALL
Java_com_jappsy_core_Memory_free(JNIEnv *env, jclass type, jobject memory) {
    if (memory == NULL) {
        jniThrow(env, eNullPointer, NULL);
        return;
    }

    void* jniMemory = env->GetDirectBufferAddress(memory);
    memFree(jniMemory);
    env->DeleteGlobalRef(memory);
}

JNIEXPORT jintArray JNICALL
Java_com_jappsy_core_Memory_toIntArray(JNIEnv *env, jclass type, jobject memory) {
    if (memory == NULL) {
        jniThrow(env, eNullPointer, NULL);
        return NULL;
    }

    void* jniMemory = env->GetDirectBufferAddress(memory);
    if (jniMemory == NULL) {
        jniThrow(env, eNullPointer, NULL);
        return NULL;
    }
    jlong jniMemorySize = env->GetDirectBufferCapacity(memory);

    jintArray result;
    result = env->NewIntArray((jsize)jniMemorySize);
    if (result == NULL) {
        env->ExceptionClear();
        jniThrow(env, eOutOfMemory, NULL);
        return NULL;
    }

    env->SetIntArrayRegion(result, 0, (jsize)(jniMemorySize / 4), (jint*)jniMemory);

    return result;
}

#endif

#ifdef __cplusplus
}
#endif
