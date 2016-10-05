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

#include "uGLTexture.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>

GLTexture::GLTexture(GLRender* context, Vector<GLuint>& handles, GLint width, GLint height) throw(const char*) {
	this->context = context;
	uint32_t count = handles.count();
	if (count > 0) {
		this->handles.resize(count);
		memcpy(this->handles.items(), handles.items(), count * sizeof(GLuint));

		try {
			handles1iv.resize(count);
		} catch (...) {
			this->handles.clear();
			throw;
		}
	}
	this->width = width;
	this->height = height;
}

GLTexture::~GLTexture() {
	uint32_t count = handles.count();
	if (handles.count() > 0) {
		for (int i = 0; i < count; i++) {
			context->textures->releaseTextureHandle(handles[i]);
		}
	}
}

GLuint GLTexture::bind(GLint index, GLint uniform) {
	for (int i = 0; i < handles.count(); i++) {
		GLuint handle = handles.get(i);
		context->activeTexture(index);
		glBindTexture(GL_TEXTURE_2D, handle);
		handles1iv[i] = index;
		index++;
	}
	
	if (uniform != -1) {
		uint32_t count = handles.count();
		if (count == 1) {
			glUniform1i(uniform, handles1iv[0]);
		} else {
			glUniform1iv(uniform, count, handles1iv.items());
		}
	}
	
	return index;
}

static Vec4 defaultTextureColor = { 0, 0, 0, 0 };

GLTextures::GLTextures(GLRender* context) throw(const char*) {
	this->context = context;
	defaultTexture = createSolidTexture(L"null", defaultTextureColor);
}

GLTextures::~GLTextures() {
	int32_t count = list.count();
	GLTexture** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLTexture* GLTextures::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLTexture* GLTextures::createSolidTexture(const CString& key, const Vec4& rgba4fv) throw(const char*) {
	GLuint handle;
	glGenTextures(1, &handle);
	CheckGLError();
	
	uint8_t data[4] = { (uint8_t)(rgba4fv.r * 255), (uint8_t)(rgba4fv.g * 255), (uint8_t)(rgba4fv.b * 255), (uint8_t)(rgba4fv.a * 255) };
	
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	try {
		CheckGLError();
	} catch (...) {
		glDeleteTextures(1, &handle);
		throw;
	}
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	try {
		list.remove(key);

		Vector<GLuint> handles;
		handles.push(handle);
		GLTexture* texture = new GLTexture(context, handles, 1, 1);
		try {
			list.put(key, texture);
		} catch (...) {
			delete texture;
			throw;
		}
		if (key == L"null") {
			defaultTexture = texture;
			defaultTextureHandle = handles[0];
		}
		return texture;
	} catch (...) {
		glDeleteTextures(1, &handle);
		throw;
	}
}

GLTexture* GLTextures::createTexture(const CString& key, Vector<GLuint>& handles, GLint width, GLint height) throw(const char*) {
	list.remove(key);
	GLTexture* texture = new GLTexture(context, handles, width, height);
	try {
		list.put(key, texture);
	} catch (...) {
		delete texture;
		throw;
	}
	if (key == L"null") {
		defaultTexture = texture;
		defaultTextureHandle = handles[0];
	}
	return texture;
}

GLuint GLTextures::createTextureHandle(GLint width, GLint height, int style, void* data) throw(const char*) {
	GLuint handle;
	glGenTextures(1, &handle);
	CheckGLError();

	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	try {
		CheckGLError();
	} catch (...) {
		glDeleteTextures(1, &handle);
		throw;
	}
	
	if (style & GLTexture::SMOOTH) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	
	if (style & GLTexture::REPEAT) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	
	return handle;
}

void GLTextures::releaseTextureHandle(GLuint handle) {
	glDeleteTextures(1, &handle);
}
