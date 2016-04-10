/*
 * Created by VipDev on 10.04.16.
 *
 * Copyright (C) 2016 The Jappsy Open Source Project
 *
 * Project Web Page http://jappsy.com
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

#ifndef ANDROID_USYSTEM_H
#define ANDROID_USYSTEM_H

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

void uSystemInit();
void uSystemQuit();

#if defined(__WINNT__)
    extern LARGE_INTEGER uSystem_Frequency;
    extern uint64_t uSystem_SystemTimeShiftNS;
    extern uint64_t uSystem_SystemTimeShiftMS;
    extern LARGE_INTEGER uSystem_CounterShift;
#endif

#if !defined(nanoTime)
    #define nanoTime nanoTime_Inline

    static inline uint64_t nanoTime_Inline() {
    #if defined(__IOS__)
        return (uint64_t)floor([[NSDate date] timeIntervalSince1970] * 1000000000);
    #elif defined(__WINNT__)
        LARGE_INTEGER Counter;
        QueryPerformanceCounter(&Counter);
        return (Counter.QuadPart - CounterShift.QuadPart) * 1000000000LL / uSystem_Frequency.QuadPart + uSystem_SystemTimeShiftNS;
    #elif defined(__JNI__)
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return (uint64_t)ts.tv_sec * 1000000000LL + (uint64_t)ts.tv_nsec;
    #else
    #error TODO nanoTime
        return 0;
    #endif
    }
#endif

#if !defined(currentTimeMillis)
    #define currentTimeMillis currentTimeMillis_Inline

    static inline uint64_t currentTimeMillis_Inline() {
    #if defined(__IOS__)
        return (uint64_t)floor([[NSDate date] timeIntervalSince1970] * 1000);
    #elif defined(__WINNT__)
        LARGE_INTEGER Counter;
        QueryPerformanceCounter(&Counter);
        return (Counter.QuadPart - CounterShift.QuadPart) * 1000LL / uSystem_Frequency.QuadPart + uSystem_SystemTimeShiftMS;
    #elif defined(__JNI__)
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return ((uint64_t)ts.tv_sec * 1000LL) + ((uint64_t)ts.tv_nsec / 1000000LL);
    #else
    #error TODO currentTimeMillis
        return 0;
    #endif
    }
#endif

#if !defined(systemSleep)
    #define systemSleep systemSleep_Inline

    static inline void systemSleep_Inline(int ms) {
    #if defined(__IOS__)
        [NSThread sleepForTimeInterval:(double)ms / 1000.0];
    #elif defined(__WINNT__)
        Sleep(ms);
    #else
        sleep((unsigned int)ms);
    #endif
    }
#endif

#ifdef __cplusplus
}
#endif

#endif //ANDROID_USYSTEM_H
