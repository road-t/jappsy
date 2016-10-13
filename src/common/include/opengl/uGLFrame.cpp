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

void defaultOnFrame(GLRender* context, void* userData) {
}

GLFrame::GLFrame(GLEngine* engine, GLRender* context, onFrameCallback callback, void* userData) {
	this->engine = engine;
	this->context = context;
	
	this->currentTime = this->lastFrame = currentTimeMillis();
	this->frames = memNew(this->frames, SmoothValue(10));
	this->fps = 0;
	
	this->onFrame = (callback != NULL) ? callback : (onFrameCallback)defaultOnFrame;
	this->userData = userData;
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
	
	this->engine = NULL;
	this->context = NULL;
	
	this->currentTime = this->lastFrame = 0;
	memDelete(this->frames);
	this->frames = NULL;
	this->fps = 0;
	
	this->onFrame = NULL;
}

void GLFrame::loop() {
	this->currentTime = currentTimeMillis();
	uint64_t elapsed = this->currentTime - this->lastFrame;
	if (elapsed > 0)
		this->frames->put(1000.0 / elapsed);
	this->fps = (uint32_t)floorf(this->frames->value());
	
	this->onFrame(this->context, this->userData);
	
	this->lastFrame = this->currentTime;
}
