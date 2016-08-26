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
#include <data/uObject.h>
#include <event/uMotionEvent.h>
#include <data/uVector.h>

class GLRender;

class GLTouchPoint : public CObject {
public:
	union {
		struct {
			GLfloat x;
			GLfloat y;
		};
		
		GLfloat v[2];
	};
	
	uint64_t time;
	
	inline GLTouchPoint() { }
	
	inline GLTouchPoint(const GLfloat x, const GLfloat y, const uint64_t time) {
		this->x = x;
		this->y = y;
		this->time = time;
	}
	
	inline void update(const GLfloat x, const GLfloat y, const uint64_t time) {
		this->x = x;
		this->y = y;
		this->time = time;
	}
	
	GLTouchPoint& operator =(const GLTouchPoint& point) {
		this->x = point.x;
		this->y = point.y;
		this->time = point.time;
		return *this;
	}
	
	bool operator ==(const GLTouchPoint& point) {
		return (this->x == point.x) && (this->y == point.y) && (this->time == point.time);
	}
};

class GLTouchObject : public CObject {
public:
	typedef bool (*Callback)(const CString& event, const GLTouchPoint* cur, const GLTouchPoint* delta, const GLTouchPoint* speed, void* userData);
	
	CString name;
	
	GLfloat x;
	GLfloat y;
	GLfloat w;
	GLfloat h;
	
	Callback onevent;
	void* userData;
	
	inline GLTouchObject(const CString& name, const GLfloat x, const GLfloat y, const GLfloat w, const GLfloat h, Callback callback, void* userData) {
		this->name = name;
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->onevent = callback;
		this->userData = userData;
	}
};

class GLTouchScreen : public CObject {
public:
	typedef void* (*onTouchCallback)(const CString& event, void* userData);

private:
	GLRender* context = NULL;
	onTouchCallback onTouch = NULL;
	void* userData;

	GLfloat width;
	GLfloat height;
	
	GLfloat recordDistance;
	GLfloat minimalDistance;
	GLfloat swipeDistance;
	Vector<GLTouchObject*> clickList;
	Vector<GLTouchObject*> trackList;
	
	GLTouchPoint cur;
	GLTouchPoint* trackLast = NULL;
	GLTouchPoint trackSpeed;
	Vector<GLTouchObject*> trackingList;
	Vector<GLTouchPoint> trackingLast;
	
	Vector<Vec3> touchList;
	Vec3 _touchLast;
	Vec3* touchLast = NULL;
	bool touchCancel = false;
	uint64_t touchStart = 0;
	
	uint64_t touchTime = 0;
	bool touchDown = false;
	uint64_t mouseTime = 0;
	bool mouseDown = false;
	uint64_t mouseRepeatTime = 0;
	uint64_t touchTimeout = 0;
	
	bool checkBounds(float x, float y);

	bool canStartTouch();
	bool canStartMouse();
	
	void checkTimeout();

	void analyze(float x, float y);
	void recordTrack(float x, float y, bool stop);
	void record(float x, float y);
public:
	GLTouchScreen(GLRender* context, onTouchCallback callback, void* userData);
	~GLTouchScreen();
	
	void update(float width, float height);
	
	void trackEvent(const CString& name, float x, float y, float w, float h, GLTouchObject::Callback callback, void* userData);
	void clickEvent(const CString& name, float x, float y, float w, float h, GLTouchObject::Callback callback, void* userData);
	
	void onTouchStart(MotionEvent* event);
	void onTouchEnd(MotionEvent* event);
	void onTouchMove(MotionEvent* event);
	void onMouseDown(MotionEvent* event);
	void onMouseUp(MotionEvent* event);
	void onMouseOut(MotionEvent* event);
	void onMouseMove(MotionEvent* event);
};

#endif //JAPPSY_UGLTOUCHSCREEN_H