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
#include "uGLContext.h"

GLTexture::GLTexture(GLContext& context) {
	this->context = &context;
	handle = GL_NONE;
}

GLTexture::GLTexture(GLContext& context, uint32_t rgba) throw(const char*) {
	glGenTextures(1, &handle);
	CheckGLError();
	
	GLuint restore = context.state.attachTemporaryTexture(handle);
	
	uint8_t data[4];
	*((uint32_t*)data) = rgba;
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
	try {
		CheckGLError();
	} catch (...) {
		glBindTexture(GL_TEXTURE_2D, restore);
		glDeleteTextures(1, &handle);
		handle = GL_NONE;
		throw;
	}
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, restore);

	width = height = 1;
	state = GLRepeat | GLSmoothNone;
	this->context = &context;
}

GLTexture::GLTexture(GLContext& context, const Vec4& rgba) throw(const char*) {
	glGenTextures(1, &handle);
	CheckGLError();
	
	GLuint restore = context.state.attachTemporaryTexture(handle);
	
	uint8_t data[4] = { (uint8_t)(rgba.r * 255), (uint8_t)(rgba.g * 255), (uint8_t)(rgba.b * 255), (uint8_t)(rgba.a * 255) };
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
	try {
		CheckGLError();
	} catch (...) {
		glBindTexture(GL_TEXTURE_2D, restore);
		glDeleteTextures(1, &handle);
		handle = GL_NONE;
		throw;
	}
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, restore);
	
	width = height = 1;
	state |= GLRepeat | GLSmoothNone;
	this->context = &context;
}

GLTexture::GLTexture(GLContext& context, uint32_t width, uint32_t height, uint32_t mode, void* data) throw(const char*) {
	GLubyte* pixelData = (GLubyte*)data;
	if (pixelData == NULL) {
		uint32_t dataSize = width * height * 4;
		pixelData = (GLubyte *)mmalloc(dataSize * sizeof(GLubyte));
		if (pixelData == NULL) {
			throw eOutOfMemory;
		}
#ifdef DEBUG
		for (uint32_t i = 0; i < dataSize; i += 4) {
			pixelData[i] = pixelData[i+1] = pixelData[i+2] = rand() % 256;
			pixelData[i+3] = rand() % 256;
		}
#endif
		dirtyRect.set(0, 0, width, height);
		state |= GLDirty;
	}
	
	glGenTextures(1, &handle);
	CheckGLError();

	GLuint restore = context.state.attachTemporaryTexture(handle);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
	if (data == NULL) {
		mmfree(pixelData);
	}
	try {
		CheckGLError();
	} catch (...) {
		glBindTexture(GL_TEXTURE_2D, restore);
		glDeleteTextures(1, &handle);
		handle = GL_NONE;
		throw;
	}

	this->width = width;
	this->height = height;
	setMode(mode);
	validate();
	glBindTexture(GL_TEXTURE_2D, restore);

	this->context = &context;
}

void GLTexture::resize(uint32_t newWidth, uint32_t newHeight) throw(const char*) {
	if ((newWidth != width) || (newHeight != height)) {
		uint32_t dataSize = newWidth * newHeight * 4;
		GLubyte* pixelData = (GLubyte *)mmalloc(dataSize * sizeof(GLubyte));
		if (pixelData == NULL) {
			throw eOutOfMemory;
		}
	
		if (handle != GL_NONE) {
			glDeleteTextures(1, &handle);
			handle = GL_NONE;
		}

		glGenTextures(1, &handle);
		try {
			CheckGLError();
		} catch (...) {
			destroy();
			throw;
		}

		GLuint restore = context->state.attachTemporaryTexture(handle);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
		mmfree(pixelData);
		try {
			CheckGLError();
		} catch (...) {
			glBindTexture(GL_TEXTURE_2D, restore);
			destroy();
			throw;
		}
	
		width = newWidth;
		height = newHeight;
		dirtyRect.set(0, 0, newWidth, newHeight);
		state |= GLTextureInvalid | GLDirty;
		validate();
		glBindTexture(GL_TEXTURE_2D, restore);

		context->state.reattachTexture(*this);
	}
}

GLTexture::~GLTexture() {
	destroy();
}

void GLTexture::destroy() {
	if ((context != NULL) && (handle != GL_NONE)) {
		context->state.detachTexture(*this);
		glDeleteTextures(1, &handle);
		handle = GL_NONE;
	}
}

void GLTexture::validate() {
	if ((state & GLTextureInvalid) != 0) {
		state &= ~GLTextureInvalid;
		
		GLuint restore = context->state.attachTemporaryTexture(handle);
		
		if ((state & GLSmoothMin) != 0) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
	
		if ((state & GLSmoothMag) != 0) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
	
		if ((state & GLRepeatMirroredX) == GLRepeatX) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		} else if ((state & GLRepeatMirroredX) == GLRepeatMirroredX) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		}
	
		if ((state & GLRepeatMirroredY) == GLRepeatY) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		} else if ((state & GLRepeatMirroredY) == GLRepeatMirroredY) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		
		glBindTexture(GL_TEXTURE_2D, restore);
	}
}

void GLTexture::setMode(uint32_t mode) throw(const char*) {
	state = (state & (~GLRepeatMask)) | (mode & GLRepeatMask) | GLTextureInvalid;
}

void GLTexture::setOnUpdateRectCallback(onUpdateRect callback, void* userData) {
	updateRect = callback;
	updateRectUserData = userData;
}

void GLTexture::dirty() {
	dirtyRect.set(0, 0, width, height);
	state |= GLDirty;
}

void GLTexture::dirty(GLint left, GLint top, GLint right, GLint bottom) {
	if ((state & GLDirty) == 0) {
		dirtyRect.set(left, top, right, bottom);
	} else {
		dirtyRect |= GLRect(left, top, right, bottom);
	}
	dirtyRect &= GLRect(0, 0, width, height);
	if (!dirtyRect.isEmpty()) {
		state |= GLDirty;
	}
}
