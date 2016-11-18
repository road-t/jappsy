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

#ifndef JAPPSY_UGLFRAMEBUFFER_H
#define JAPPSY_UGLFRAMEBUFFER_H

#include <opengl/core/uGLContextState.h>

class GLContext;

class GLFrameBuffer : public GLTexture {
	friend struct GLContextState;
	friend class GLContext;
	
protected:
	GLuint frameBuffer;
	GLuint renderBuffer;
	GLuint colorRenderBuffer;
	GLuint depthRenderBuffer;
	GLuint stencilRenderBuffer;
	
	GLSize newSize;
	
	GLContextState contextState;

private:
	// Grab current OpenGL Framebuffer (window frame buffer)
	GLFrameBuffer(GLContext& context) throw(const char*);
	
public:
	// Create OpenGL Framebuffer
	GLFrameBuffer(GLContext& context, GLint width, GLint height, uint32_t style = (GLRepeat | GLSmooth), void* data = NULL) throw(const char*);
	
	~GLFrameBuffer();
	
private:
	void destroy();
	
public:
	void resize(GLint newWidth, GLint newHeight);
private:
	void validate() throw(const char*);
};

#endif //JAPPSY_UGLFRAMEBUFFER_H
