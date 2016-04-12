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

#ifndef ANDROID_UATOMIC_H
#define ANDROID_UATOMIC_H

#include <platform.h>

/** Functions
    void AtomicLock(volatile int32_t* ptr);
    void AtomicFastLock(volatile int32_t* ptr);
    void AtomicUnlock(volatile int32_t* ptr);

    int32_t AtomicGet(volatile int32_t* ptr);
    int32_t AtomicSet(volatile int32_t* ptr);
 	int32_t AtomicExchange(volatile int32_t* ptr, int32_t val);
	int32_t AtomicCompareExchange(volatile int32_t* ptr, int32_t val, int32_t compare);
	int32_t AtomicAdd(volatile int32_t* ptr, int32_t val);
	int32_t AtomicSub(volatile int32_t* ptr, int32_t val);
	int32_t AtomicIncrement(volatile int32_t* ptr);
	int32_t AtomicDecrement(volatile int32_t* ptr);
	int32_t AtomicOr(volatile int32_t* ptr, int32_t val);
	int32_t AtomicXor(volatile int32_t* ptr, int32_t val);
	int32_t AtomicNor(volatile int32_t* ptr, int32_t val);
	int32_t AtomicAnd(volatile int32_t* ptr, int32_t val);
	int32_t AtomicNand(volatile int32_t* ptr, int32_t val);

    int64_t AtomicGet64(volatile int64_t* ptr);
    int64_t AtomicSet64(volatile int64_t* ptr);
	int64_t AtomicExchange64(volatile int64_t* ptr, int64_t val);
	int64_t AtomicCompareExchange64(volatile int64_t* ptr, int64_t val, int64_t compare);
	int64_t AtomicAdd64(volatile int64_t* ptr, int64_t val);
	int64_t AtomicSub64(volatile int64_t* ptr, int64_t val);
	int64_t AtomicIncrement64(volatile int64_t* ptr);
	int64_t AtomicDecrement64(volatile int64_t* ptr);
	int64_t AtomicOr64(volatile int64_t* ptr, int64_t val);
	int64_t AtomicXor64(volatile int64_t* ptr, int64_t val);
	int64_t AtomicNor64(volatile int64_t* ptr, int64_t val);
	int64_t AtomicAnd64(volatile int64_t* ptr, int64_t val);
	int64_t AtomicNand64(volatile int64_t* ptr, int64_t val);

    void* AtomicGetPtr(void* volatile *ptr);
    void* AtomicSetPtr(void* volatile *ptr);
	void* AtomicExchangePtr(void* volatile *ptr, void* val);
	void* AtomicCompareExchangePtr(void* volatile *ptr, void* val, void* compare);
 **/

#if defined(__IOS__)
    #import <libkern/OSAtomic.h>
#endif

#if !defined(AtomicLock)
    #define AtomicLock AtomicLock_Inline

    static inline void AtomicLock_Inline(volatile int32_t* ptr) {
        while (__sync_lock_test_and_set(ptr, 1) != 0)
            #if defined(__WINNT__)
                Sleep(1);
            #else
                sleep(1);
            #endif
    }
#endif

#if !defined(AtomicFastLock)
    #define AtomicFastLock AtomicFastLock_Inline

    static inline void AtomicFastLock_Inline(volatile int32_t* ptr) {
        while (__sync_lock_test_and_set(ptr, 1) != 0)
            #if defined(__WINNT__)
                Sleep(0);
            #else
                sleep(0);
            #endif
    }
#endif

#if !defined(AtomicUnlock)
    #define AtomicUnlock AtomicUnlock_Inline

    static inline void AtomicUnlock(volatile int32_t* ptr) {
        __sync_lock_release(ptr);
    }
#endif

#if !defined(AtomicGet)
    #define AtomicGet AtomicGet_Inline

    static inline int32_t AtomicGet_Inline(volatile int32_t* ptr) {
        #if defined(OSAtomicCompareAndSwap32)
            int32_t old_val = *ptr;
            if (OSAtomicCompareAndSwap32(0, 0, ptr))
                return 0;
            return old_val;
        #else
            return __sync_val_compare_and_swap(ptr, 0, 0);
        #endif
    }
