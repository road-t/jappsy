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

#ifndef JAPPSY_UMOTIONEVENT_H
#define JAPPSY_UMOTIONEVENT_H

#include <platform.h>

struct MotionPointer {
	float x;
	float y;
	uint32_t event;
	int64_t time;
};

class MotionEvent {
private:
	int32_t* indexes;
	MotionPointer* pointers;
	uint32_t count;
	
public:
	constexpr static const uint32_t ACTION_NONE         = 0x00000000;
	constexpr static const uint32_t ACTION_CANCEL       = 0x00000004;
	constexpr static const uint32_t ACTION_DOWN         = 0x00000001;
	constexpr static const uint32_t ACTION_MOVE         = 0x00000003;
	constexpr static const uint32_t ACTION_OUTSIDE      = 0x00000005;
	constexpr static const uint32_t ACTION_UP           = 0x00000002;
	constexpr static const uint32_t ACTION_SCROLL       = 0x00000006;
	constexpr static const int32_t INVALID_POINTER_ID  = -1;
	
	bool multitouch = false;

	int32_t actionIndex = 0;
	uint32_t actionEvent = 0;
	
	MotionEvent();
	~MotionEvent();
	
	inline int32_t getPointerCount() const { return count; }
	inline int32_t getActionIndex() const { return actionIndex; }
	inline uint32_t getActionMasked() const { return actionEvent; }
	
	int32_t getPointerId(int32_t pointerIndex) const;
	float getX(int32_t pointerIndex) const;
	float getY(int32_t pointerIndex) const;
	MotionPointer* getPointer(int32_t pointerIndex) const;
	
	MotionPointer* insert(int32_t id) throw(const char*);
	void remove(int32_t pointerIndex);
};

#endif //JAPPSY_UGLMOTIONEVENT_H