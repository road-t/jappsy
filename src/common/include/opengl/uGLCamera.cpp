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


GLCamera::GLCamera(GLRender* context, const wchar_t* key) {
	this->context = context;
	this->key = key;
	Vec3SetV(position.v, defaultPosition);
	Vec3SetV(target.v, defaultTarget);
	Vec3SetV(head.v, defaultHead);
	fov = 45;
	width = 1920;
	height = 1080;
	min = 0.1;
	max = 1000;
	style = PERSPECTIVE;
	invalid = true;
	scale = 0.01;
}

GLCamera::~GLCamera() {
	this->context = NULL;
	this->key = NULL;
}

GLCamera& GLCamera::invalidate() {
	invalid = true;
	return *this;
}

GLCamera& GLCamera::size(GLfloat width, GLfloat height) {
	this->width = width;
	this->height = height;
	invalid = true;
	return *this;
}

GLCamera& GLCamera::perspective(GLfloat fov, GLfloat min, GLfloat max) {
	this->fov = fov;
	this->min = min;
	this->max = max;
	style = PERSPECTIVE;
	invalid = true;
	return *this;
}

GLCamera& GLCamera::ortho(GLfloat min, GLfloat max) {
	this->min = min;
	this->max = max;
	style = ORTHOGRAPHIC;
	invalid = true;
	return *this;
}

GLCamera& GLCamera::layer(GLfloat offsetX, GLfloat offsetY) {
	target.x = offsetX;
	target.y = offsetY;
	style = LAYER;
	invalid = true;
	return *this;
}

GLCamera& GLCamera::lookAt(const Vec3& position, const Vec3& target, const Vec3& head) {
	this->position.set(position);
	this->target.set(target);
	this->head.set(head);
	invalid = true;
	return *this;
}

GLCamera& GLCamera::rotate(const Vec3& vec, GLfloat angle) {
	Vec3 normal; normal.normalize(vec);
	Mat4 rotate; rotate.rotate(normal, angle);
	position.subtract(target).transform(rotate).add(target);
	invalid = true;
	return *this;
}

bool GLCamera::update() {
	if (invalid) {
		if (style == LAYER) {
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
				GLfloat w2 = width / 2.0;
				GLfloat h2 = height / 2.0;
				projection16fv.ortho(-w2, w2, -h2, h2, min, max);
			} else if (style == PERSPECTIVE) {
				Vec3 v; v.set(scale);
				Mat4 scale; scale.scale(v);
				projection16fv.perspective(fov, width/height, min, max).multiply(scale);
			}
			
			view16fv.lookAt(position, target, head);
		}
		
		invalid = false;
		return true;
	}
	
	return false;
}