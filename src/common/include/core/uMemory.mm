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
	
	void atomic_bzero(void* dst, uint32_t size) {
		if ((dst == NULL) || (size == 0))
			return;
		
		uint8_t* buf = (uint8_t*)dst;
#if defined(__X64__)
		uint32_t aligned = size >> 3; size &= 7;
		while (aligned-- > 0) {
			__atomic_store_n((uint64_t*)buf, 0, __ATOMIC_RELEASE);
			buf += 8;
		}
#else
		uint32_t aligned = size >> 2; size &= 3;
		while (aligned-- > 0) {
			__atomic_store_n((uint32_t*)buf, 0, __ATOMIC_RELEASE);
			buf += 4;
		}
#endif
		while (size-- > 0) {
			__atomic_store_n(buf, 0, __ATOMIC_RELEASE);
			buf++;
		}
	}
	
	void atomic_memset(void* dst, uint8_t val, uint32_t size) {
		if ((dst == NULL) || (size == 0))
			return;
		
		uint8_t* buf = (uint8_t*)dst;
#if defined(__X64__)
		uint32_t aligned = size >> 3; size &= 7;
		uint64_t value = ((uint32_t)val << 24) | ((uint32_t)val << 16) | ((uint32_t)val << 8) | (uint32_t)val; value |= (value << 32);
		while (aligned-- > 0) {
			__atomic_store_n((uint64_t*)buf, value, __ATOMIC_RELEASE);
			buf += 8;
		}
#else
		uint32_t aligned = size >> 2; size &= 3;
		uint32_t value = ((uint32_t)val << 24) | ((uint32_t)val << 16) | ((uint32_t)val << 8) | (uint32_t)val;
		while (aligned-- > 0) {
			__atomic_store_n((uint32_t*)buf, value, __ATOMIC_RELEASE);
			buf += 4;
		}
#endif
		while (size-- > 0) {
			__atomic_store_n(buf, val, __ATOMIC_RELEASE);
			buf++;
		}
	}
	
	void atomic_memread(void* dst, const void* src, uint32_t size) {
		if ((dst == NULL) || (src == NULL) || (dst == src) || (size == 0))
			return;
		
		uint8_t* target = (uint8_t*)dst;
		uint8_t* source = (uint8_t*)src;
#if defined(__X64__)
		uint32_t aligned = size >> 3; size &= 7;
		while (aligned-- > 0) {
			__atomic_load((uint64_t*)source, (uint64_t*)target, __ATOMIC_ACQUIRE);
			target += 8; source += 8;
		}
#else
		uint32_t aligned = size >> 2; size &= 3;
		while (aligned-- > 0) {
			__atomic_load((uint32_t*)source, (uint32_t*)target, __ATOMIC_ACQUIRE);
			target += 4; source += 4;
		}
#endif
		while (size-- > 0) {
			__atomic_load(source, target, __ATOMIC_ACQUIRE);
			target++; source++;
		}
	}
	
	void atomic_memwrite(void* dst, const void* src, uint32_t size) {
		if ((dst == NULL) || (src == NULL) || (dst == src) || (size == 0))
			return;
		
		uint8_t* target = (uint8_t*)dst;
		uint8_t* source = (uint8_t*)src;
#if defined(__X64__)
		uint32_t aligned = size >> 3; size &= 7;
		while (aligned-- > 0) {
			__atomic_store((uint64_t*)target, (uint64_t*)source, __ATOMIC_RELEASE);
			target += 8; source += 8;
		}
#else
		uint32_t aligned = size >> 2; size &= 3;
		while (aligned-- > 0) {
			__atomic_store((uint32_t*)target, (uint32_t*)source, __ATOMIC_RELEASE);
			target += 4; source += 4;
		}
#endif
		while (size-- > 0) {
			__atomic_store(target, source, __ATOMIC_RELEASE);
			target++; source++;
		}
	}
	
	void atomic_memcpy(void* dst, const void* src, uint32_t size) {
		if ((dst == NULL) || (src == NULL) || (dst == src) || (size == 0))
			return;
		
		uint8_t* target = (uint8_t*)dst;
		uint8_t* source = (uint8_t*)src;
#if defined(__X64__)
		uint32_t aligned = size >> 3; size &= 7;
		while (aligned-- > 0) {
			__atomic_store_n((uint64_t*)target, __atomic_load_n((uint64_t*)source, __ATOMIC_ACQUIRE), __ATOMIC_RELEASE);
			target += 8; source += 8;
		}
#else
		uint32_t aligned = size >> 2; size &= 3;
		while (aligned-- > 0) {
			__atomic_store_n((uint32_t*)target, __atomic_load_n((uint32_t*)source, __ATOMIC_ACQUIRE), __ATOMIC_RELEASE);
			target += 4; source += 4;
		}
#endif
		while (size-- > 0) {
			__atomic_store_n(target, __atomic_load_n(source, __ATOMIC_ACQUIRE), __ATOMIC_RELEASE);
			target++; source++;
		}
	}
	
	void atomic_memmove(void* dst, const void* src, uint32_t size) {
		if ((dst == NULL) || (src == NULL) || (dst == src) || (size == 0))
			return;
		
		uint8_t* target = (uint8_t*)dst;
		uint8_t* source = (uint8_t*)src;
		
		if (((intptr_t)target < (intptr_t)source) || ((intptr_t)(source + size) <= (intptr_t)target)) {
#if defined(__X64__)
			uint32_t aligned = size >> 3; size &= 7;
			while (aligned-- > 0) {
				__atomic_store_n((uint64_t*)target, __atomic_load_n((uint64_t*)source, __ATOMIC_ACQUIRE), __ATOMIC_RELEASE);
				target += 8; source += 8;
			}
#else
			uint32_t aligned = size >> 2; size &= 3;
			while (aligned-- > 0) {
				__atomic_store_n((uint32_t*)target, __atomic_load_n((uint32_t*)source, __ATOMIC_ACQUIRE), __ATOMIC_RELEASE);
				target += 4; source += 4;
			}
#endif
			while (size-- > 0) {
				__atomic_store_n(target, __atomic_load_n(source, __ATOMIC_ACQUIRE), __ATOMIC_RELEASE);
				target++; source++;
			}
		} else {
			target += size; source += size;
#if defined(__X64__)
			uint32_t aligned = size >> 3; size &= 7;
			while (aligned-- > 0) {
				target -= 8; source -= 8;
				__atomic_store_n((uint64_t*)target, __atomic_load_n((uint64_t*)source, __ATOMIC_ACQUIRE), __ATOMIC_RELEASE);
			}
#else
			uint32_t aligned = size >> 2; size &= 3;
			while (aligned-- > 0) {
				target -= 4; source -= 4;
				__atomic_store_n((uint32_t*)target, __atomic_load_n((uint32_t*)source, __ATOMIC_ACQUIRE), __ATOMIC_RELEASE);
			}
#endif
			while (size-- > 0) {
				target--; source--;
				__atomic_store_n(target, __atomic_load_n(source, __ATOMIC_ACQUIRE), __ATOMIC_RELEASE);
			}
		}
	}
	
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
volatile bool mmLock = false;
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

