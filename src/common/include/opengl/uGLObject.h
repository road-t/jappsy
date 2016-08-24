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

class GLScene;

class GLObject : public CObject {
public:
	GLScene* scene = NULL;
	CString key;
	
	GLSceneObject* object = NULL;
	bool visible = true;
	const GLfloat* time = NULL;
	
	Mat4 modelMatrix;
	Mat4 objectMatrix;
	
	GLObject(GLScene* context, const CString& key);
	~GLObject();
	
	GLObject* setModel(const CString& key) throw(const char*);
	GLObject* setFunc(const CString& key) throw(const char*);
	GLObject* setParticleSystem(const CString& key) throw(const char*);
	GLObject* setDrawing(const CString& key, const GLfloat* time = NULL) throw(const char*);
	
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
};

#endif //JAPPSY_UGLOBJECT_H