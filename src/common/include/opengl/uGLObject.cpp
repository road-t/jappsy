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

#include "uGLObject.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>

RefGLObject::RefGLObject(GLRender* context) {
	THIS.context = context;
}

RefGLObject::~RefGLObject() {
	THIS.context = NULL;
}

GLObjects::GLObjects(GLRender* context) throw(const char*) {
	THIS.context = context;
	list = new HashMap<String, GLObject>();
}

GLObjects::~GLObjects() {
	list = null;
	context = NULL;
}

GLObject& GLObjects::get(const wchar_t* key) throw(const char*) {
	return (GLObject&)list.get(key);
}

GLObject& GLObjects::create(const wchar_t* key) throw(const char*) {
	try {
		list.remove(key);
		GLObject* shader = &(list.put(key, new RefGLObject(context)));
		if (wcscmp(key, L"null") == 0) {
		}
		return *shader;
	} catch (...) {
		throw;
	}
}
