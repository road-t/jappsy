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
#include <data/uHashMap.h>

class GLRender;

class RefGLFunc : public RefObject {
public:
	GLRender* context = NULL;
	
	inline RefGLFunc() { throw eInvalidParams; }
	RefGLFunc(GLRender* context);
	~RefGLFunc();
};

class GLFunc : public Object {
public:
	RefClass(GLFunc, RefGLFunc)
};

class GLFuncs {
private:
	GLRender* context;
	HashMap<String, GLFunc> list;
	
public:
	GLFuncs(GLRender* context) throw(const char*);
	~GLFuncs();
	
	GLFunc& get(const wchar_t* key) throw(const char*);
	GLFunc& create(const wchar_t* key) throw(const char*);
};

#endif //JAPPSY_UGLFUNC_H