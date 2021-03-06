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

static GLenum GLTextureIndexes[32] = {
	GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7,
	GL_TEXTURE8, GL_TEXTURE9, GL_TEXTURE10, GL_TEXTURE11, GL_TEXTURE12, GL_TEXTURE13, GL_TEXTURE14, GL_TEXTURE15,
	GL_TEXTURE16, GL_TEXTURE17, GL_TEXTURE18, GL_TEXTURE19, GL_TEXTURE20, GL_TEXTURE21, GL_TEXTURE22, GL_TEXTURE23,
	GL_TEXTURE24, GL_TEXTURE25, GL_TEXTURE26, GL_TEXTURE27, GL_TEXTURE28, GL_TEXTURE29, GL_TEXTURE30, GL_TEXTURE31
};

GLTexture::GLTexture(GLContext& context) {
	this->context = &context;
	handle = GL_NONE;
	rectArrayBuffer = GL_NONE;
}

GLTexture::GLTexture(GLContext& context, uint32_t rgba) throw(const char*) {
	rectArrayBuffer = GL_NONE;
	glGenTextures(1, &handle);
	CheckGLError();

	glBindTexture(GL_TEXTURE_2D, handle);
	
	uint8_t data[4];
	*((uint32_t*)data) = rgba;
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
	try {
		CheckGLError();
	} catch (...) {
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		glDeleteTextures(1, &handle);
		handle = GL_NONE;
		throw;
	}
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	
	width = height = bufferWidth = bufferHeight = 1;
	state = GLRepeat | GLSmoothNone | GLFlipNone;
	this->context = &context;
}

GLTexture::GLTexture(GLContext& context, const Vec4& rgba) throw(const char*) {
	rectArrayBuffer = GL_NONE;
	glGenTextures(1, &handle);
	CheckGLError();
	
	glBindTexture(GL_TEXTURE_2D, handle);
	
	uint8_t data[4] = { (uint8_t)(rgba.r * 255), (uint8_t)(rgba.g * 255), (uint8_t)(rgba.b * 255), (uint8_t)(rgba.a * 255) };
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
	try {
		CheckGLError();
	} catch (...) {
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		glDeleteTextures(1, &handle);
		handle = GL_NONE;
		throw;
	}
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	
	width = height = bufferWidth = bufferHeight = 1;
	state |= GLRepeat | GLSmoothNone | GLFlipNone;
	this->context = &context;
}

GLTexture::GLTexture(GLContext& context, GLint width, GLint height, uint32_t mode, void* data) throw(const char*) {
	handle = GL_NONE;
	rectArrayBuffer = GL_NONE;
	
	GLuint w = (GLuint)abs(width);
	GLuint h = (GLuint)abs(height);
	
	bufferWidth = context.NPOT(w);
	bufferHeight = context.NPOT(h);
	
	uint32_t* pixelData = (uint32_t*)data;
	if ((pixelData == NULL) || ((bufferWidth != w) || (bufferHeight != h))) {
		uint32_t dataSize = bufferWidth * bufferHeight * sizeof(uint32_t);
		pixelData = (uint32_t*)mmalloc(dataSize);
		if (pixelData == NULL) {
			throw eOutOfMemory;
		}
#ifdef DEBUG
		for (GLuint y = 0; y < bufferHeight; y++) {
			for (GLuint x = 0; x < bufferWidth; x++) {
				pixelData[x + y * bufferWidth] = 0xFF000000 | ((x * 255 / bufferWidth) & 0xFF) | (((y * 255 / bufferHeight) & 0xFF) << 8) | (((x/8 + y/8) & 0x1) != 0 ? 0xFF0000 : 0);
			}
		}
		/*
		for (uint32_t i = 0; i < dataSize; i += 4) {
			pixelData[i] = pixelData[i+1] = pixelData[i+2] = rand() % 256;
			pixelData[i+3] = rand() % 256;
		}
		 */
#endif
		dirtyRect.set(0, 0, w, h);
		state |= GLDirty;
		
		/*
		if (data != NULL) {
			GLubyte* src = (GLubyte*)data;
			GLubyte* dst = (GLubyte*)pixelData;
			uint32_t lineSize = ((bufferWidth < width) ? bufferWidth : width) * 4;
			for (uint32_t i = h; i > 0; i--) {
				memcpy(dst, src, lineSize * sizeof(GLubyte));
				src += w;
				dst +=
			}
		}
		 */
	}
	
	glGenTextures(1, &handle);
	CheckGLError();

	glBindTexture(GL_TEXTURE_2D, handle);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bufferWidth, bufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLubyte*)pixelData);
	if (data == NULL) {
		mmfree(pixelData);
	}
	try {
		CheckGLError();
		glGenBuffers(1, &rectArrayBuffer);
		CheckGLError();
	} catch (...) {
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		glDeleteTextures(1, &handle);
		handle = GL_NONE;
		throw;
	}
	
	GLfloat tw = (GLfloat)w / (GLfloat)bufferWidth;
	GLfloat th = (GLfloat)h / (GLfloat)bufferHeight;
	
	const GLfloat squareBuffer[8] = { 0.0f, 0.0f, tw, 0.0f, 0.0f, th, tw, th };
	glBindBuffer(GL_ARRAY_BUFFER, rectArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), squareBuffer, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

	this->width = w;
	this->height = h;
	setMode(mode);
	if (width < 0) state |= GLFlipX;
	if (height < 0) state |= GLFlipY;
	validate();
	glBindTexture(GL_TEXTURE_2D, GL_NONE);

	this->context = &context;
}

