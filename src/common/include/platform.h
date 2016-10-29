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

#ifndef JAPPSY_PLATFORM_H
#define JAPPSY_PLATFORM_H

#ifndef DEBUG
	#warning Remove Always Debug Mode
	#define DEBUG
#else
	//#undef DEBUG
#endif
#define JAPPSY_IO_RESTORE_FILE_POINTER

#if defined(_AIX)
    #error Unsupported Platform - IBM AIX
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
    #if defined(_DragonFly__) || defined(__FreeBSD__) || defined(_NetBSD__) || defined(__OpenBSD__)
        #error Unsupported Platform - BSD (DragonFly BSD, FreeBSD, OpenBSD, NetBSD)
    #elif defined(__APPLE__) && defined(__MACH__)
        #include <TargetConditionals.h>
        #if TARGET_IPHONE_SIMULATOR == 1
            /* iOS in Xcode simulator */
            #ifndef __IOS__
                #define __IOS__
            #endif
            #ifndef __SIMULATOR__
                #define __SIMULATOR__
            #endif
        #elif TARGET_OS_IPHONE == 1
            /* iOS on iPhone, iPad, etc. */
            #ifndef __IOS__
                #define __IOS__
            #endif
        #elif TARGET_OS_MAC == 1
            /* OSX */
            #ifndef __OSX__
                #define __OSX__
            #endif
        #endif
    #endif
#elif defined(__hpux)
    #error Unsupported Platform - Hewlett-Packard HP-UX
#elif defined(__linux__)
    /* Linux. --------------------------------------------------- */
    #ifndef __LINUX__
        #define __LINUX__
    #endif
#elif defined(__sun) && defined(__SVR4)
    #error Unsupported Platform - Solaris
#elif defined(__CYGWIN__) && !defined(_WIN32)
    #error Unsupported Platform - Cygwin POSIX under Microsoft Windows
#elif defined(_WIN64)
    /* Microsoft Windows (64-bit). ------------------------------ */
    #ifndef __WIN64__
        #define __WIN64__
    #endif
    #ifndef __WINNT__
        #define __WINNT__
    #endif
#elif defined(_WIN32)
    /* Microsoft Windows (32-bit). ------------------------------ */
    #ifndef __WIN32__
        #define __WIN32__
    #endif
    #ifndef __WINNT__
        #define __WINNT__
    #endif
#endif

#if !defined(__IOS__) && !defined(__OSX__) && !defined(__WINNT__)
    #if defined(__arm__)
        #if defined(__ARM_ARCH_7A__)
            #if defined(__ARM_NEON__)
                #if defined(__ARM_PCS_VFP)
                    #define ABI "armeabi-v7a/NEON (hard-float)"
                #else
                    #define ABI "armeabi-v7a/NEON"
                #endif
            #else
                #if defined(__ARM_PCS_VFP)
                    #define ABI "armeabi-v7a (hard-float)"
                #else
                    #define ABI "armeabi-v7a"
                #endif
            #endif
        #else
            #define ABI "armeabi"
        #endif
    #elif defined(__i386__)
        #define ABI "x86"
    #elif defined(__x86_64__)
        #define ABI "x86_64"
    #elif defined(__mips64)  /* mips64el-* toolchain defines __mips__ too */
        #define ABI "mips64"
    #elif defined(__mips__)
        #define ABI "mips"
    #elif defined(__aarch64__)
        #define ABI "arm64-v8a"
    #else
        #define ABI "unknown"
    #endif

    #ifdef ABI
        #define __JNI__
    #endif
#endif

#if defined(__GNUC__)
    #define GCC_VERSION (__GNUC__ * 10000 \
							+ __GNUC_MINOR__ * 100 \
                            + __GNUC_PATCHLEVEL__)
#endif

#if __x86_64__
	#define	__X64__
#elif defined(__LP64__)
	#define	__X64__
#else
	#define __X32__
#endif

#if defined(__IOS__) || defined(__JNI__) || defined(__WINNT__)
	#ifdef __cplusplus
		template <typename Owner, typename Type>
		class Property {
		protected:
			typedef Type (*getter)(const Owner& self);
			typedef Type (*setter)(Owner& self, Type value);
			Owner* m_owner;
			getter m_getter;
			setter m_setter;
		public:
			inline Property() : m_owner((Owner*)0), m_getter((getter)0), m_setter((setter)0) {}
			inline Property(Owner * const owner, getter getmethod, setter setmethod) : m_owner(owner), m_getter(getmethod), m_setter(setmethod) {}
			inline operator Type() const { if (m_getter) return m_getter(*m_owner); else return (Type)0; }
			inline Type operator =(Type data) { if (m_setter) return m_setter(*m_owner, data); else return (Type)0; }
			inline void initialize(Owner * const owner, getter getmethod, setter setmethod) { m_owner = owner, m_getter = getmethod; m_setter = setmethod; }
		};
	#endif
