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
	friend class GLProgramTexture;

	friend class GLEngine;
	friend class GLSprite;
	friend class GLModel;
	friend class GLParticleSystem;
	
protected:
	static const char* extensions;
	static bool isExtensionSupported(const char *extension);
	GLuint NPOT(GLuint value);

	GLuint maxTextureIndex = 0;
	GLuint maxTextureSize = 0;
	bool isNPOTSupported = false;

public:
	GLContext() throw(const char*);
	~GLContext();	

protected:
	// Normal, FlipX, FlipY, FlipXY
	GLuint rectArrayBuffers[4] = { GL_NONE, GL_NONE, GL_NONE, GL_NONE };
	
	/* Program & Shader base functions */
	
protected:
	GLContextState state;

	Vector<GLContextState> stackState;
	Vector<GLContextStateViewport> stackViewport;
	Vector<GLContextStateScissor> stackScissor;
	Vector<GLContextStateDepth> stackDepth;
	Vector<GLContextStateStencil> stackStencil;
	Vector<GLContextStateColorMask> stackColorMask;
	Vector<GLContextStateBlend> stackBlend;

	GLFrameBuffer* mainFrameBuffer = NULL;
	GLProgramTexture* programTexture = NULL;
	
protected:
	void release();
	
public:
	void reset() throw(const char*);
	void resize(GLint width, GLint height);
	
public: // State Change/Save/Restore
	void pushState();
	bool popState();
	
	void pushViewport();
	void popViewport();
	void pushScissor();
	void popScissor();
	void pushDepth();
	void popDepth();
	void pushStencil();
	void popStencil();
	void pushColorMask();
	void popColorMask();
	void pushBlend();
	void popBlend();

	void setViewport(GLint left, GLint top, GLint right, GLint bottom);
	
	void enableScissor();
	void disableScissor();
	void setScissor(GLint left, GLint top, GLint right, GLint bottom);
	
	void enableDepth();
	void disableDepth();
	void setDepth(GLclampf depth, GLclampf zNear, GLclampf zFar, GLenum func);
	
	void enableStencil();
	void disableStencil();
	void setStencil(GLenum func, GLint ref, GLuint mask, GLenum fail, GLenum zFail, GLenum zPass);
	void setClearStencil(GLint s);
	
	void setColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
	
	void enableBlend();
	void disableBlend();
	void setBlend(GLenum sFactor, GLenum dFactor, GLenum mode);

public: // Clipping mask by stencil
	// Framebuffer must support stencil buffer
	void beginClipUpdate();
	void clearClip();
	void endClipUpdate();
	void beginClip(bool inverse = false);
	void endClip();
};

#endif //JAPPSY_UGLCONTEXT_H
