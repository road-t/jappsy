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

#include "uGLScene.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>
#include <opengl/uGLLight.h>
#include <opengl/uGLObject.h>

RefGLScene::RefGLScene(GLRender* context) {
	THIS.context = context;

	//lights = new GLLights(this);
}

RefGLScene::~RefGLScene() {
	THIS.context = NULL;
}

GLScenes::GLScenes(GLRender* context) throw(const char*) {
	THIS.context = context;
	list = new HashMap<JString, GLScene>();
}

GLScenes::~GLScenes() {
	list = null;
	context = NULL;
}

GLScene& GLScenes::get(const JString& key) throw(const char*) {
	return (GLScene&)list.get(key);
}

GLScene& GLScenes::create(const JString& key) throw(const char*) {
	try {
		list.remove(key);
		GLScene* shader = &(list.put(key, new RefGLScene(context)));
		return *shader;
	} catch (...) {
		throw;
	}
}
