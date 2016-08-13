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

#ifndef JAPPSY_UGLCAMERA_H
#define JAPPSY_UGLCAMERA_H

#include <data/uObject.h>
#include <opengl/uOpenGL.h>
#include <data/uVector.h>
#include <data/uNamedArray.h>

class GLRender;

class GLCamera : public Object {
public:
	constexpr static uint32_t PERSPECTIVE = 0;
	constexpr static uint32_t ORTHOGRAPHIC = 1;
	constexpr static uint32_t LAYER = 2;

	GLRender* context;
	const wchar_t* key;
	Vec3 position;
	Vec3 target;
	Vec3 head;
	GLfloat fov;
	GLfloat width;
	GLfloat height;
	GLfloat min;
	GLfloat max;
	uint32_t style;
	bool invalid;
	GLfloat scale;
	
	Mat4 projection16fv;
	Mat4 view16fv;
	
	GLCamera(GLRender* context, const wchar_t* key);
	~GLCamera();
	
	GLCamera& invalidate();
	GLCamera& size(GLfloat width, GLfloat height);
	
	GLCamera& perspective(GLfloat fov, GLfloat min, GLfloat max);
	GLCamera& ortho(GLfloat min, GLfloat max);
	GLCamera& layer(GLfloat offsetX, GLfloat offsetY);
	
	GLCamera& lookAt(const Vec3& position, const Vec3& target, const Vec3& head);
	GLCamera& rotate(const Vec3& vec, GLfloat angle);
	
	bool update();
};

class GLCameras {
private:
	GLRender* context;
	NamedArray<GLCamera>* list;
	
public:
	GLCamera* gui;
	
	GLCameras(GLRender* context) throw(const char*);
	~GLCameras();
	
	GLCamera* get(const wchar_t* key);
	GLCamera* createCamera(const wchar_t* key) throw(const char*);
};

#endif //JAPPSY_UGLCAMERA_H