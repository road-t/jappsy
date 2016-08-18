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

#ifndef JAPPSY_UGLMODEL_H
#define JAPPSY_UGLMODEL_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uHashMap.h>

class GLRender;

class RefGLModel : public RefObject {
public:
	GLRender* context = NULL;
	
	inline RefGLModel() { throw eInvalidParams; }
	RefGLModel(GLRender* context);
	~RefGLModel();
};

class GLModel : public Object {
public:
	RefClass(GLModel, RefGLModel)
};

class GLModels {
private:
	GLRender* context;
	HashMap<String, GLModel> list;
	
public:
	GLModels(GLRender* context) throw(const char*);
	~GLModels();
	
	GLModel& get(const wchar_t* key) throw(const char*);
	GLModel& create(const wchar_t* key) throw(const char*);
};

#endif //JAPPSY_UGLMODEL_H