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

#ifndef JAPPSY_UATOMIC_H
#define JAPPSY_UATOMIC_H

#include <platform.h>

/** Functions
    void AtomicLock(volatile int32_t* ptr);
	bool AtomicLockTry(volatile int32_t* ptr);
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

#if defined(__atomic_test_and_set)

	typedef bool jlock;
	typedef bool jbool;

    #define AtomicLock(ptr) while (__atomic_test_and_set(ptr, __ATOMIC_ACQ_REL)) sleep(1)
    #define AtomicFastLock(ptr) while (__atomic_test_and_set(ptr, __ATOMIC_ACQ_REL)) sleep(0)
	#define AtomicLockTry(ptr) (__atomic_test_and_set(ptr, __ATOMIC_ACQ_REL) == false)
    #define AtomicUnlock(ptr) __atomic_clear(ptr, __ATOMIC_RELEASE)
    #define AtomicGet(ptr) __atomic_load_n(ptr, __ATOMIC_ACQUIRE)
    #define AtomicExchange(ptr, val) __atomic_exchange_n(ptr, val, __ATOMIC_ACQ_REL)
    #define AtomicSet(ptr, val) __atomic_exchange_n(ptr, val, __ATOMIC_ACQ_REL)

	#if !defined(AtomicCompareExchange)
		#define AtomicCompareExchange AtomicCompareExchange_Inline

		static inline int32_t AtomicCompareExchange_Inline(volatile int32_t* ptr, int32_t val, int32_t compare) {
			(void)__atomic_compare_exchange_n(ptr, &compare, val, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
			return compare;
		}
	#endif

    #define AtomicAdd(ptr, val) __atomic_fetch_add(ptr, val, __ATOMIC_ACQ_REL)
    #define AtomicSub(ptr, val) __atomic_fetch_sub(ptr, val, __ATOMIC_ACQ_REL)
    #define AtomicIncrement(ptr) __atomic_fetch_add(ptr, 1, __ATOMIC_ACQ_REL)
    #define AtomicDecrement(ptr) __atomic_fetch_sub(ptr, 1, __ATOMIC_ACQ_REL)
    #define AtomicOr(ptr, val) __atomic_fetch_or(ptr, val, __ATOMIC_ACQ_REL)
    #define AtomicXor(ptr, val) __atomic_fetch_xor(ptr, val, __ATOMIC_ACQ_REL)
	#define AtomicNor(ptr, val) __atomic_fetch_and(ptr, ~val, __ATOMIC_ACQ_REL)
    #define AtomicAnd(ptr, val) __atomic_fetch_and(ptr, val, __ATOMIC_ACQ_REL)

	#if GCC_VERSION >= 40400
		#define AtomicNand(ptr, val) __atomic_fetch_nand(ptr, val, __ATOMIC_ACQ_REL)
	#else
		#define AtomicNand AtomicNand_Inline

		static inline int32_t AtomicNand_Inline(volatile int32_t* ptr, int32_t val) {
			#if GCC_VERSION >= 40400
				return __sync_fetch_and_nand(ptr, val);
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

    #define AtomicGetPtr(ptr) __atomic_load_n(ptr, __ATOMIC_ACQUIRE)
    #define AtomicExchangePtr(ptr, val) __atomic_exchange_n(ptr, val, __ATOMIC_ACQ_REL)
    #define AtomicSetPtr AtomicExchangePtr

	#if !defined(AtomicCompareExchangePtr)
		#define AtomicCompareExchangePtr AtomicCompareExchangePtr_Inline

		static inline void* AtomicCompareExchangePtr_Inline(void* volatile *ptr, void* val, void* compare) {
			(void)__atomic_compare_exchange_n((void* volatile *)ptr, &compare, val, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
			return compare;
		}
	#endif

#else // Old GCC

	typedef int32_t jlock;
	typedef int32_t jbool;

	#define AtomicLock(ptr) while (__sync_lock_test_and_set(ptr, 1) != 0) sleep(1)
    #define AtomicFastLock(ptr) while (__sync_lock_test_and_set(ptr, 1) != 0) sleep(0)
	#define AtomicLockTry(ptr) (__sync_lock_test_and_set(ptr, 1) == 0)
    #define AtomicUnlock(ptr) __sync_lock_release(ptr)
    #define AtomicGet(ptr) __sync_val_compare_and_swap(ptr, 0, 0)

	#if !defined(AtomicExchange)
		#define AtomicExchange AtomicExchange_Inline

		static inline int32_t AtomicExchange_Inline(volatile int32_t* ptr, int32_t val) {
			int32_t old_val;
			do {
				old_val = *ptr;
			}
			while (__sync_val_compare_and_swap(ptr, old_val, val) != old_val);
			return old_val;
		}
	#endif

	#if !defined(AtomicSet)
		#define AtomicSet AtomicExchange
	#endif

	#define AtomicCompareExchange(ptr, val, compare) __sync_val_compare_and_swap(ptr, compare, val)

    #define AtomicAdd(ptr, val) __sync_fetch_and_add(ptr, val)
    #define AtomicSub(ptr, val) __sync_fetch_and_sub(ptr, val)
    #define AtomicIncrement(ptr) __sync_fetch_and_add(ptr, 1)
    #define AtomicDecrement(ptr) __sync_fetch_and_sub(ptr, 1)
    #define AtomicOr(ptr, val) __sync_fetch_and_or(ptr, val)
    #define AtomicXor(ptr, val) __sync_fetch_and_xor(ptr, val)
    #define AtomicNor(ptr, val) __sync_fetch_and_and(ptr, ~val)
    #define AtomicAnd(ptr, val) __sync_fetch_and_and(ptr, val)

	#if !defined(AtomicNand)
		#define AtomicNand AtomicNand_Inline

		static inline int32_t AtomicNand_Inline(volatile int32_t* ptr, int32_t val) {
			#if GCC_VERSION >= 40400
				return __sync_fetch_and_nand(ptr, val);
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

	#define AtomicGetPtr(ptr) __sync_val_compare_and_swap(ptr, NULL, NULL)

	#if !defined(AtomicExchangePtr)
		#define AtomicExchangePtr(ptr, val) AtomicExchangePtr_Inline((void* volatile *)(ptr), val)

		static inline void* AtomicExchangePtr_Inline(void* volatile *ptr, void* val) {
			void* old_val;
			do {
				old_val = *ptr;
			}
			while (__sync_val_compare_and_swap(ptr, old_val, val) != old_val);
			return old_val;
		}
	#endif

	#if !defined(AtomicSetPtr)
		#define AtomicSetPtr AtomicExchangePtr
	#endif

	#define AtomicCompareExchangePtr(ptr, val, compare) __sync_val_compare_and_swap(ptr, compare, val)

#endif

#endif //JAPPSY_UATOMIC_H
