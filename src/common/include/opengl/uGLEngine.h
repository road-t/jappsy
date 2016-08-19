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

#ifndef JAPPSY_UGLENGINE_H
#define JAPPSY_UGLENGINE_H

#include <data/uObject.h>
#include <event/uMotionEvent.h>

class GLRender;

class RefGLEngine : public RefObject {
protected:
	GLRender* context;
	
public:
	inline RefGLEngine() {}
	virtual inline ~RefGLEngine() { release(); }
	virtual void release() { context = NULL; }
	
	void onRender();
	void onUpdate(int width, int height);
	void onTouch(MotionEvent* event);

};

class GLEngine : public Object {
public:
	RefClass(GLEngine, RefGLEngine);
	
	inline void release() throw(const char*) { THIS.ref().release(); }
	
	inline void onRender() throw(const char*) { THIS.ref().onRender(); }
	inline void onUpdate(int width, int height) throw(const char*) { THIS.ref().onUpdate(width, height); }
	inline void onTouch(MotionEvent* event) throw(const char*) { THIS.ref().onTouch(event); }
};

#endif //JAPPSY_UGLENGINE_H