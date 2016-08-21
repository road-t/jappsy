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
#include <data/uHashMap.h>

class GLRender;

class RefGLObject : public JRefObject {
public:
	GLRender* context = NULL;
	
	inline RefGLObject() { throw eInvalidParams; }
	RefGLObject(GLRender* context);
	~RefGLObject();
};

class GLObject : public JObject {
public:
	JRefClass(GLObject, RefGLObject)
};

class GLObjects {
private:
	GLRender* context;
	HashMap<JString, GLObject> list;
	
public:
	GLObjects(GLRender* context) throw(const char*);
	~GLObjects();
	
	GLObject& get(const JString& key) throw(const char*);
	GLObject& create(const JString& key) throw(const char*);
};

#endif //JAPPSY_UGLOBJECT_H