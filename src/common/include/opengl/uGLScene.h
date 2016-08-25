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

#ifndef JAPPSY_UGLSCENE_H
#define JAPPSY_UGLSCENE_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uString.h>
#include <opengl/uGLCamera.h>
#include <data/uVector.h>
#include <opengl/uGLObject.h>
#include <opengl/uGLDrawing.h>

class GLRender;
class GLLights;
class GLObjects;
class GLObject;

class GLScene : public CObject {
public:
	GLRender* context = NULL;
	GLCamera* camera = NULL;
	Vec3 ambient = {0.25, 0.25, 0.25};
	GLLights* lights = NULL;
	GLObjects* objects = NULL;
	Vector<Vector<GLObject*>*> layers;
	
	GLuint lights1i = 0;
	GLfloat* lights16fvv = NULL;
	Mat4 modelViewProjection16fv;
	Mat4 modelView16fv;
	Mat4 normal16fv;
	
	GLScene(GLRender* context);
	~GLScene();
	
	inline GLScene* setCamera(GLCamera* camera) { this->camera = camera; return this; }
	
	GLScene* rotateGroup(Vector<GLObject*>& group, const Vec3& vec, GLfloat angle, bool permanent = false);
	GLScene* visibleGroup(Vector<GLObject*>& group, bool visible);
	
	Vector<GLObject*>* createLayer() throw(const char*);
	GLObject* createLayerObject(Vector<GLObject*>* layer, const CString& key) throw(const char*);
	GLObject* createLayerDrawing(Vector<GLObject*>* layer, const CString& key, const GLfloat time = NAN) throw(const char*);
	
	void update();
	void render();
};

class GLScenes : public CObject {
private:
	GLRender* context;
	VectorMap<CString&, GLScene*> list;
	
public:
	GLScenes(GLRender* context) throw(const char*);
	~GLScenes();
	
	GLScene* get(const CString& key) throw(const char*);
	GLScene* createScene(const CString& key) throw(const char*);
};

#endif //JAPPSY_UGLSCENE_H