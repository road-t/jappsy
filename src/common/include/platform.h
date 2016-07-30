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

#define DEBUG
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

#endif //JAPPSY_PLATFORM_H

/* INCLUDES */

#if defined(__IOS__)
    #import <Foundation/Foundation.h>
#elif defined(__WINNT__)
    #include <stdint.h>
    #include <windows.h>
    #include <unistd.h>
    #include <string.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <time.h>
    #include <errno.h>

    #include <core/uError.h>

    #ifndef __cplusplus
        #ifndef bool
            #define bool int8_t
            #define false 0
            #define true 1
        #endif
    #endif
#elif defined(__JNI__)
    #include <jni.h>
    #include <stdint.h>
    #include <unistd.h>
    #include <string.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <time.h>

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
#endif

#ifndef LOG
    #ifdef DEBUG
        #if defined(__IOS__)
            #define LOG(f, ...) NSLog(@ ## f, ##__VA_ARGS__);
        #elif defined(__WINNT__)
            #define LOG(f, ...) printf(f, ##__VA_ARGS__);
        #elif defined(__JNI__)
            #include <android/log.h>
            #define LOG(f, ...) __android_log_print(ANDROID_LOG_INFO, "JNI", f, ##__VA_ARGS__);
        #endif
    #else
        #define LOG(f, ...)
    #endif
#endif

