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

#include "uGLFunc.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>

GLFunc::GLFunc(GLRender* context, onFuncCallback callback) {
	this->context = context;
	this->onfunc = callback;
}

GLFunc::~GLFunc() {
	this->context = NULL;
}

void GLFunc::render(GLObject* object, const GLfloat time) {
	onfunc(context->engine);
}

GLFuncs::GLFuncs(GLRender* context) throw(const char*) {
	this->context = context;
}

GLFuncs::~GLFuncs() {
	int32_t count = list.count();
	GLFunc** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLFunc* GLFuncs::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLFunc* GLFuncs::createFunc(const CString& key, GLFunc::onFuncCallback callback) throw(const char*) {
	try {
		list.removedelete(key);
		GLFunc* func = new GLFunc(context, callback);
		try {
			list.put(key, func);
		} catch (...) {
			delete func;
			throw;
		}
		return func;
	} catch (...) {
		throw;
	}
}
