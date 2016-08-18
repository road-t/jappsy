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

RefGLTexture::RefGLTexture(GLRender* context, GLuint* handles, GLuint count, GLint width, GLint height) {
	THIS.context = context;
	if ((handles != NULL) && (count > 0)) {
		THIS.handles = memAlloc(GLuint, THIS.handles, count * sizeof(GLuint));
		if (THIS.handles == NULL)
			throw eOutOfMemory;
		memcpy(THIS.handles, handles, count * sizeof(GLuint));
		
		handles1iv = memAlloc(GLint, handles1iv, count * sizeof(GLint));
		if (handles1iv == NULL) {
			memFree(THIS.handles);
			THIS.handles = NULL;
			throw eOutOfMemory;
		}
		
		THIS.count = count;
	}
	THIS.width = width;
	THIS.height = height;
}

RefGLTexture::~RefGLTexture() {
	if (handles != NULL) {
		for (int i = 0; i < count; i++) {
			context->textures->releaseTextureHandle(handles[i]);
		}
		width = height = 0;
		memFree(handles);
		handles = NULL;
		count = 0;
	}
	if (handles1iv != NULL) {
		memFree(handles1iv);
		handles1iv = NULL;
	}
	context = NULL;
}

GLuint RefGLTexture::bind(GLint index, GLint uniform) {
	for (int i = 0; i < count; i++) {
		context->activeTexture(handles[i]);
		glBindTexture(GL_TEXTURE_2D, handles[i]);
		handles1iv[i] = index;
		index++;
	}
	
	if (uniform != -1) {
		if (count == 1) {
			glUniform1i(uniform, handles1iv[0]);
		} else {
			glUniform1iv(uniform, count, handles1iv);
		}
	}
	
	return index;
}

static Vec4 defaultTextureColor = { 0, 0, 0, 0 };

GLTextures::GLTextures(GLRender* context) throw(const char*) {
	THIS.context = context;
	list = new HashMap<String, GLTexture>();
	defaultTexture = createSolidTexture(L"null", defaultTextureColor);
}

GLTextures::~GLTextures() {
	list = null;
	defaultTexture = null;
	context = NULL;
}

GLTexture& GLTextures::get(const wchar_t* key) throw(const char*) {
	return (GLTexture&)list.get(key);
}

GLTexture& GLTextures::createSolidTexture(const wchar_t* key, const Vec4& rgba4fv) throw(const char*) {
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
		GLTexture* texture = &(list.put(key, new RefGLTexture(context, &handle, 1, 1, 1)));
		if (wcscmp(key, L"null") == 0) {
			defaultTexture = *texture;
		}
		return *texture;
	} catch (...) {
		glDeleteTextures(1, &handle);
		throw;
	}
}

GLTexture& GLTextures::createTexture(const wchar_t* key, GLuint* handles, GLuint count, GLint width, GLint height) throw(const char*) {
	try {
		list.remove(key);
		GLTexture* texture = &(list.put(key, new RefGLTexture(context, handles, count, 1, 1)));
		if (wcscmp(key, L"null") == 0) {
			defaultTexture = *texture;
		}
		return *texture;
	} catch (...) {
		glDeleteTextures(count, handles);
		throw;
	}
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
