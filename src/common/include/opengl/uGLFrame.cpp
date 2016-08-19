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

#include "uGLFrame.h"
#include <core/uSystem.h>
#include <core/uMemory.h>
#include <data/uSmoothValue.h>
#include <opengl/uOpenGL.h>
#include <math.h>

#include <opengl/uGLRender.h>

void defaultOnFrame(GLRender* context, Object& userData) {
}

GLFrame::GLFrame(GLEngine& engine, GLRender* context, onFrameCallback callback, Object& userData) {
	THIS.engine = engine;
	THIS.context = context;
	
	THIS.currentTime = THIS.lastFrame = currentTimeMillis();
	THIS.frames = memNew(THIS.frames, SmoothValue(10));
	THIS.fps = 0;
	
	THIS.onFrame = (callback != NULL) ? callback : defaultOnFrame;
	THIS.userData = userData;
}

GLFrame::~GLFrame() {
	do {
		// Unassign all attributes
		GLuint tempBuffer;
		glGenBuffers(1, &tempBuffer);
		if (glGetError() == GL_OUT_OF_MEMORY)
			break;
		glBindBuffer(GL_ARRAY_BUFFER, tempBuffer);
		GLuint numAttributes = 0;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, (GLint*)&numAttributes);
		for (GLuint attrib = 0; attrib < numAttributes; ++attrib)
			glVertexAttribPointer(attrib, 1, GL_FLOAT, GL_FALSE, 0, NULL);
		glDeleteBuffers(1, &tempBuffer);
	} while (false);
	
	THIS.engine = null;
	THIS.context = NULL;
	
	THIS.currentTime = THIS.lastFrame = 0;
	memDelete(THIS.frames);
	THIS.frames = NULL;
	THIS.fps = 0;
	
	THIS.onFrame = NULL;
}

void GLFrame::loop() {
	THIS.currentTime = currentTimeMillis();
	uint64_t elapsed = THIS.currentTime - THIS.lastFrame;
	if (elapsed > 0)
		THIS.frames->put(1000.0 / elapsed);
	THIS.fps = (uint32_t)floorf(THIS.frames->value());
	
	THIS.onFrame(THIS.context, THIS.userData);
	
	THIS.lastFrame = THIS.currentTime;
}
