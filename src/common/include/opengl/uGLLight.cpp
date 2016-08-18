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

#include "uGLLight.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>

RefGLLight::RefGLLight(GLRender* context) {
	THIS.context = context;
}

RefGLLight::~RefGLLight() {
	THIS.context = NULL;
}

GLLights::GLLights(GLRender* context) throw(const char*) {
	THIS.context = context;
	list = new HashMap<String, GLTexture>();
}

GLLights::~GLLights() {
	list = null;
	context = NULL;
}

GLLight& GLLights::get(const wchar_t* key) throw(const char*) {
	return (GLLight&)list.get(key);
}

GLLight& GLLights::create(const wchar_t* key) throw(const char*) {
	try {
		list.remove(key);
		GLLight* shader = &(list.put(key, new RefGLLight(context)));
		if (wcscmp(key, L"null") == 0) {
		}
		return *shader;
	} catch (...) {
		throw;
	}
}
