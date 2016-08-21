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

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uHashMap.h>
#include <data/uVector.h>

class GLRender;

class RefGLCamera : public JRefObject {
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
	
	inline RefGLCamera() { throw eInvalidParams; }
	RefGLCamera(GLRender* context, const wchar_t* key);
	~RefGLCamera();
	
	RefGLCamera& invalidate();
	RefGLCamera& size(GLfloat width, GLfloat height);
	
	RefGLCamera& perspective(GLfloat fov, GLfloat min, GLfloat max);
	RefGLCamera& ortho(GLfloat min, GLfloat max);
	RefGLCamera& layer(GLfloat offsetX, GLfloat offsetY);
	
	RefGLCamera& lookAt(const Vec3& position, const Vec3& target, const Vec3& head);
	RefGLCamera& rotate(const Vec3& vec, GLfloat angle);
	
	bool update();
};

class GLCamera : public JObject {
public:
	JRefClass(GLCamera, RefGLCamera);
	
	inline RefGLCamera& invalidate() throw(const char*) { return THIS.ref().invalidate(); }
	inline RefGLCamera& size(GLfloat width, GLfloat height) throw(const char*) { return THIS.ref().size(width, height); }
	
	inline RefGLCamera& perspective(GLfloat fov, GLfloat min, GLfloat max) throw(const char*) { return THIS.ref().perspective(fov, min, max); }
	inline RefGLCamera& ortho(GLfloat min, GLfloat max) throw(const char*) { return THIS.ref().ortho(min, max); }
	inline RefGLCamera& layer(GLfloat offsetX, GLfloat offsetY) throw(const char*) { return THIS.ref().layer(offsetX, offsetY); }
	
	inline RefGLCamera& lookAt(const Vec3& position, const Vec3& target, const Vec3& head) throw(const char*) { return THIS.ref().lookAt(position, target, head); }
	inline RefGLCamera& rotate(const Vec3& vec, GLfloat angle) throw(const char*) { return THIS.ref().rotate(vec, angle); }
	
	inline bool update() throw(const char*) { return THIS.ref().update(); }
};

class GLCameras {
private:
	GLRender* context;
	HashMap<JString, GLCamera> list;
	
public:
	GLCamera gui;
	
	GLCameras(GLRender* context) throw(const char*);
	~GLCameras();
	
	GLCamera& get(const wchar_t* key) throw(const char*);
	GLCamera& createCamera(const wchar_t* key) throw(const char*);
};

#endif //JAPPSY_UGLCAMERA_H