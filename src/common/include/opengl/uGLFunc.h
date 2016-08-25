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

#ifndef JAPPSY_UGLFUNC_H
#define JAPPSY_UGLFUNC_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uVector.h>
#include <opengl/uGLSceneObject.h>

class GLRender;
class GLEngine;

class GLFunc : public GLSceneObject {
public:
	typedef void (*onFuncCallback)(GLEngine* engine);

private:
	GLRender* context = NULL;
	onFuncCallback onfunc = NULL;
	
public:
	GLFunc(GLRender* context, onFuncCallback callback);
	~GLFunc();
	
	void render(GLObject* object, const GLfloat time = NAN);
};

class GLFuncs : public CObject {
private:
	GLRender* context;
	VectorMap<CString&, GLFunc*> list;
	
public:
	GLFuncs(GLRender* context) throw(const char*);
	~GLFuncs();
	
	GLFunc* get(const CString& key) throw(const char*);
	GLFunc* createFunc(const CString& key, GLFunc::onFuncCallback callback) throw(const char*);
};

#endif //JAPPSY_UGLFUNC_H