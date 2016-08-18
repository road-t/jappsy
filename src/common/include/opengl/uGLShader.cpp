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

#include "uGLShader.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>

RefGLShader::RefGLShader(GLRender* context) {
	THIS.context = context;
}

RefGLShader::~RefGLShader() {
	THIS.context = NULL;
}

GLShaders::GLShaders(GLRender* context) throw(const char*) {
	THIS.context = context;
	list = new HashMap<String, GLTexture>();
}

GLShaders::~GLShaders() {
	list = null;
	context = NULL;
}

GLShader& GLShaders::get(const wchar_t* key) throw(const char*) {
	return (GLShader&)list.get(key);
}

GLShader& GLShaders::create(const wchar_t* key) throw(const char*) {
	try {
		list.remove(key);
		GLShader* shader = &(list.put(key, new RefGLShader(context)));
		if (wcscmp(key, L"null") == 0) {
		}
		return *shader;
	} catch (...) {
		throw;
	}
}
