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

#include "uGLContext.h"

const char* GLContext::extensions = NULL;

bool GLContext::isExtensionSupported(const char *extension) {
	if (extensions == NULL)
		return false;
	
	const char *start;
	const char *where, *terminator;
	
	where = strchr(extension, ' ');
	if ( where || *extension == '\0' )
		return false;
	
	for ( start = extensions; ; ) {
		where = strstr( start, extension );
		
		if ( !where )
			break;
		
		terminator = where + strlen( extension );
		
		if ( where == start || *(where - 1) == ' ' )
			if ( *terminator == ' ' || *terminator == '\0' )
				return true;
		
		start = terminator;
	}
	
	return false;
}

GLuint GLContext::NPOT(GLuint value) {
	if (isNPOTSupported) {
		if (value >= maxTextureSize) {
			return maxTextureSize;
		}

		return value;
	}
	
	GLuint result = 1;
	while (result < value) {
		result <<= 1;
		
		if (result >= maxTextureSize) {
			return maxTextureSize;
		}
	}
	
	return result;
}

GLContext::GLContext() throw(const char*) {
	reset();
}

GLContext::~GLContext() {
	release();
}

void GLContext::release() {
	for (int i = 0; i < 4; i++) {
		if (rectArrayBuffers[i] != GL_NONE) {
			glBindBuffer(GL_ARRAY_BUFFER, rectArrayBuffers[i]);
			glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
			glDeleteBuffers(1, &rectArrayBuffers[i]);
		}
		glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
	}
	
	if (programTexture != NULL) {
		memDelete(programTexture);
		programTexture = NULL;
	}

	if (mainFrameBuffer != NULL) {
		memDelete(mainFrameBuffer);
		mainFrameBuffer = NULL;
	}
}

void GLContext::reset() throw(const char*) {
	release();
	
	try {
		// Get Context Params
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&maxTextureSize);
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)&maxTextureIndex);
		
		extensions = (const char*)glGetString(GL_EXTENSIONS);
		isNPOTSupported = isExtensionSupported("GL_OES_texture_npot");

		// Create Internal Objects
		mainFrameBuffer = memNew(mainFrameBuffer, GLFrameBuffer(*this));
		programTexture = memNew(programTexture, GLProgramTexture(*this));

		//glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Build Rect Array Buffers
		const GLfloat squareBuffer[4][8] = {
			{ 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f },
			{ 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f }
		};
		glGenBuffers(4, rectArrayBuffers);
		CheckGLError();
		for (int i = 0; i < 4; i++) {
			glBindBuffer(GL_ARRAY_BUFFER, rectArrayBuffers[i]);
			glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), squareBuffer[i], GL_STATIC_DRAW);
		}
		glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
	} catch (...) {
		release();
		throw;
	}
}

void GLContext::resize(GLint width, GLint height) {
	mainFrameBuffer->resize(width, height);
}

void GLContext::pushState() {
	stackState.push(state);
}

bool GLContext::popState() {
	if (stackState.count() > 0) {
		state.setFrom(stackState.pop());
		return true;
	}
	return false;
}

void GLContext::pushViewport() {
	stackViewport.push(state.viewport);
}

void GLContext::popViewport() {
	if (stackViewport.count() > 0) {
		state.setFrom(stackViewport.pop());
	}
}

void GLContext::pushScissor() {
	stackScissor.push(state.scissor);
}

void GLContext::popScissor() {
	if (stackScissor.count() > 0) {
		state.setFrom(stackScissor.pop());
	}
}

void GLContext::pushDepth() {
	stackDepth.push(state.depth);
}

void GLContext::popDepth() {
	if (stackDepth.count() > 0) {
		state.setFrom(stackDepth.pop());
	}
}

void GLContext::pushStencil() {
	stackStencil.push(state.stencil);
}

void GLContext::popStencil() {
	if (stackStencil.count() > 0) {
		state.setFrom(stackStencil.pop());
	}
}

void GLContext::pushColorMask() {
	stackColorMask.push(state.colorMask);
}

void GLContext::popColorMask() {
	if (stackColorMask.count() > 0) {
		state.setFrom(stackColorMask.pop());
	}
}

void GLContext::pushBlend() {
	stackBlend.push(state.blend);
}

void GLContext::popBlend() {
	if (stackBlend.count() > 0) {
		state.setFrom(stackBlend.pop());
	}
}

void GLContext::setViewport(GLint left, GLint top, GLint right, GLint bottom) {
	state.setViewport(left, top, right, bottom);
}

void GLContext::enableScissor() {
	state.enableScissor();
}

void GLContext::disableScissor() {
	state.disableScissor();
}

void GLContext::setScissor(GLint left, GLint top, GLint right, GLint bottom) {
	state.setScissor(left, top, right, bottom);
}

void GLContext::enableDepth() {
	state.enableDepth();
}

void GLContext::disableDepth() {
	state.disableDepth();
}

void GLContext::setDepth(GLclampf depth, GLclampf zNear, GLclampf zFar, GLenum func) {
	state.setDepth(depth, zNear, zFar, func);
}

void GLContext::enableStencil() {
	state.enableStencil();
}

void GLContext::disableStencil() {
	state.disableStencil();
}

void GLContext::setStencil(GLenum func, GLint ref, GLuint mask, GLenum fail, GLenum zFail, GLenum zPass) {
	state.setStencil(func, ref, mask, fail, zFail, zPass);
}

void GLContext::setClearStencil(GLint s) {
	state.setClearStencil(s);
}

void GLContext::setColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
	state.setColorMask(red, green, blue, alpha);
}

void GLContext::enableBlend() {
	state.enableBlend();
}

void GLContext::disableBlend() {
	state.disableBlend();
}

void GLContext::setBlend(GLenum sFactor, GLenum dFactor, GLenum mode) {
	state.setBlend(sFactor, dFactor, mode);
}

void GLContext::beginClipUpdate() {
	stackColorMask.push(state.colorMask);
	stackStencil.push(state.stencil);
	state.setColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	state.enableStencil();
	state.setStencil(GL_ALWAYS, 1, 1, GL_KEEP, GL_KEEP, GL_REPLACE);
}

void GLContext::clearClip() {
	state.setClearStencil(0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_STENCIL_BUFFER_BIT);
}

void GLContext::endClipUpdate() {
	state.setFrom(stackStencil.pop());
	state.setFrom(stackColorMask.pop());
}

void GLContext::beginClip(bool inverse) {
	stackStencil.push(state.stencil);
	state.enableStencil();
	state.setStencil(inverse ? GL_EQUAL : GL_NOTEQUAL, 1, 1, GL_KEEP, GL_KEEP, GL_KEEP);
}

void GLContext::endClip() {
	state.setFrom(stackStencil.pop());
}