#endif

#if !defined(AtomicExchange)
    #define AtomicExchange AtomicExchange_Inline

    static inline int32_t AtomicExchange_Inline(volatile int32_t* ptr, int32_t val) {
        int32_t old_val;
        do {
            old_val = *ptr;
        }
        #if defined(OSAtomicCompareAndSwap32)
            while (!OSAtomicCompareAndSwap32(old_val, val, ptr));
        #else
            while (__sync_val_compare_and_swap(ptr, old_val, val) != old_val);
        #endif
        return old_val;
    }
#endif

#if !defined(AtomicSet)
    #define AtomicSet AtomicExchange
#endif

#if !defined(AtomicCompareExchange)
    #define AtomicCompareExchange AtomicCompareExchange_Inline

    static inline int32_t AtomicCompareExchange_Inline(volatile int32_t* ptr, int32_t val, int32_t compare) {
        #if defined(OSAtomicCompareAndSwap32)
            int32_t old_val = *ptr;
            if (OSAtomicCompareAndSwap32(compare, val, ptr))
                return compare;
            return old_val;
        #else
            return __sync_val_compare_and_swap(ptr, compare, val);
        #endif
    }
#endif

#if !defined(AtomicAdd)
    #define AtomicAdd AtomicAdd_Inline

    static inline int32_t AtomicAdd_Inline(volatile int32_t* ptr, int32_t val) {
        #if defined(OSAtomicAdd32)
            return OSAtomicAdd32(val, ptr);
        #else
            return __sync_add_and_fetch(ptr, val);
        #endif
    }
#endif

#if !defined(AtomicSub)
    #define AtomicSub AtomicSub_Inline

    static inline int32_t AtomicSub_Inline(volatile int32_t* ptr, int32_t val) {
        #if defined(OSAtomicAdd32)
            return OSAtomicAdd32(-val, ptr);
        #else
            return __sync_sub_and_fetch(ptr, val);
        #endif
    }
#endif

#if !defined(AtomicIncrement)
    #define AtomicIncrement AtomicIncrement_Inline

    static inline int32_t AtomicIncrement_Inline(volatile int32_t* ptr) {
        #if defined(OSAtomicIncrement32)
            return OSAtomicIncrement32(ptr);
        #else
            return __sync_add_and_fetch(ptr, (int32_t)1);
        #endif
    }
#endif

#if !defined(AtomicDecrement)
    #define AtomicDecrement AtomicDecrement_Inline

    static inline int32_t AtomicDecrement_Inline(volatile int32_t* ptr) {
        #if defined(OSAtomicDecrement32)
            return OSAtomicDecrement32(ptr);
        #else
            return __sync_sub_and_fetch(ptr, (int32_t)1);
        #endif
    }
#endif

#if !defined(AtomicOr)
    #define AtomicOr AtomicOr_Inline

    static inline int32_t AtomicOr_Inline(volatile int32_t* ptr, int32_t val) {
        #if defined(OSAtomicOr32)
            return OSAtomicOr32(val, ptr);
        #else
            return __sync_or_and_fetch(ptr, val);
        #endif
    }
#endif

#if !defined(AtomicXor)
    #define AtomicXor AtomicXor_Inline

    static inline int32_t AtomicXor_Inline(volatile int32_t* ptr, int32_t val) {
        #if defined(OSAtomicXor32)
            return OSAtomicXor32(val, ptr);
        #else
            return __sync_xor_and_fetch(ptr, val);
        #endif
    }
#endif

#if !defined(AtomicNor)
    #define AtomicNor AtomicNor_Inline

    static inline int32_t AtomicNor_Inline(volatile int32_t* ptr, int32_t val) {
        #if defined(OSAtomicAnd32)
            return OSAtomicAnd32(~val, ptr);
        #else
            return __sync_and_and_fetch(ptr, ~val);
        #endif
    }
#endif

