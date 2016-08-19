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

#include <android/log.h>
#include <platform.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <malloc.h>
#include <jappsy.h>

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
Java_com_jappsy_JappsyEngine_mallinfo(JNIEnv *env, jclass type) {
	LOG("Memory:: Size: %lld, Alloc: %lld, Free %lld", (long long int)getNativeHeapSize(), (long long int)getNativeHeapAllocatedSize(), (long long int)getNativeHeapFreeSize());
}

JNIEXPORT jboolean JNICALL
Java_com_jappsy_JappsyEngine_initialize(JNIEnv *env, jclass type) {
	static uint16_t x = 0x0001;
	jint result = (jint)*((uint8_t*)&x);

	jappsyInit();

	if (result == 0) {
		__android_log_print(ANDROID_LOG_ERROR, "JNI", "Unsupported Platform!");
		return JNI_FALSE;
	}

	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "Start");

	return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_free(JNIEnv *env, jclass type) {
	jappsyQuit();

	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "Stop");
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onCreate(JNIEnv *env, jclass type) {
	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "onCreate");
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onDestroy(JNIEnv *env, jclass type) {
	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "onDestroy");
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onUpdate(JNIEnv *env, jclass type, jint width, jint height) {
	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "onUpdate");

	glViewport(0, 0, width, height);
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onPause(JNIEnv *env, jclass type) {
	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "onPause");
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onResume(JNIEnv *env, jclass type) {
	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "onResume");
}

static int color = 0;

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onFrame(JNIEnv *env, jclass type) {
	//__android_log_print(ANDROID_LOG_DEBUG, "JNI", "onFrame");

	float c = (float)color / 255.0f;
	glClearColor(c, c, c, 1.0f);
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	color++;
	if (color >= 256) color = 0;
}
