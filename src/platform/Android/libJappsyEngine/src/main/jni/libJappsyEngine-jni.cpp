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
#include <sound/uMixer.h>

class JappsyEngine : public CObject {
public:
	jbool running = false;
	jbool stopping = false;
	GLEngine* engine = NULL;
};

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
	jappsyInit(nativeString, (void*)env);
	env->ReleaseStringUTFChars(cacheDir, nativeString);

	if (result == 0) {
		LOG("Unsupported Platform!");
		return JNI_FALSE;
	}

	//LOG("Start");

	return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_free(JNIEnv *env, jclass type) {
	jappsyQuit();

	//LOG("Stop");
}

void initOpenGLThreadId();
void releaseOpenGLThreadId();

JNIEXPORT jlong JNICALL
Java_com_jappsy_JappsyEngine_onCreate(JNIEnv *env, jclass type) {
	LOG("onCreate");

	try {
		JappsyEngine *context = new JappsyEngine();
		AtomicSet(&(context->running), false);
		AtomicSet(&(context->stopping), false);
		return (jlong) (intptr_t) (context);
	} catch (...) {
		return (jlong)NULL;
	}
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_setEngine(JNIEnv *env, jclass type, jlong handle, jlong engineHandle) {
	LOG("setEngine");

	if (handle != 0) {
		JappsyEngine *context = (JappsyEngine*)(intptr_t)(handle);
		AtomicSet(&(context->engine), (GLEngine*)(intptr_t)(engineHandle));
	}
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onStart(JNIEnv *env, jclass type, jlong handle) {
	if (handle != 0) {
		JappsyEngine *context = (JappsyEngine*)(intptr_t)(handle);
		if (!AtomicGet(&(context->stopping))) {
			LOG("onStart");

			initOpenGLThreadId();

			//Java_com_jappsy_JappsyEngine_onResume(env, type, handle)
		}
	}
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onPause(JNIEnv *env, jclass type, jlong handle) {
	if (handle != 0) {
		JappsyEngine *context = (JappsyEngine*)(intptr_t)(handle);
		if (!AtomicGet(&(context->stopping))) {
			if (AtomicCompareExchange(&(context->running), false, true)) {
				LOG("onPause");

				pauseAudioPlayer();
			}
		}
	}
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onResume(JNIEnv *env, jclass type, jlong handle) {
	if (handle != 0) {
		JappsyEngine *context = (JappsyEngine*)(intptr_t)(handle);
		if (!AtomicGet(&(context->stopping))) {
			if (!AtomicCompareExchange(&(context->running), true, false)) {
				LOG("onResume");

				resumeAudioPlayer();
			}
		}
	}
}

void* onShutdown(void* userData) {
	LOG("onShutdown");

	JappsyEngine *context = (JappsyEngine*)userData;
	GLEngine* engine = AtomicGet(&(context->engine));

	if (engine != NULL) {
		delete engine;
	}

	delete context;

	return NULL;
}

void* onShutdownRequest(void* userData) {
	LOG("onShutdownRequest");

	JappsyEngine *context = (JappsyEngine*)userData;
	GLEngine* engine = AtomicGet(&(context->engine));

	if (engine != NULL) {
		try {
			engine->shutdown();
		} catch (...) {
		}
	}

	MainThreadAsync(onShutdown, NULL, context);

	return NULL;
}

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onStop(JNIEnv *env, jclass type, jlong handle) {
	if (handle != 0) {
		JappsyEngine *context = (JappsyEngine*)(intptr_t)(handle);

		if (!AtomicGet(&(context->stopping))) {
			Java_com_jappsy_JappsyEngine_onPause(env, type, handle);
		}

		if (!AtomicCompareExchange(&(context->stopping), true, false)) {
			LOG("onStop");

			NewThreadAsync(onShutdownRequest, NULL, context);

			releaseOpenGLThreadId();
		}
	}
}

/*
JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onDestroy(JNIEnv *env, jclass type, jlong handle) {
	LOG("onDestroy");

	if (handle != 0) {
		Java_com_jappsy_JappsyEngine_onStop(env, type, handle);
	}
}
 */

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onUpdate(JNIEnv *env, jclass type, jlong handle, jint width,
									  jint height) {
	glViewport(0, 0, width, height);

	if (handle != 0) {
		JappsyEngine *context = (JappsyEngine*)(intptr_t)(handle);

		if (!AtomicGet(&(context->stopping))) {
			if (AtomicGet(&(context->running))) {
				LOG("onUpdate(%d,%d)", width, height);

				GLEngine *engine = AtomicGet(&(context->engine));
				if (engine != NULL) {
					try {
						engine->onUpdate(width, height);
					} catch (...) {
					}
				}
			}
		}
	}
}

static int color = 0;

JNIEXPORT void JNICALL
Java_com_jappsy_JappsyEngine_onFrame(JNIEnv *env, jclass type, jlong handle) {
	//LOG("onFrame");

	//LOG("OpenGL Thread Message Loop (Start)");
	OpenGLThreadMessageLooper();
	//LOG("OpenGL Thread Message Loop (End)");

	if (handle != 0) {
		JappsyEngine *context = (JappsyEngine *) (intptr_t) (handle);

		if (!AtomicGet(&(context->stopping))) {
			if (AtomicGet(&(context->running))) {
				GLEngine *engine = AtomicGet(&(context->engine));
				if (engine != NULL) {
					try {
						engine->onRender();
					} catch (...) {
					}
				}
			}
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
