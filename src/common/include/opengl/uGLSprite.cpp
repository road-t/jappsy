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

#include "uGLSprite.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>

GLSprite::GLSprite(GLRender* context) {
	this->context = context;
}

GLSprite::~GLSprite() {
	this->context = NULL;
}

GLSprites::GLSprites(GLRender* context) throw(const char*) {
	this->context = context;
}

GLSprites::~GLSprites() {
	int32_t count = list.count();
	GLSprite** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLSprite* GLSprites::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLSprite* GLSprites::create(const CString& key) throw(const char*) {
	try {
		list.removedelete(key);
		GLSprite* sprite = new GLSprite(context);
		try {
			list.put(key, sprite);
		} catch (...) {
			delete sprite;
			throw;
		}
		return sprite;
	} catch (...) {
		throw;
	}
}
