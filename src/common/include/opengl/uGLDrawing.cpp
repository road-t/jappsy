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

GLDrawing::GLDrawing(GLRender* context) {
	this->context = context;
}

GLDrawing::~GLDrawing() {
	this->context = NULL;
}

GLDrawings::GLDrawings(GLRender* context) throw(const char*) {
	this->context = context;
}

GLDrawings::~GLDrawings() {
	int32_t count = list.count();
	GLDrawing** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLDrawing* GLDrawings::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLDrawing* GLDrawings::create(const CString& key) throw(const char*) {
	try {
		list.remove(key);
		GLDrawing* drawing = new GLDrawing(context);
		try {
			list.put(key, drawing);
		} catch (...) {
			delete drawing;
			throw;
		}
		return drawing;
	} catch (...) {
		throw;
	}
}
