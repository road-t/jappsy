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
#include <opengl/uGLEngine.h>

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

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_mallinfo(JNIEnv *env, jclass type) {
	LOG("Memory:: Size: %lld, Alloc: %lld, Free %lld", (long long int) getNativeHeapSize(),
		(long long int) getNativeHeapAllocatedSize(), (long long int) getNativeHeapFreeSize());
}

JNIEXPORT jboolean JNICALL
Java_com_jappsy_JappsyEngine_initialize(JNIEnv *env, jclass type, jstring cacheDir) {
	static uint16_t x = 0x0001;
	jint result = (jint) *((uint8_t *) &x);

	const char *nativeString = env->GetStringUTFChars(cacheDir, 0);
	jappsyInit(nativeString);
	env->ReleaseStringUTFChars(cacheDir, nativeString);

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

JNIEXPORT jlong JNICALL
Java_com_jappsy_JappsyEngine_onCreate(JNIEnv *env, jclass type) {
	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "onCreate");
/*
	try {
		GLEngine* engine = new GLEngine();
		return (jlong)(intptr_t)(engine);
	} catch (...) {
		return (jlong)NULL;
	}
 */
	return (jlong)NULL;
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onDestroy(JNIEnv *env, jclass type, jlong handle) {
	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "onDestroy");

	if (handle != 0) {
		try {
			GLEngine *engine = (GLEngine *)(intptr_t)(handle);
			delete engine;
		} catch (...) {
		}
	}
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onUpdate(JNIEnv *env, jclass type, jlong handle, jint width,
									  jint height) {
	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "onUpdate");

	glViewport(0, 0, width, height);

	if (handle != 0) {
		try {
			GLEngine *engine = (GLEngine *)(intptr_t)(handle);
			engine->onUpdate(width, height);
		} catch (...) {
		}
	}
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onPause(JNIEnv *env, jclass type, jlong handle) {
	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "onPause");
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onResume(JNIEnv *env, jclass type, jlong handle) {
	__android_log_print(ANDROID_LOG_DEBUG, "JNI", "onResume");
}

static int color = 0;

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onFrame(JNIEnv *env, jclass type, jlong handle) {
	//__android_log_print(ANDROID_LOG_DEBUG, "JNI", "onFrame");

	if (handle != 0) {
		try {
			GLEngine *engine = (GLEngine *)(intptr_t)(handle);
			engine->onRender();
		} catch (...) {
		}
	} else {
		float c = (float) color / 255.0f;
		glClearColor(c, c, c, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		color++;
		if (color >= 256) color = 0;
	}
}

#ifdef __cplusplus
}
#endif