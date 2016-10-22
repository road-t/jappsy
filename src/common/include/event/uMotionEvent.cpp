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

#include "uMotionEvent.h"
#include <core/uMemory.h>
#include <core/uSystem.h>
#include <opengl/uOpenGL.h>
#include <opengl/uGLEngine.h>

MotionEvent::MotionEvent() {
	indexes = NULL;
	pointers = NULL;
	count = 0;
}

MotionEvent::~MotionEvent() {
	if (count > 0) {
		memFree(indexes);
		indexes = NULL;
		memFree(pointers);
		pointers = NULL;
		
		count = 0;
	}
}

int32_t MotionEvent::getPointerId(int32_t pointerIndex) const {
	if ((pointerIndex >= 0) && (pointerIndex < count))
		return indexes[pointerIndex];
	
	return INVALID_POINTER_ID;
}

float MotionEvent::getX(int32_t pointerIndex) const {
	if ((pointerIndex >= 0) && (pointerIndex < count))
		return pointers[pointerIndex].x;
	return 0;
}

float MotionEvent::getY(int32_t pointerIndex) const {
	if ((pointerIndex >= 0) && (pointerIndex < count))
		return pointers[pointerIndex].y;
	return 0;
}

MotionPointer* MotionEvent::getPointer(int32_t pointerIndex) const {
	if ((pointerIndex >= 0) && (pointerIndex < count))
		return &(pointers[pointerIndex]);
	return NULL;
}

void MotionEvent::remove(int32_t pointerIndex) {
	if ((pointerIndex >= 0) && (pointerIndex < count)) {
		if (pointerIndex < count - 1) {
			memmove(&indexes[pointerIndex], &indexes[pointerIndex+1], (count-pointerIndex-1) * sizeof(int32_t));
			memmove(&pointers[pointerIndex], &pointers[pointerIndex+1], (count-pointerIndex-1) * sizeof(MotionPointer));
		}
		count--;
		
		if (count == 0) {
			memFree(indexes);
			indexes = NULL;
			memFree(pointers);
			pointers = NULL;
		}
	}
}

MotionPointer* MotionEvent::insert(int32_t id) throw(const char*) {
	int32_t* newIndexes = memRealloc(int32_t, newIndexes, indexes, (count+1) * sizeof(int32_t));
	MotionPointer* newPointers = memRealloc(MotionPointer, newPointers, pointers, (count+1) * sizeof(MotionPointer));
	
	if (newIndexes != NULL)
		indexes = newIndexes;
	if (newPointers != NULL)
		pointers = newPointers;

	if ((newIndexes == NULL) || (newPointers == NULL))
		throw eOutOfMemory;

	count++;

	indexes[count-1] = id;
	return &pointers[count-1];
}

int32_t MotionEvent::findPointerIndex(int32_t id) const {
	for (int i = 0; i < count; i++) {
		if (indexes[i] == id) {
			return i;
		}
	}

	return -1;
}

#if defined(__JNI__)

static const char _clsMotionEvent[] = "android/view/MotionEvent";
const char* clsMotionEvent = _clsMotionEvent;

struct clazzMotionEvent {
	jclass clazz;

	int ACTION_DOWN;
	int ACTION_POINTER_DOWN;
	int ACTION_MOVE;
	int ACTION_UP;
	int ACTION_POINTER_UP;
	int ACTION_CANCEL;

	jmethodID getActionIndex;
	jmethodID getPointerId;
	jmethodID getActionMasked;
	jmethodID getPointerCount;
	jmethodID getX;
	jmethodID getY;
};

static struct clazzMotionEvent jMotionEvent = {0};

static MotionEvent motionEvent;