#if !defined(AtomicAnd)
    #define AtomicAnd AtomicAnd_Inline

    static inline int32_t AtomicAnd_Inline(volatile int32_t* ptr, int32_t val) {
        #if defined(OSAtomicAnd32)
            return OSAtomicAnd32(val, ptr);
        #else
            return __sync_and_and_fetch(ptr, val);
        #endif
    }
#endif

#if !defined(AtomicNand)
    #define AtomicNand AtomicNand_Inline

    static inline int32_t AtomicNand_Inline(volatile int32_t* ptr, int32_t val) {
        #if GCC_VERSION >= 40400
            return __sync_nand_and_fetch(ptr, val);
        #else
            int32_t i,j;
            j = *ptr;
            do {
                i = j;
                j = AtomicCompareExchange((int32_t*)ptr, ~(i & val), i);
            } while (i != j);
            return j;
        #endif
    }
#endif

#if !defined(AtomicGet64)
    #define AtomicGet64 AtomicGet64_Inline

    static inline int64_t AtomicGet64_Inline(volatile int64_t* ptr) {
        #if defined(OSAtomicCompareAndSwap64)
            int64_t old_val = *ptr;
            if (OSAtomicCompareAndSwap64(0, 0, ptr))
                return 0;
            return old_val;
        #else
            return __sync_val_compare_and_swap(ptr, 0, 0);
        #endif
    }
#endif

#if !defined(AtomicExchange64)
    #define AtomicExchange64 AtomicExchange64_Inline

    static inline int64_t AtomicExchange64_Inline(volatile int64_t* ptr, int64_t val) {
        int64_t old_val;
        do {
            old_val = *ptr;
        }
        #if defined(OSAtomicCompareAndSwap64)
            while (!OSAtomicCompareAndSwap64(old_val, val, ptr));
        #else
            while (__sync_val_compare_and_swap(ptr, old_val, val) != old_val);
        #endif
        return old_val;
    }
#endif

#if !defined(AtomicSet64)
    #define AtomicSet64 AtomicExchange64
#endif

#if !defined(AtomicCompareExchange64)
    #define AtomicCompareExchange64 AtomicCompareExchange64_Inline

    static inline int64_t AtomicCompareExchange64_Inline(volatile int64_t* ptr, int64_t val, int64_t compare) {
        #if defined(OSAtomicCompareAndSwap64)
            int64_t old_val = *ptr;
            if (OSAtomicCompareAndSwap64(compare, val, ptr))
                return compare;
            return old_val;
        #else
            return __sync_val_compare_and_swap(ptr, compare, val);
        #endif
    }
#endif

#if !defined(AtomicAdd64)
    #define AtomicAdd64 AtomicAdd64_Inline

    static inline int64_t AtomicAdd64_Inline(volatile int64_t* ptr, int64_t val) {
        #if defined(OSAtomicAdd64)
            return OSAtomicAdd64(val, ptr);
        #else
            return __sync_add_and_fetch(ptr, val);
        #endif
    }
#endif

#if !defined(AtomicSub64)
    #define AtomicSub64 AtomicSub64_Inline

    static inline int64_t AtomicSub64_Inline(volatile int64_t* ptr, int64_t val) {
        #if defined(OSAtomicAdd64)
            return OSAtomicAdd64(-val, ptr);
        #else
            return __sync_sub_and_fetch(ptr, val);
        #endif
    }
#endif

#if !defined(AtomicIncrement64)
    #define AtomicIncrement64 AtomicIncrement64_Inline

    static inline int64_t AtomicIncrement64_Inline(volatile int64_t* ptr) {
        #if defined(OSAtomicIncrement64)
            return OSAtomicIncrement64(ptr);
        #else
            return __sync_add_and_fetch(ptr, (int64_t)1);
        #endif
    }
#endif

#if !defined(AtomicDecrement64)
    #define AtomicDecrement64 AtomicDecrement64_Inline

    static inline int64_t AtomicDecrement64_Inline(volatile int64_t* ptr) {
        #if defined(OSAtomicDecrement64)
            return OSAtomicDecrement64(ptr);
        #else
            return __sync_sub_and_fetch(ptr, (int64_t)1);
        #endif
    }
