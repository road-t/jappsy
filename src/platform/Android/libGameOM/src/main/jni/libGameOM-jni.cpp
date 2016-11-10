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
#include "include/OMGame.h"

#ifdef __cplusplus
extern "C" {
#endif

struct onUpdateStateThreadData {
	jobject omView;
	int state;
};

void* onUpdateStateThread(void* userData) {
	onUpdateStateThreadData* thread = (onUpdateStateThreadData*)userData;

	JNIEnv* env = GetThreadEnv();

	jclass clazz = env->GetObjectClass(thread->omView);
	jmethodID method = env->GetMethodID(clazz, "updateState", "(I)V");
	env->CallVoidMethod(thread->omView, method, (jint) thread->state);
	env->ExceptionCheck();

	ReleaseThreadEnv();

	return NULL;
}

void onUpdateState(int state, void* userData) {
	onUpdateStateThreadData thread;
	thread.omView = (jobject)userData;
	thread.state = state;
	MainThreadSync(onUpdateStateThread, &thread);
}

class onLocationThreadData {
public:
	jobject omView;
	int index;
	CString url;
};

void* onLocationThread(void* userData) {
	onLocationThreadData* thread = (onLocationThreadData*)userData;

	JNIEnv* env = GetThreadEnv();

	jclass clazz = env->GetObjectClass(thread->omView);
	jmethodID method = env->GetMethodID(clazz, "onLocation", "(ILjava/lang/String;)V");
	env->CallVoidMethod(thread->omView, method, (jint) thread->index, thread->url.toJString(env));
	env->ExceptionCheck();

	ReleaseThreadEnv();

	memDelete(thread);
	return NULL;
}

void onLocation(int index, const CString& url, void* userData) {
	onLocationThreadData* thread = memNew(thread, onLocationThreadData);
	thread->omView = (jobject)userData;
	thread->index = index;
	thread->url = url;
	MainThreadAsync(onLocationThread, NULL, thread);
}

class onScriptThreadData {
public:
	jobject omView;
	int index;
	CString script;
};

void* onScriptThread(void* userData) {
	onScriptThreadData* thread = (onScriptThreadData*)userData;

	JNIEnv* env = GetThreadEnv();

	jclass clazz = env->GetObjectClass(thread->omView);
	jmethodID method = env->GetMethodID(clazz, "onScript", "(ILjava/lang/String;)V");
	env->CallVoidMethod(thread->omView, method, (jint) thread->index, thread->script.toJString(env));
	env->ExceptionCheck();

	ReleaseThreadEnv();

	memDelete(thread);
	return NULL;
}

void onScript(int index, const CString& script, void* userData) {
	onScriptThreadData* thread = memNew(thread, onScriptThreadData);
	thread->omView = (jobject)userData;
	thread->index = index;
	thread->script = script;
	MainThreadAsync(onScriptThread, NULL, thread);
}

JNIEXPORT jlong JNICALL
Java_com_jappsy_OMEngine_onCreate(JNIEnv *env, jclass type, jobject view, jstring basePath_,
								  jstring token_, jstring sessid_, jstring devid_,
								  jstring locale_) {

	LOG("onCreate");

	const char *basePath = env->GetStringUTFChars(basePath_, 0);
	const char *token = env->GetStringUTFChars(token_, 0);
	const char *sessid = env->GetStringUTFChars(sessid_, 0);
	const char *devid = env->GetStringUTFChars(devid_, 0);
	const char *locale = env->GetStringUTFChars(locale_, 0);

	try {
		OMGame *engine = new OMGame(
				basePath,
				token,
				sessid,
				devid,
				locale
		);

		engine->setOnUpdateState(onUpdateState, env->NewGlobalRef(view));
		engine->setWebCallbacks(onLocation, onScript, env->NewGlobalRef(view));

		env->ReleaseStringUTFChars(basePath_, basePath);
		env->ReleaseStringUTFChars(token_, token);
		env->ReleaseStringUTFChars(sessid_, sessid);
		env->ReleaseStringUTFChars(devid_, devid);
		env->ReleaseStringUTFChars(locale_, locale);

		return (jlong) (intptr_t) (engine);
	} catch (...) {
	}

	env->ReleaseStringUTFChars(basePath_, basePath);
	env->ReleaseStringUTFChars(token_, token);
	env->ReleaseStringUTFChars(sessid_, sessid);
	env->ReleaseStringUTFChars(devid_, devid);
	env->ReleaseStringUTFChars(locale_, locale);

	return (jlong) NULL;
}

JNIEXPORT void JNICALL
Java_com_jappsy_OMEngine_setMinimized(JNIEnv *env, jclass type, jlong engineptr, jboolean minimized) {
	OMGame *engine = (OMGame*)(intptr_t)(engineptr);

	if (engine != NULL) {
		engine->minimized = (minimized == JNI_TRUE);
	}
}

JNIEXPORT void JNICALL
Java_com_jappsy_OMEngine_onWebLocation(JNIEnv *env, jclass type, jlong engineptr, jint index,
									   jstring location_) {
	OMGame *engine = (OMGame*)(intptr_t)(engineptr);

	if (engine != NULL) {
		const char *location = env->GetStringUTFChars(location_, 0);
		CString url = location;
		env->ReleaseStringUTFChars(location_, location);

		engine->onWebLocation(index, url);
	}
}

JNIEXPORT void JNICALL
Java_com_jappsy_OMEngine_onWebReady(JNIEnv *env, jclass type, jlong engineptr, jint index) {
	OMGame *engine = (OMGame*)(intptr_t)(engineptr);

	if (engine != NULL) {
		engine->onWebReady(index);
	}
}

JNIEXPORT void JNICALL
Java_com_jappsy_OMEngine_onWebFail(JNIEnv *env, jclass type, jlong engineptr, jint index,
								   jstring error_) {
	OMGame *engine = (OMGame*)(intptr_t)(engineptr);

	if (engine != NULL) {
		const char *error = env->GetStringUTFChars(error_, 0);
		CString err = error;
		env->ReleaseStringUTFChars(error_, error);

		engine->onWebFail(index, err);
	}
}

#ifdef __cplusplus
}
#endif
