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

MotionEvent::MotionEvent() {
	indexes = NULL;
	pointers = NULL;
	count = 0;
}

MotionEvent::~MotionEvent() {
	if (count > 0) {
		mmfree(indexes);
		indexes = NULL;
		mmfree(pointers);
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
			mmfree(indexes);
			indexes = NULL;
			mmfree(pointers);
			pointers = NULL;
		}
	}
}

MotionPointer* MotionEvent::insert(int32_t id) throw(const char*) {
	int32_t* newIndexes = (int32_t*)mmrealloc(indexes, (count+1) * sizeof(int32_t));
	MotionPointer* newPointers = (MotionPointer*)mmrealloc(pointers, (count+1) * sizeof(MotionPointer));
	
	if (newIndexes != NULL)
		indexes = newIndexes;
	if (newPointers != NULL)
		pointers = newPointers;

	if ((newIndexes == NULL) || (newPointers == NULL))
		throw eOutOfMemory;

	count++;
	
	return &pointers[count-1];
}
