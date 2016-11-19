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

#ifndef JAPPSY_UGLCONTEXT_H
#define JAPPSY_UGLCONTEXT_H

#include <opengl/core/uGLFramebuffer.h>
#include <opengl/core/uGLProgram.h>

class GLContext : public CObject {
	friend class GLTexture;
	friend class GLFrameBuffer;
	
public:
	GLContext();
	~GLContext();
	
public:
	GLContextState state;

	Vector<GLContextState> stackState;
	Vector<GLContextStateViewport> stackViewport;
	Vector<GLContextStateScissor> stackScissor;
	Vector<GLContextStateDepth> stackDepth;
	Vector<GLContextStateStencil> stackStencil;
	Vector<GLContextStateColorMask> stackColorMask;
	Vector<GLContextStateBlend> stackBlend;

public:
	GLFrameBuffer* mainFrameBuffer = NULL;
	GLProgramTexture* programTexture = NULL;
	
	void reset();
	void resize(GLint width, GLint height);

	void pushState();
	bool popState();
		
	// Framebuffer must support stencil buffer
	void beginClipUpdate();
	void clearClip();
	void endClipUpdate();
	void beginClip(bool inverse = false);
	void endClip();
};

#endif //JAPPSY_UGLCONTEXT_H
