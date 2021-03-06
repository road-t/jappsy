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

#include "jappsy.h"
#include <core/uMemory.h>
#include <core/uSystem.h>
#include <sound/uMixer.h>

CString* jappsyCacheDir = NULL;

jbool jappsyInitialized = false;

#ifdef __cplusplus
extern "C" {
#endif

void jappsyInit(const char* cacheDir, void* system) {
    if (!AtomicCompareExchange(&jappsyInitialized, true, false)) {
        LOG("Jappsy Init > Cache %s", cacheDir);

        mmInit();
        uSystemInit(system);
        jappsyCacheDir = new CString(cacheDir);
#if defined(__IOS__)
        initAudioPlayer();
#elif defined(__JNI__)
        initAudioPlayer();
#endif
    }
}

void jappsyQuit() {
    if (AtomicCompareExchange(&jappsyInitialized, false, true)) {
#ifdef DEBUG
        memLogStats(NULL, NULL, NULL, NULL);
#endif

#if defined(__IOS__)
        shutdownAudioPlayer();
#elif defined(__JNI__)
        shutdownAudioPlayer();
#endif
        delete jappsyCacheDir;
        uSystemQuit();
        mmQuit();

        LOG("Jappsy Quit");
    }
}

#ifdef __cplusplus
}
#endif
