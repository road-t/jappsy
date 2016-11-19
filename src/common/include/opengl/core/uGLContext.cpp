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

GLContext::GLContext() {
	reset();
}

GLContext::~GLContext() {
	memDelete(mainFrameBuffer);
	memDelete(programTexture);
}

void GLContext::reset() {
	if (mainFrameBuffer != NULL) {
		memDelete(mainFrameBuffer);
	}
	mainFrameBuffer = memNew(mainFrameBuffer, GLFrameBuffer(*this));

#ifdef DEBUG
	GLuint maxTextureIndex = 0;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)&maxTextureIndex);
	LOG("GLContext: MaxTextureIndex %d", maxTextureIndex);
#endif

	//glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if (programTexture != NULL) {
		memDelete(programTexture);
	}
	programTexture = memNew(programTexture, GLProgramTexture(*this));
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

