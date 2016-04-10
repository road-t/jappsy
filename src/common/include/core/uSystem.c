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

#include "uSystem.h"

#if defined(__WINNT__)
    LARGE_INTEGER uSystem_Frequency;
    uint64_t uSystem_SystemTimeShiftNS;
    uint64_t uSystem_SystemTimeShiftMS;
    LARGE_INTEGER uSystem_CounterShift;
#endif

void uSystemInit() {
#if defined(__WINNT__)
    QueryPerformanceFrequency(&uSystem_Frequency);
    GetSystemTimeAsFileTime((FILETIME*)(&uSystem_SystemTimeShiftNS));
    QueryPerformanceCounter(&uSystem_CounterShift);
    uSystem_SystemTimeShiftNS -= 116444736000000000LL;
    uSystem_SystemTimeShiftMS = uSystem_SystemTimeShiftNS / 10000LL;
    uSystem_SystemTimeShiftNS *= 100LL;
#endif
}

void uSystemQuit() {

}