static volatile int32_t mmInitCounter = 0;

void* mmalloc(uint32_t dwSize) {
#if defined(MM_CHECK_LEFT) || defined(MM_CHECK_RIGHT) || (MM_VIRTUAL != 0)
	if (AtomicCompareExchange(&mmInitCounter, 0, 0) == 0)
		return malloc(dwSize);
    return mmvalloc(dwSize);
#elif defined(__WINNT__)
	if (AtomicCompareExchange(&mmInitCounter, 0, 0) == 0)
		return malloc(dwSize);
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
	if (AtomicCompareExchange(&mmInitCounter, 0, 0) == 0) {
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
	}
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
	if (AtomicCompareExchange(&mmInitCounter, 0, 0) == 0) {
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
	}
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
	if (AtomicCompareExchange(&mmInitCounter, 0, 0) == 0) {
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
	}
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
		if (AtomicCompareExchange(&mmInitCounter, 0, 0) == 0) {
			free(mem);
			return;
		}
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
		if (AtomicCompareExchange(&mmInitCounter, 0, 0) == 0) {
			try {
				free(mem);
			} catch (...) {
			}
			return;
		}
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
#ifdef DEBUG
		atomic_bzero(ptr, prevSize);
#endif
        if (prevSize < 2048) {
            free(ptr);
        } else {
            mmVirtualFree(ptr);
        }
#elif defined(__WINNT__)
		if (AtomicCompareExchange(&mmInitCounter, 0, 0) == 0) {
			free(mem);
			return;
		}
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

struct MemLogElement {
    char type[60];
    char var[60];
    void* ptr;
    int32_t size;
    char loc[MAX_PATH];
    char caller[3][MAX_PATH];
};

#if MEMORY_LOG == 1
static volatile struct MemLogElement* memLogElements = 0;
static volatile int32_t memLogElementsCount = 0;
#endif
static volatile bool memLockTrigger = false;

#define memLock() AtomicFastLock(&memLockTrigger)
#define memUnlock() AtomicUnlock(&memLockTrigger)

#if MEMORY_LOG == 1
#define memLogMaxCallStackSize		256
static volatile int32_t memLogCallStackCount = -1;
static volatile char memLogCallStack[memLogMaxCallStackSize][MAX_PATH];
#endif

void memLogCallFunction(const char* location) {
#if MEMORY_LOG == 1
    memLock();
	int32_t index;
	if ((index = AtomicIncrement(&memLogCallStackCount)) < memLogMaxCallStackSize) {
		int32_t len = strlen(location);
		atomic_memwrite((char*)memLogCallStack[index], location, len + 1);
	}
    memUnlock();
#endif
}

void memLogRetFunction() {
#if MEMORY_LOG == 1
    memLock();
	AtomicDecrement(&memLogCallStackCount);
    memUnlock();
#endif
}

void memAdd(const char* location, const char* type, const char* var, const void* ptr, uint32_t size) {
#if MEMORY_LOG == 1
    memLock();
    struct MemLogElement* el = 0;
	struct MemLogElement* els = (struct MemLogElement*)AtomicGetPtr(&memLogElements);
    if (els == 0) {
        els = (struct MemLogElement*)mmalloc(sizeof(struct MemLogElement)*1024);
		AtomicSetPtr(&memLogElements, els);
		atomic_bzero((void*)els, sizeof(struct MemLogElement)*1024);
        AtomicSet(&memLogElementsCount, 1024);
        el = els;
    } else {
        int32_t i = 0;
		int32_t count = AtomicGet(&memLogElementsCount);
        for (; i < count; i++) {
            if (AtomicGetPtr(&(els[i].ptr)) == NULL) {
                el = &(els[i]);
                break;
            }
        }
    }
    if (el == 0) {
		int32_t count = AtomicGet(&memLogElementsCount);
        struct MemLogElement* newElements = (struct MemLogElement*)mmalloc((uint32_t)sizeof(struct MemLogElement)*(count+1024));
		atomic_bzero((void*)newElements, sizeof(struct MemLogElement)*(count+1024));
		atomic_memcpy((void*)newElements, (void*)els, sizeof(struct MemLogElement)*(count));
		mmfree(els);
		els = newElements;
		AtomicSetPtr(&memLogElements, els);
		AtomicAdd(&memLogElementsCount, 1024);
		el = &(els[count]);
    }
	bzero(el, sizeof(struct MemLogElement));
	strcpy(el->loc, location);
	
	int cnt = AtomicGet(&memLogCallStackCount);
    if (cnt >= 0) {
		__sync_synchronize();
		
		cnt++;
        if (cnt > 3) cnt = 3;
        int i = 0;
        for (; i < cnt; i++) {
            strcpy(el->caller[cnt-i-1], (char*)memLogCallStack[memLogCallStackCount-i]);
        }
    }

	strcpy(el->type, type);
    strcpy(el->var, var);
    AtomicSetPtr(&(el->ptr), (void*)ptr);
    AtomicSet(&(el->size), size);
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

    struct MemLogElement* el = 0;
    uint32_t freeCount = 0;
	struct MemLogElement* els = (struct MemLogElement*)AtomicGetPtr(&memLogElements);
	int32_t count = AtomicGet(&memLogElementsCount);
    if (els != 0) {
        int32_t i = 0;
        for (; i < count; i++) {
			void* elptr = AtomicGetPtr(&(els[i].ptr));
            if (elptr != 0) {
                if (elptr == ptr) {
                    el = &(els[i]);
                }
            } else {
                freeCount++;
            }
        }
        if (el != 0) freeCount++;
    }
	if ((freeCount > 0) && (freeCount == count)) {
		AtomicSet(&memLogElementsCount, 0);
		mmfree(els);
		AtomicSetPtr(&memLogElements, NULL);
	}
    if (el == 0) {
        memUnlock();
        return;
    }
	
    (void)AtomicSub(&memSize, el->size);
    atomic_bzero(el, sizeof(struct MemLogElement));
    memUnlock();
#endif
}

void memLogSort() {
#if MEMORY_LOG == 1
    memLock();
	struct MemLogElement* els = (struct MemLogElement*)AtomicGetPtr(&memLogElements);
	int32_t count = AtomicGet(&memLogElementsCount);
    if (els != 0) {
        int32_t i = 0; int32_t j = 0;
        while (i < count) {
			void* elptri = AtomicGetPtr(&(els[i].ptr));
            if (elptri != 0) {
                i++; j = i+1;
            } else if (j < count) {
				void* elptrj = AtomicGetPtr(&(els[i].ptr));
                if (elptrj == 0) {
                    j++;
                } else {
					atomic_memcpy(&(els[i]), &(els[j]), sizeof(struct MemLogElement));
					atomic_bzero(&(els[j]), sizeof(struct MemLogElement));
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
    } else if (ptrOld != 0) {
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

	__sync_synchronize();
    for (i = 0; i < memCount; i++) {
        struct MemLogElement* item = (struct MemLogElement*)&(memLogElements[i]);
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
            snprintf(buf, 2048, "/* %s */\n%s* %s = (%s*)malloc(%d); // 0x%08lx\n",
                     item->loc, item->type, item->var, item->type, item->size, (intptr_t)item->ptr);
#else
            sprintf(buf, "/* %s */\r\n%s* %s = (%s*)malloc(%d); // 0x%08x\r\n",
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
        (void)AtomicSetPtr(&memLogElements, NULL);
        (void)AtomicSet(&memLogElementsCount, 0);
#endif
        (void)AtomicSet(&memLockTrigger, 0);
#if MEMORY_LOG == 1
        (void)AtomicSet(&memLogCallStackCount, -1);
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

    return env->NewDirectByteBuffer(jniMemory, size);
}

JNIEXPORT void JNICALL
Java_com_jappsy_core_Memory_free(JNIEnv *env, jclass type, jobject memory) {
    if (memory == NULL) {
        jniThrow(env, eNullPointer, NULL);
        return;
    }

    void* jniMemory = env->GetDirectBufferAddress(memory);
    memFree(jniMemory);
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
