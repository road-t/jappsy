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

#ifndef JAPPSY_UGLLIGHT_H
#define JAPPSY_UGLLIGHT_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uString.h>
#include <opengl/uGLScene.h>
#include <data/uVector.h>

enum GLLightStyle { OMNI, SPOT, DIRECT };

class GLLight : public CObject {
private:
	GLScene* scene = NULL;
	CString key;
	Vec3 position = {0.0, 0.0, -1.0};
	Vec3 target = {0.0, 0.0, 0.0};
	Vec3 color = {1.0, 1.0, 1.0};
	GLfloat intensivity = 1.0;
	GLfloat hotspot = 0;
	GLfloat falloff = 0;
	bool fixed = false;
	GLLightStyle style = GLLightStyle::DIRECT;
	bool active = true;
	bool invalid = true;
	
public:
	Vec3 position3fv;
	Vec3 target3fv;
	Mat4 light16fv;
	
	GLLight(GLScene* scene, const CString& key);
	
	inline GLLight& invalidate() { invalid = true; return *this; }

	GLLight* omni(const Vec3& position, const Vec3& color, const GLfloat intensivity = 1.0, const GLfloat radius = 0.0, const GLfloat falloff = 0.0, const bool fixed = false);
	GLLight* spot(const Vec3& position, const Vec3& target, const Vec3& color, const GLfloat intensivity = 1.0, const GLfloat angle = 0.0, const GLfloat falloff = 0.0, const bool fixed = false);
	GLLight* direct(const Vec3& position, const Vec3& target, const Vec3& color, const GLfloat intensivity = 1.0, const GLfloat radius = 0.0, const GLfloat falloff = 0.0, const bool fixed = false);
	
	bool update();
};

class GLLights : public CObject {
private:
	GLScene* scene = NULL;
	VectorMap<CString&, GLLight*> list;

public:
	GLLights(GLScene* scene) throw(const char*);
	~GLLights();
	
	GLLight* get(const CString& key) throw(const char*);
	GLLight* createLight(const CString& key) throw(const char*);
};

#endif //JAPPSY_UGLLIGHT_H