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

#ifndef JAPPSY_UGLOBJECT_H
#define JAPPSY_UGLOBJECT_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uString.h>
#include <data/uVector.h>
#include <opengl/uGLSceneObject.h>
#include <opengl/uGLFunc.h>
#include <opengl/uGLTouchScreen.h>

class GLScene;
class GLDrawing;

enum GLObjectType {
	OBJECT_NONE,
	OBJECT_MODEL,
	OBJECT_FUNC,
	OBJECT_PARTICLE,
	OBJECT_DRAWING
};

class GLObject : public CObject {
public:
	GLScene* scene = NULL;
	CString key;
	
	GLObjectType objectType = OBJECT_NONE;
	GLSceneObject* object = NULL;
	bool visible = true;
	GLfloat time = NAN;
	
	typedef bool (*onEventCallback)(GLEngine* engine, const CString& event, GLDrawing* drawing);
	static bool eventHandler(const CString& event, const GLTouchPoint* cur, const GLTouchPoint* delta, const GLTouchPoint* speed, void* userData);
	onEventCallback onevent = NULL;
	
	Mat4 modelMatrix;
	Mat4 objectMatrix;
	
	GLObject(GLScene* context, const CString& key);
	~GLObject();
	
	GLObject* setModel(const CString& key) throw(const char*);
	GLObject* setFunc(GLFunc::onFuncCallback callback) throw(const char*);
	GLObject* setParticleSystem(const CString& key) throw(const char*);
	GLObject* setDrawing(const CString& key, const GLfloat time = NAN) throw(const char*);
	
	void trackEvent(onEventCallback callback);
	
	void render();
};

class GLObjects : public CObject {
private:
	GLScene* scene;
	VectorMap<CString&, GLObject*> list;
	
public:
	GLObjects(GLScene* context) throw(const char*);
	~GLObjects();
	
	GLObject* get(const CString& key) throw(const char*);
	GLObject* createObject(const CString& key) throw(const char*);
	void trackEvents(const Vector<GLObject*>& group, GLObject::onEventCallback onevent);
};

#endif //JAPPSY_UGLOBJECT_H