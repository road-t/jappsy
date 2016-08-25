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

#ifndef JAPPSY_UGLANIMATION_H
#define JAPPSY_UGLANIMATION_H

#include <platform.h>
#include <opengl/uOpenGL.h>
#include <opengl/uGLSceneObject.h>
#include <opengl/uGLPaint.h>

class GLAnimation;

class GLAnimationTarget : public GLSceneObject {
public:
	GLAnimation* animation = NULL;
	GLPaint* paint = NULL;
};

class GLAnimation : public CObject {
private:
	static void updatePingPong(GLAnimation* animation);
	static void updateLightOut(GLAnimation* animation);
	static void updateLightIn(GLAnimation* animation);
	static void updateFadeOut(GLAnimation* animation);
	static void updateFadeIn(GLAnimation* animation);

public:
	typedef void (*onUpdateCallback)(GLAnimation* animation);
	
	GLAnimationTarget* target = NULL;
	uint64_t time;
	GLfloat min;
	GLfloat max;
	GLfloat speed;
	onUpdateCallback onupdate = NULL;
	
	GLAnimation(GLAnimationTarget* target, GLfloat ofsValue, GLfloat minValue, GLfloat maxValue, uint64_t timeLength, onUpdateCallback onupdate);
	~GLAnimation();
	
	void update();
	
	static void createPingPong(GLAnimationTarget* target, GLfloat minValue, GLfloat maxValue, uint64_t timeLength) throw(const char*);
	static void createBlink(GLAnimationTarget* target, GLfloat minValue, GLfloat maxValue, uint64_t timeLength) throw(const char*);
	static void createLightOut(GLAnimationTarget* target, GLfloat minValue, GLfloat maxValue, uint64_t timeLength) throw(const char*);
	static void createLightIn(GLAnimationTarget* target, GLfloat minValue, GLfloat maxValue, uint64_t timeLength) throw(const char*);
	static void createFadeOut(GLAnimationTarget* target, GLfloat minValue, GLfloat maxValue, uint64_t timeLength) throw(const char*);
	static void createFadeIn(GLAnimationTarget* target, GLfloat minValue, GLfloat maxValue, uint64_t timeLength) throw(const char*);
};

#endif //JAPPSY_UGLANIMATION_H