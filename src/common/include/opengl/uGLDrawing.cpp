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

#include "uGLDrawing.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>

RefGLDrawing::RefGLDrawing(GLRender* context) {
	THIS.context = context;
}

RefGLDrawing::~RefGLDrawing() {
	THIS.context = NULL;
}

GLDrawings::GLDrawings(GLRender* context) throw(const char*) {
	THIS.context = context;
	list = new HashMap<String, GLDrawing>();
}

GLDrawings::~GLDrawings() {
	list = null;
	context = NULL;
}

GLDrawing& GLDrawings::get(const wchar_t* key) throw(const char*) {
	return (GLDrawing&)list.get(key);
}

GLDrawing& GLDrawings::create(const wchar_t* key) throw(const char*) {
	try {
		list.remove(key);
		GLDrawing* shader = &(list.put(key, new RefGLDrawing(context)));
		if (wcscmp(key, L"null") == 0) {
		}
		return *shader;
	} catch (...) {
		throw;
	}
}
