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

#ifndef JAPPSY_UGLTOUCHSCREEN_H
#define JAPPSY_UGLTOUCHSCREEN_H

#include <platform.h>
#include <opengl/uOpenGL.h>
#include <core/uHandler.h>
#include <event/uMotionEvent.h>

class GLRender;

class GLTouchEvent {
public:
	typedef bool (*Callback)(const wchar_t* event);

	float x;
	float y;
	float w;
	float h;
	wchar_t* name;
	Callback onEvent;
};

class GLTouchScreen {
public:
	typedef void (*onTouchCallback)(const wchar_t* event);

private:
	Handler handler = new Handler();
	
	GLRender* context;
	onTouchCallback onTouch;
	GLfloat recordDistance;
	GLfloat minimalDistance;
	GLfloat swipeDistance;
	GLTouchEvent* clickList;
	GLTouchEvent* trackList;
	
	GLTouchEvent* trackLast;
	GLfloat trackSpeed;
	GLTouchEvent* trackingList;
	GLTouchEvent* trackingLast;
	
	GLTouchEvent* touchList;
	GLTouchEvent* touchLast;
	bool touchCancel;
	uint64_t touchStart;
	
	uint64_t touchTime;
	bool touchDown;
	uint64_t mouseTime;
	bool mouseDown;
	uint64_t mouseRepeatTime;
	void* touchTimeout;
	
	void setTimeout(int delay);
	void clearTimeout();

	bool checkBounds(float x, float y);

	bool canStartTouch();
	bool canStartMouse();
	
	void analyze(float x, float y);
	void recordTrack(float x, float y, bool stop);
	void record(float x, float y);
public:
	GLTouchScreen(GLRender* context, onTouchCallback callback);
	~GLTouchScreen();
	
	void update();
	
	void trackEvent(const wchar_t* name, float x, float y, float w, float h, GLTouchEvent::Callback callback);
	void clickEvent(const wchar_t* name, float x, float y, float w, float h, GLTouchEvent::Callback callback);
	
	void onTimeout();

	void onTouchStart(MotionEvent* event);
	void onTouchEnd(MotionEvent* event);
	void onTouchMove(MotionEvent* event);
	void onMouseDown(MotionEvent* event);
	void onMouseUp(MotionEvent* event);
	void onMouseOut(MotionEvent* event);
	void onMouseMove(MotionEvent* event);
};

#endif //JAPPSY_UGLTOUCHSCREEN_H