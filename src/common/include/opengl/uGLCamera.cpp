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

#include "uGLCamera.h"
#include <opengl/uGLRender.h>

static GLfloat defaultPosition[3] = { 0, 0, -1 };
static GLfloat defaultTarget[3] = { 0, 0, 0 };
static GLfloat defaultHead[3] = { 0, 1, 0 };

GLCamera::GLCamera(GLRender* context, const CString& key) {
	this->context = context;
	this->key = key;
	Vec3SetV(position.v, defaultPosition);
	Vec3SetV(target.v, defaultTarget);
	Vec3SetV(head.v, defaultHead);
	fov = 45;
	width = 1920;
	height = 1080;
	ratio = width / height;
	min = 0.1;
	max = 1000;
	style = PERSPECTIVE;
	invalid = true;

	// В каждой системе разный буфер глубины, на его точность влияет размер значений
	// Поэтому производится уменьшение масштаба всех обьектов, чтобы глубина не превышала порог
#if defined(__IOS__)
	scale = 0.001;
#elif defined(__JNI__)
	scale = 0.0001;
#else
	#error Unsupported platform!
#endif
}

GLCamera* GLCamera::invalidate() {
	invalid = true;
	return this;
}

GLCamera* GLCamera::size(GLfloat width, GLfloat height) {
	this->width = width;
	this->height = height;
	this->ratio = width / height;
	invalid = true;
	return this;
}

GLCamera* GLCamera::perspective(GLfloat fov, GLfloat min, GLfloat max) {
	this->fov = fov;
	this->min = min;
	this->max = max;
	style = PERSPECTIVE;
	invalid = true;
	return this;
}

GLCamera* GLCamera::ortho(GLfloat min, GLfloat max) {
	this->min = min;
	this->max = max;
	style = ORTHOGRAPHIC;
	invalid = true;
	return this;
}

GLCamera* GLCamera::layer(GLfloat offsetX, GLfloat offsetY) {
	target.x = offsetX;
	target.y = offsetY;
	style = LAYER;
	invalid = true;
	return this;
}

GLCamera* GLCamera::background() {
	target.x = target.y = 0;
	style = BACKGROUND;
	invalid = true;
	return this;
}

GLCamera* GLCamera::lookAt(const Vec3& position, const Vec3& target, const Vec3& head) {
	this->position.set(position);
	this->target.set(target);
	this->head.set(head);
	invalid = true;
	return this;
}

GLCamera* GLCamera::rotate(const Vec3& vec, GLfloat angle) {
	Vec3 normal; normal.normalize(vec);
	Mat4 rotate; rotate.rotate(normal, angle);
	position.subtract(target).transform(rotate).add(target);
	invalid = true;
	return this;
}

bool GLCamera::update() {
	if (invalid) {
		if ((style == LAYER) || (style == BACKGROUND)) {
			projection16fv.set(0);
			projection16fv[0] = 2.0f / width;
			projection16fv[5] = -2.0f / height;
			projection16fv[12] = -1.0f + target.x * projection16fv[0];
			projection16fv[13] = 1.0f + target.y * projection16fv[5];
			projection16fv[10] = projection16fv[15] = 1.0f;
			
			/*
			 {  2/w,     0,     0,    0}		rX = sX * 2 / w - 1 <=> (0..w) -> (-1..+1)
			 {    0,  -2/h,     0,    0}		rY = sY * -2 / h + 1 <=> (0..w) -> (+1..-1)	// flip vertical
			 {    0,     0,     1,    0}
			 {   -1,     1,     0,    1}
			 */
			
			view16fv.identity();
		} else {
			if (style == ORTHOGRAPHIC) {
				GLfloat w2 = width / 2.0f;
				GLfloat h2 = height / 2.0f;
				projection16fv.ortho(-w2, w2, -h2, h2, min, max);
			} else if (style == PERSPECTIVE) {
				Vec3 v; v.set(scale);
				Mat4 scale; scale.scale(v);
				projection16fv.perspective(fov, width/height, min, max).multiply(scale);
			}
			
			view16fv.lookAt(position, target, head);
		}
		
		if (style != BACKGROUND) {
			projection16fv.multiply(context->ratio16fv, projection16fv);
		}
		
		invalid = false;
		return true;
	}
	
	return false;
}

GLCameras::GLCameras(GLRender* context) throw(const char*) {
	this->context = context;
}

GLCameras::~GLCameras() {
	int32_t count = list.count();
	GLCamera** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLCamera* GLCameras::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLCamera* GLCameras::createCamera(const CString& key) throw(const char*) {
	try {
		return list.get(key);
	} catch (...) {
		GLCamera* cam = new GLCamera(context, key);
		try {
			list.put(key, cam);
		} catch (...) {
			delete cam;
			throw;
		}
		if (key == L"gui") {
			gui = cam;
		} else if (key == L"background") {
			background = cam;
		}
		return cam;
	}
}

void GLCameras::forceUpdate() {
	int32_t count = list.count();
	GLCamera** items = list.items();
	for (int i = 0; i < count; i++) {
		items[i]->invalid = true;
	}
}
