/*
 * Created by VipDev on 01.04.16.
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

#include "include/platform.h"
//#include <jni.h>
//#include <stdint.h>
//#include <string.h>
//#include <stdio.h>

#include <jappsy.h>

#include <zlib.h>
#include <png.h>

#include <malloc.h>
#include <core/uMemory.h>

static uint64_t getNativeHeapSize() {
    struct mallinfo info = mallinfo();
    return (uint64_t)info.usmblks;
}

static uint64_t getNativeHeapAllocatedSize() {
    struct mallinfo info = mallinfo();
    return (uint64_t)info.uordblks;
}

static uint64_t getNativeHeapFreeSize() {
    struct mallinfo info = mallinfo();
    return (uint64_t)info.fordblks;
}

JNIEXPORT void JNICALL
Java_com_jappsy_Jappsy_malinfo(JNIEnv *env, jclass type) {
    LOG("Memory:: Size: %lld, Alloc: %lld, Free %lld", (long long int)getNativeHeapSize(), (long long int)getNativeHeapAllocatedSize(), (long long int)getNativeHeapFreeSize());
}

JNIEXPORT jboolean JNICALL
Java_com_jappsy_Jappsy_initialize(JNIEnv *env, jclass type) {
    static uint16_t x = 0x0001;
    jint result = (jint)*((uint8_t*)&x);

    jappsyInit();

    if (result == 0) {
        __android_log_print(ANDROID_LOG_ERROR, "JNI", "Unsupported Platform!");
        return JNI_FALSE;
    }

    __android_log_print(ANDROID_LOG_INFO, "JNI", "Start");

    return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_jappsy_Jappsy_free(JNIEnv *env, jclass type) {
    jappsyQuit();

    __android_log_print(ANDROID_LOG_INFO, "JNI", "Stop");
}

