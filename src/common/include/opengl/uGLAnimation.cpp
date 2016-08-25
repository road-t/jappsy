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

#include "uGLAnimation.h"
#include <core/uSystem.h>
#include <math.h>

GLAnimation::GLAnimation(GLAnimationTarget* target, GLfloat ofsValue, GLfloat minValue, GLfloat maxValue, uint64_t timeLength, onUpdateCallback onupdate) {
	this->target = target;
	this->time = currentTimeMillis() + (int)floorf(ofsValue * (GLfloat)timeLength);
	this->min = minValue;
	this->max = maxValue;
	this->speed = timeLength;
	this->onupdate = onupdate;
}

GLAnimation::~GLAnimation() {
	if (target != NULL) {
		target->animation = NULL;
	}
}

void GLAnimation::update() {
	if (onupdate != NULL)
		onupdate(this);
}

void GLAnimation::updatePingPong(GLAnimation* animation) {
	uint64_t time = currentTimeMillis();
	GLPaint* paint = animation->target->paint;
	if (time < animation->time) {
		GLfloat ofs = (GLfloat)(animation->time - time) / animation->speed;
		paint->m_color4iv[0] = paint->m_color4iv[1] = paint->m_color4iv[2] = paint->m_color4iv[3]
		= animation->min - (animation->max - animation->min) * ofs;
	} else {
		GLfloat ofs = (animation->speed - fabsf(modulef(time - animation->time, animation->speed * 2.0) - animation->speed)) / animation->speed;
		paint->m_color4iv[3] = 1.0;
		paint->m_color4iv[0] = paint->m_color4iv[1] = paint->m_color4iv[2] =
			animation->min + (animation->max - animation->min) * ofs;
	}
}

void GLAnimation::updateLightOut(GLAnimation* animation) {
	uint64_t time = currentTimeMillis();
	GLPaint* paint = animation->target->paint;
	GLfloat elapsed = time - animation->time;
	if (elapsed < animation->speed) {
		GLfloat ofs = elapsed / animation->speed;
		paint->m_color4iv[3] = 1.0;
		paint->m_color4iv[0] = paint->m_color4iv[1] = paint->m_color4iv[2] =
			animation->max - (animation->max - animation->min) * ofs;
	} else {
		paint->m_color4iv[3] = 1.0;
		paint->m_color4iv[0] = paint->m_color4iv[1] = paint->m_color4iv[2] =
			animation->min;
		delete animation;
	}
}

void GLAnimation::updateLightIn(GLAnimation* animation) {
	uint64_t time = currentTimeMillis();
	GLPaint* paint = animation->target->paint;
	GLfloat elapsed = time - animation->time;
	if (elapsed < animation->speed) {
		GLfloat ofs = elapsed / animation->speed;
		paint->m_color4iv[3] = 1.0;
		paint->m_color4iv[0] = paint->m_color4iv[1] = paint->m_color4iv[2] =
			animation->min + (animation->max - animation->min) * ofs;
	} else {
		paint->m_color4iv[3] = 1.0;
		paint->m_color4iv[0] = paint->m_color4iv[1] = paint->m_color4iv[2] =
			animation->max;
		delete animation;
	}
}

void GLAnimation::updateFadeOut(GLAnimation* animation) {
	uint64_t time = currentTimeMillis();
	GLPaint* paint = animation->target->paint;
	GLfloat elapsed = time - animation->time;
	if (elapsed < animation->speed) {
		GLfloat ofs = elapsed / animation->speed;
		paint->m_color4iv[0] = paint->m_color4iv[1] = paint->m_color4iv[2] = paint->m_color4iv[3] =
			animation->max - (animation->max - animation->min) * ofs;
	} else {
		paint->m_color4iv[0] = paint->m_color4iv[1] = paint->m_color4iv[2] = paint->m_color4iv[3] =
			animation->min;
		delete animation;
	}
}

void GLAnimation::updateFadeIn(GLAnimation* animation) {
	uint64_t time = currentTimeMillis();
	GLPaint* paint = animation->target->paint;
	GLfloat elapsed = time - animation->time;
	if (elapsed < animation->speed) {
		GLfloat ofs = elapsed / animation->speed;
		paint->m_color4iv[0] = paint->m_color4iv[1] = paint->m_color4iv[2] = paint->m_color4iv[3] =
			animation->min + (animation->max - animation->min) * ofs;
	} else {
		paint->m_color4iv[0] = paint->m_color4iv[1] = paint->m_color4iv[2] = paint->m_color4iv[3] =
			animation->max;
		delete animation;
	}
}

void GLAnimation::createPingPong(GLAnimationTarget* target, GLfloat minValue, GLfloat maxValue, uint64_t timeLength) throw(const char*) {
	GLfloat ofs = (target->paint->m_color4iv[0] - minValue) / (maxValue - minValue);
	if (target->animation != NULL)
		delete target->animation;
	target->animation = new GLAnimation(target, -ofs, minValue, maxValue, timeLength, updatePingPong);
}

void GLAnimation::createBlink(GLAnimationTarget* target, GLfloat minValue, GLfloat maxValue, uint64_t timeLength) throw(const char*) {
	if (target->animation != NULL)
		delete target->animation;
	target->animation = new GLAnimation(target, 0, minValue, maxValue, timeLength, updateLightOut);
}

void GLAnimation::createLightOut(GLAnimationTarget* target, GLfloat minValue, GLfloat maxValue, uint64_t timeLength) throw(const char*) {
	GLfloat ofs = maxValue - (target->paint->m_color4iv[0] - minValue) / (maxValue - minValue);
	if (target->animation != NULL)
		delete target->animation;
	target->animation = new GLAnimation(target, -ofs, minValue, maxValue, timeLength, updateLightOut);
}

void GLAnimation::createLightIn(GLAnimationTarget* target, GLfloat minValue, GLfloat maxValue, uint64_t timeLength) throw(const char*) {
	GLfloat ofs = (target->paint->m_color4iv[0] - minValue) / (maxValue - minValue);
	if (target->animation != NULL)
		delete target->animation;
	target->animation = new GLAnimation(target, -ofs, minValue, maxValue, timeLength, updateLightIn);
}

void GLAnimation::createFadeOut(GLAnimationTarget* target, GLfloat minValue, GLfloat maxValue, uint64_t timeLength) throw(const char*) {
	GLfloat ofs = maxValue - (target->paint->m_color4iv[0] - minValue) / (maxValue - minValue);
	if (target->animation != NULL)
		delete target->animation;
	target->animation = new GLAnimation(target, -ofs, minValue, maxValue, timeLength, updateFadeOut);
}

void GLAnimation::createFadeIn(GLAnimationTarget* target, GLfloat minValue, GLfloat maxValue, uint64_t timeLength) throw(const char*) {
	GLfloat ofs = (target->paint->m_color4iv[0] - minValue) / (maxValue - minValue);
	if (target->animation != NULL)
		delete target->animation;
	target->animation = new GLAnimation(target, -ofs, minValue, maxValue, timeLength, updateFadeIn);
}