#endif

#endif //JAPPSY_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef JAPPSYLOG
	#define JAPPSYLOG

	#ifdef DEBUG
		#if defined(__IOS__)
			void CNSLog(const char* format, ...);

			#define LOG(f, ...) CNSLog(f, ##__VA_ARGS__);
        #elif defined(__WINNT__)
            #define LOG(f, ...) printf(f, ##__VA_ARGS__);
        #elif defined(__JNI__)
            #include <android/log.h>
			#include <sys/syscall.h>
            #define LOG(f, ...) __android_log_print(ANDROID_LOG_INFO, "JNI", "(%d) " f, syscall(__NR_gettid)/*(uint32_t)((intptr_t)(pthread_self()))*/, ##__VA_ARGS__);

			#define LOGMEM(ptr, blocks) { \
				LOG("DUMP: MEMORY %08X", (uint64_t)(intptr_t)ptr); \
				for (int i = 0; i < blocks; i++) { \
					LOG("DUMP: %04X : %08X %08X %08X %08X %08X %08X %08X %08", \
						i * 8 * 4, \
						__builtin_bswap32(*(uint32_t*)((uint8_t*)ptr + (i * 8 + 0) * 4)), \
						__builtin_bswap32(*(uint32_t*)((uint8_t*)ptr + (i * 8 + 1) * 4)), \
						__builtin_bswap32(*(uint32_t*)((uint8_t*)ptr + (i * 8 + 2) * 4)), \
						__builtin_bswap32(*(uint32_t*)((uint8_t*)ptr + (i * 8 + 3) * 4)), \
						__builtin_bswap32(*(uint32_t*)((uint8_t*)ptr + (i * 8 + 4) * 4)), \
						__builtin_bswap32(*(uint32_t*)((uint8_t*)ptr + (i * 8 + 5) * 4)), \
						__builtin_bswap32(*(uint32_t*)((uint8_t*)ptr + (i * 8 + 6) * 4)), \
						__builtin_bswap32(*(uint32_t*)((uint8_t*)ptr + (i * 8 + 7) * 4)) \
					); \
				} \
			}
        #endif
	#else
		#define LOG(f, ...)
	#endif
#endif

#ifdef __cplusplus
}
#endif

/* INCLUDES */

#if defined(__IOS__)
	#ifdef __OBJC__
		#import <Foundation/Foundation.h>
	#else
		#include <stdint.h>
		#include <unistd.h>
		#include <string.h>
		#include <stdlib.h>
		#include <stdio.h>
		#include <time.h>
	#endif

	#ifndef __cplusplus
		#ifndef bool
			#define bool int8_t
			#define false 0
			#define true 1
		#endif
	#endif

	#include <core/uError.h>

	#ifndef MAX_PATH
        #ifndef PATH_MAX
            #define PATH_MAX 4096
        #endif
        #define MAX_PATH PATH_MAX
    #endif

	#define null ((const void*)0)

#elif defined(__WINNT__)
    #include <stdint.h>
    #include <windows.h>
    #include <unistd.h>
    #include <string.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <time.h>
    #include <errno.h>

    #ifndef __cplusplus
        #ifndef bool
            #define bool int8_t
            #define false 0
            #define true 1
        #endif
    #endif

	#include <core/uError.h>

    #ifndef NULL
		#define NULL ((const void*)0)
	#endif

	#ifndef null
		#define null ((const void*)0)
	#endif
#elif defined(__JNI__)
    #include <jni.h>
    #include <stdint.h>
    #include <unistd.h>
    #include <string.h>
    #include <stdlib.h>
    #include <stdio.h>
	#include <stddef.h>
    #include <time.h>
	#include <pthread.h>

    #include <core/uError.h>

    #ifndef __cplusplus
        #ifndef bool
            #define bool int8_t
            #define false 0
            #define true 1
        #endif
    #endif

    #ifndef MAX_PATH
        #ifndef PATH_MAX
            #define PATH_MAX 4096
        #endif
        #define MAX_PATH PATH_MAX
	#endif

	#ifndef NULL
		#define NULL 0
	#endif

	#ifndef nullptr
		#define nullptr NULL
	#endif

	#ifndef null
		#define null ((void*)0)
	#endif
#endif