void onMotionEvent(JNIEnv* env, jobject event, GLEngine* engine) {
	int pointerIndex = (int)env->CallIntMethod(event, jMotionEvent.getActionIndex);
	int pointerId = (int)env->CallIntMethod(event, jMotionEvent.getPointerId, (jint)pointerIndex);
	int maskedAction = (int)env->CallIntMethod(event, jMotionEvent.getActionMasked);
	int count = (int)env->CallIntMethod(event, jMotionEvent.getPointerCount);

	if ((maskedAction == jMotionEvent.ACTION_DOWN) || (maskedAction == jMotionEvent.ACTION_POINTER_DOWN)) {
		//LOG("MotionEvent: ACTION_DOWN");

		MotionPointer* pointer = motionEvent.insert(pointerId);
		if (pointer != NULL) {
			pointer->x = (GLfloat)env->CallFloatMethod(event, jMotionEvent.getX, (jint)pointerIndex);
			pointer->y = (GLfloat)env->CallFloatMethod(event, jMotionEvent.getY, (jint)pointerIndex);
			pointer->event = MotionEvent::ACTION_DOWN;
			pointer->time = currentTimeMillis();

			if (count > 1) {
				motionEvent.multitouch = true;
			}

			motionEvent.actionEvent = MotionEvent::ACTION_DOWN;
			motionEvent.actionIndex = motionEvent.getPointerCount() - 1;

			engine->onTouch(&motionEvent);
		}
	} else if (maskedAction == jMotionEvent.ACTION_MOVE) {
		//LOG("MotionEvent: ACTION_MOVE");

		for (int i = 0; i < count; i++) {
			pointerId = (int)env->CallIntMethod(event, jMotionEvent.getPointerId, (jint)i);
			int32_t index = motionEvent.findPointerIndex(pointerId);
			if (index >= 0) {
				MotionPointer* pointer = motionEvent.getPointer(index);
				pointer->x = (GLfloat)env->CallFloatMethod(event, jMotionEvent.getX, (jint)i);
				pointer->y = (GLfloat)env->CallFloatMethod(event, jMotionEvent.getY, (jint)i);
				pointer->event = MotionEvent::ACTION_MOVE;
				pointer->time = currentTimeMillis();
			}
		}

		motionEvent.actionEvent = MotionEvent::ACTION_MOVE;

		for (int i = 0; i < count; i++) {
			pointerId = (int)env->CallIntMethod(event, jMotionEvent.getPointerId, (jint)i);
			int32_t index = motionEvent.findPointerIndex(pointerId);
			if (index >= 0) {
				motionEvent.actionIndex = index;
				engine->onTouch(&motionEvent);
			}
		}
	} else if ((maskedAction == jMotionEvent.ACTION_UP) || (maskedAction == jMotionEvent.ACTION_POINTER_UP) || (maskedAction == jMotionEvent.ACTION_CANCEL)) {
		//LOG("MotionEvent: ACTION_UP");

		motionEvent.actionEvent = MotionEvent::ACTION_UP;

		int32_t index = motionEvent.findPointerIndex(pointerId);
		if (index >= 0) {
			MotionPointer* pointer = motionEvent.getPointer(index);
			pointer->x = (GLfloat)env->CallFloatMethod(event, jMotionEvent.getX, (jint)pointerIndex);
			pointer->y = (GLfloat)env->CallFloatMethod(event, jMotionEvent.getY, (jint)pointerIndex);
			pointer->event = MotionEvent::ACTION_UP;
			pointer->time = currentTimeMillis();

			motionEvent.actionIndex = index;
			engine->onTouch(&motionEvent);

			motionEvent.remove(index);
		}

		if (motionEvent.getPointerCount() == 0) {
			motionEvent.multitouch = false;
		}
	}
/*
	for (int i = 0; i < count; i++) {
		pointerId = (int)env->CallIntMethod(event, jMotionEvent.getPointerId, (jint)i);

		GLfloat x = (GLfloat)env->CallFloatMethod(event, jMotionEvent.getX, (jint)i);
		GLfloat y = (GLfloat)env->CallFloatMethod(event, jMotionEvent.getY, (jint)i);

		LOG("Pointer(%d): %d : %.2f x %.2f", i, pointerId, x, y);
	}
 */
}