void GLTexture::resize(GLint newWidth, GLint newHeight) throw(const char*) {
	if ((newWidth != width) || (newHeight != height)) {
		GLuint w = abs(newWidth);
		GLuint h = abs(newHeight);

		GLuint bw = context->NPOT(w);
		GLuint bh = context->NPOT(h);
		
		if ((bw != bufferWidth) || (bh != bufferHeight)) {
			bufferWidth = bw;
			bufferHeight = bh;
			
			uint32_t dataSize = bw * bh * sizeof(uint32_t);
			uint32_t* pixelData = (uint32_t*)mmalloc(dataSize);
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
				release();
				throw;
			}

			glBindTexture(GL_TEXTURE_2D, handle);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bw, bh, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLubyte*)pixelData);
			mmfree(pixelData);
			try {
				CheckGLError();
			} catch (...) {
				glBindTexture(GL_TEXTURE_2D, GL_NONE);
				release();
				throw;
			}
	
			state |= GLTextureInvalid | GLDirty;
			validate();
			glBindTexture(GL_TEXTURE_2D, GL_NONE);
		}
		
		GLfloat tw = (GLfloat)w / (GLfloat)bufferWidth;
		GLfloat th = (GLfloat)h / (GLfloat)bufferHeight;
		
		const GLfloat squareBuffer[8] = { 0.0f, 0.0f, tw, 0.0f, 0.0f, th, tw, th };
		glBindBuffer(GL_ARRAY_BUFFER, rectArrayBuffer);
		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), squareBuffer, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
		
		width = w;
		height = h;
		dirtyRect.set(0, 0, w, h);
		state &= ~GLFlipMask;
		if (newWidth < 0) state |= GLFlipX;
		if (newHeight < 0) state |= GLFlipY;
	}
}

GLTexture::~GLTexture() {
	release();
}

void GLTexture::release() {
	if ((context != NULL) && (handle != GL_NONE)) {
		glDeleteTextures(1, &handle);
		handle = GL_NONE;
		
		if (rectArrayBuffer != NULL) {
			glBindBuffer(GL_ARRAY_BUFFER, rectArrayBuffer);
			glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
			glDeleteBuffers(1, &rectArrayBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
		}
	}
}

void GLTexture::validate() {
	if ((state & GLTextureInvalid) != 0) {
		state &= ~GLTextureInvalid;
		
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
	}
}

void GLTexture::setMode(uint32_t mode) throw(const char*) {
	state = (state & (~(GLRepeatMask | GLSmoothMask))) | (mode & (GLRepeatMask | GLSmoothMask)) | GLTextureInvalid;
}

void GLTexture::setOnUpdateRectCallback(onRectCallback callback, void* userData) {
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

void GLTexture::update() {
	if ((state & (GLDirty | GLDirtyResize)) != 0) {
		if (updateRect != NULL) {
			if ((state & GLDirtyResize) != 0) {
				dirtyRect.set(0, 0, width, height);
			}

			updateRect(this, dirtyRect, updateRectUserData);
		}
		
		dirtyRect.set(0, 0, 0, 0);
		state &= ~(GLDirty | GLDirtyResize);
	}
}

void GLTexture::attachTexture(GLuint index) {
	glActiveTexture(GLTextureIndexes[index]);
	glBindTexture(GL_TEXTURE_2D, handle);
	validate();
}

void GLTexture::detachTexture(GLuint index) {
	glActiveTexture(GLTextureIndexes[index]);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
}
