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

#include <GLContext.h>

GLContext::GLContext(EAGLContext* context, CAEAGLLayer* layer) throw(const char*) {
	this->context = context;
	this->engine = NULL;
	
	glGenFramebuffers(1, &frameBuffer);
	if (glGetError() == GL_OUT_OF_MEMORY)
		throw eOutOfMemory;
	glGenRenderbuffers(1, &colorRenderBuffer);
	if (glGetError() == GL_OUT_OF_MEMORY) {
		glDeleteFramebuffers(1, &frameBuffer); frameBuffer = 0;
		throw eOutOfMemory;
	}
	glGenRenderbuffers(1, &depthRenderBuffer);
	if (glGetError() == GL_OUT_OF_MEMORY) {
		glDeleteRenderbuffers(1, &colorRenderBuffer); colorRenderBuffer = 0;
		glDeleteFramebuffers(1, &frameBuffer); frameBuffer = 0;
		throw eOutOfMemory;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
	[context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderBuffer);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
	
	try {
		glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
		if (glGetError() == GL_OUT_OF_MEMORY)
			throw eOutOfMemory;
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
		GetGLFramebufferStatus();
	} catch (const char* e) {
		glDeleteRenderbuffers(1, &depthRenderBuffer); depthRenderBuffer = 0;
		glDeleteRenderbuffers(1, &colorRenderBuffer); colorRenderBuffer = 0;
		glDeleteFramebuffers(1, &frameBuffer); frameBuffer = 0;
		throw e;
	}
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)	{
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		release();
		throw eOpenGL;
	}
	
	NSLog(@"%s %s", glGetString(GL_RENDERER), glGetString(GL_VERSION));
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	//render();
	GetGLError();
}

void GLContext::release() {
	if (frameBuffer) {
		glDeleteFramebuffers(1, &frameBuffer); frameBuffer = 0;
	}
	
	if (colorRenderBuffer) {
		glDeleteRenderbuffers(1, &colorRenderBuffer); colorRenderBuffer = 0;
	}
	
	if (depthRenderBuffer) {
		glDeleteRenderbuffers(1, &depthRenderBuffer); depthRenderBuffer = 0;
	}
	
	if (engine != NULL) {
		memDelete(engine);
	}
	engine = NULL;
	context = NULL;
}

GLContext::~GLContext() {
	release();
}

void GLContext::update(CAEAGLLayer* layer) throw(const char*) {
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
	[context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
	
	glDeleteRenderbuffers(1, &depthRenderBuffer); depthRenderBuffer = 0;
	
	glGenRenderbuffers(1, &depthRenderBuffer);
	if (glGetError() == GL_OUT_OF_MEMORY) {
		glDeleteRenderbuffers(1, &colorRenderBuffer); colorRenderBuffer = 0;
		glDeleteFramebuffers(1, &frameBuffer); frameBuffer = 0;
		throw eOutOfMemory;
	}
	
	try {
		glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
		if (glGetError() == GL_OUT_OF_MEMORY)
			throw eOutOfMemory;
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
		GetGLFramebufferStatus();
	} catch (const char* e) {
		glDeleteRenderbuffers(1, &depthRenderBuffer); depthRenderBuffer = 0;
		glDeleteRenderbuffers(1, &colorRenderBuffer); colorRenderBuffer = 0;
		glDeleteFramebuffers(1, &frameBuffer); frameBuffer = 0;
		throw e;
	}
	
	glViewport(0, 0, width, height);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		release();
		throw eOpenGL;
	}
	
	if (engine)
		engine->onUpdate(width, height);
}

static int color = 0;

void GLContext::render() {
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	
	if (engine) {
		engine->onRender();
	} else {
		float c = (float)color / 255.0f;
		glClearColor(c, c, c, 1.0f);
		glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
		color++;
		if (color >= 256) color = 0;
	}
	
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
	[context presentRenderbuffer:GL_RENDERBUFFER];
}

void GLContext::touch(MotionEvent* event) {
	if (engine)
		this->engine->onTouch(event);
}

void GLContext::initialize(GLEngine* engine) {
	this->engine = engine;
	engine->onUpdate(width, height);
	engine->onRender();
}