#endif

void uMotionEventInit() {
#if defined(__JNI__)
	JNIEnv* env = GetThreadEnv();

	jclass clazz;

	clazz = env->FindClass(clsMotionEvent);
	if (env->ExceptionCheck()) {
		LOG("JNIEnv: FindClass %s (Fail)", clsMotionEvent);
		jMotionEvent.clazz = NULL;
	} else {
		jfieldID jfACTION_DOWN = env->GetStaticFieldID(clazz, "ACTION_DOWN", "I");
		jfieldID jfACTION_POINTER_DOWN = env->GetStaticFieldID(clazz, "ACTION_POINTER_DOWN", "I");
		jfieldID jfACTION_MOVE = env->GetStaticFieldID(clazz, "ACTION_MOVE", "I");
		jfieldID jfACTION_UP = env->GetStaticFieldID(clazz, "ACTION_UP", "I");
		jfieldID jfACTION_POINTER_UP = env->GetStaticFieldID(clazz, "ACTION_POINTER_UP", "I");
		jfieldID jfACTION_CANCEL = env->GetStaticFieldID(clazz, "ACTION_CANCEL", "I");

		jMotionEvent.getActionIndex = env->GetMethodID(clazz, "getActionIndex", "()I");
		jMotionEvent.getPointerId = env->GetMethodID(clazz, "getPointerId", "(I)I");
		jMotionEvent.getActionMasked = env->GetMethodID(clazz, "getActionMasked", "()I");
		jMotionEvent.getPointerCount = env->GetMethodID(clazz, "getPointerCount", "()I");
		jMotionEvent.getX = env->GetMethodID(clazz, "getX", "(I)F");
		jMotionEvent.getY = env->GetMethodID(clazz, "getY", "(I)F");

		if ((jfACTION_DOWN != NULL) && (jfACTION_POINTER_DOWN != NULL) && (jfACTION_MOVE != NULL) &&
				(jfACTION_UP != NULL) && (jfACTION_POINTER_UP != NULL) && (jfACTION_CANCEL != NULL) &&
				(jMotionEvent.getActionIndex != NULL) &&
				(jMotionEvent.getPointerId != NULL) &&
				(jMotionEvent.getActionMasked != NULL) &&
				(jMotionEvent.getPointerCount != NULL) &&
				(jMotionEvent.getX != NULL) &&
				(jMotionEvent.getY != NULL)) {
			LOG("JNIEnv: FindClass %s (OK)", clsMotionEvent);

			jMotionEvent.clazz = (jclass)(env->NewGlobalRef(clazz));

			jMotionEvent.ACTION_DOWN = (int)env->GetStaticIntField(clazz, jfACTION_DOWN);
			jMotionEvent.ACTION_POINTER_DOWN = (int)env->GetStaticIntField(clazz, jfACTION_POINTER_DOWN);
			jMotionEvent.ACTION_MOVE = (int)env->GetStaticIntField(clazz, jfACTION_MOVE);
			jMotionEvent.ACTION_UP = (int)env->GetStaticIntField(clazz, jfACTION_UP);
			jMotionEvent.ACTION_POINTER_UP = (int)env->GetStaticIntField(clazz, jfACTION_POINTER_UP);
			jMotionEvent.ACTION_CANCEL = (int)env->GetStaticIntField(clazz, jfACTION_CANCEL);
		} else {
			LOG("JNIEnv: FindClass %s (Unknown)", clsMotionEvent);
		}
	}

	ReleaseThreadEnv();
#endif
}

void uMotionEventQuit() {
#if defined(__JNI__)
	JNIEnv* env = GetThreadEnv();

	if (jMotionEvent.clazz != NULL) {
		env->DeleteGlobalRef(jMotionEvent.clazz);
		jMotionEvent.clazz = NULL;
	}

	ReleaseThreadEnv();
#endif
}
