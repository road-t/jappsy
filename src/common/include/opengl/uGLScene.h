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
#include <data/uHashMap.h>

class GLRender;

class RefGLScene : public RefObject {
public:
	GLRender* context = NULL;
	
	inline RefGLScene() { throw eInvalidParams; }
	RefGLScene(GLRender* context);
	~RefGLScene();
};

class GLScene : public Object {
public:
	RefClass(GLScene, RefGLScene)
};

class GLScenes {
private:
	GLRender* context;
	HashMap<String, GLScene> list;
	
public:
	GLScenes(GLRender* context) throw(const char*);
	~GLScenes();
	
	GLScene& get(const wchar_t* key) throw(const char*);
	GLScene& create(const wchar_t* key) throw(const char*);
};

#endif //JAPPSY_UGLSCENE_H