#endif

#if !defined(AtomicOr64)
    #define AtomicOr64 AtomicOr64_Inline

    static inline int64_t AtomicOr64_Inline(volatile int64_t* ptr, int64_t val) {
        #if defined(OSAtomicOr64)
            return OSAtomicOr64(val, ptr);
        #else
            return __sync_or_and_fetch(ptr, val);
        #endif
    }
#endif

#if !defined(AtomicXor64)
    #define AtomicXor64 AtomicXor64_Inline

    static inline int64_t AtomicXor64_Inline(volatile int64_t* ptr, int64_t val) {
        #if defined(OSAtomicXor64)
            return OSAtomicXor64(val, ptr);
        #else
            return __sync_xor_and_fetch(ptr, val);
        #endif
    }
#endif

#if !defined(AtomicNor64)
    #define AtomicNor64 AtomicNor64_Inline

    static inline int64_t AtomicNor64_Inline(volatile int64_t* ptr, int64_t val) {
        #if defined(OSAtomicAnd64)
            return OSAtomicAnd64(~val, ptr);
        #else
            return __sync_and_and_fetch(ptr, ~val);
        #endif
    }
#endif

#if !defined(AtomicAnd64)
    #define AtomicAnd64 AtomicAnd64_Inline

    static inline int64_t AtomicAnd64_Inline(volatile int64_t* ptr, int64_t val) {
        #if defined(OSAtomicAnd64)
            return OSAtomicAnd64(val, ptr);
        #else
            return __sync_and_and_fetch(ptr, val);
        #endif
    }
#endif

#if !defined(AtomicNand64)
    #define AtomicNand64 AtomicNand64_Inline

    static inline int64_t AtomicNand64_Inline(volatile int64_t* ptr, int64_t val) {
        #if GCC_VERSION >= 40400
            return __sync_nand_and_fetch(ptr, val);
        #else
            int64_t i,j;
            j = *ptr;
            do {
                i = j;
                j = AtomicCompareExchange64((int64_t*)ptr, ~(i & val), i);
            } while (i != j);
            return j;
        #endif
    }
#endif

#if !defined(AtomicGetPtr)
    #define AtomicGetPtr AtomicGetPtr_Inline

    static inline void* AtomicGetPtr_Inline(void* volatile *ptr) {
        #if defined(OSAtomicCompareAndSwap64)
            int64_t old_val = *ptr;
            if (OSAtomicCompareAndSwap64(NULL, NULL, ptr))
                return NULL;
            return old_val;
        #else
            return __sync_val_compare_and_swap(ptr, NULL, NULL);
        #endif
    }
#endif

#if !defined(AtomicExchangePtr)
    #define AtomicExchangePtr AtomicExchangePtr_Inline

    static inline void* AtomicExchangePtr_Inline(void* volatile *ptr, void* val) {
        void* old_val;
        do {
            old_val = *ptr;
        }
        #if defined(OSAtomicCompareAndSwap64)
            while (!OSAtomicCompareAndSwap64(old_val, val, ptr));
        #else
            while (__sync_val_compare_and_swap(ptr, old_val, val) != old_val);
        #endif
        return old_val;
    }
#endif

#if !defined(AtomicSetPtr)
    #define AtomicSetPtr AtomicExchangePtr
#endif

#if !defined(AtomicCompareExchangePtr)
    #define AtomicCompareExchangePtr AtomicCompareExchangePtr_Inline

    static inline void* AtomicCompareExchangePtr_Inline(void* volatile *ptr, void* val, void* compare) {
        #if defined(OSAtomicCompareAndSwap64)
            void* old_val = (void*)*ptr;
            if (OSAtomicCompareAndSwap64(compare, val, ptr))
                return compare;
            return old_val;
        #else
            return __sync_val_compare_and_swap(ptr, compare, val);
        #endif
    }
#endif

#endif //ANDROID_UATOMIC_H
