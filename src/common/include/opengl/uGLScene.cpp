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

GLScene::GLScene(GLRender* context) {
	this->context = context;

	//lights = new GLLights(this);
}

GLScene::~GLScene() {
	this->context = NULL;
}

GLScenes::GLScenes(GLRender* context) throw(const char*) {
	this->context = context;
}

GLScenes::~GLScenes() {
	int32_t count = list.count();
	GLScene** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLScene* GLScenes::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLScene* GLScenes::createScene(const CString& key) throw(const char*) {
	try {
		list.removedelete(key);
		GLScene* scene = new GLScene(context);
		try {
			list.put(key, scene);
		} catch (...) {
			delete scene;
			throw;
		}
		return scene;
	} catch (...) {
		throw;
	}
}
