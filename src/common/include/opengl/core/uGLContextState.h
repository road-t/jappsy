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

#ifndef JAPPSY_UGLCONTEXTSTATE_H
#define JAPPSY_UGLCONTEXTSTATE_H

#include <opengl/core/uGLTypes.h>
#include <opengl/core/uGLTexture.h>

class GLContext;
class GLFrameBuffer;

struct GLContextStateViewport {
	GLRect rect;

	inline GLContextStateViewport() {}
	
	inline GLContextStateViewport(const GLContextStateViewport& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateViewport));
	}
	
	inline GLContextStateViewport& operator =(const GLContextStateViewport& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateViewport));
		return *this;
	}
	
	inline bool operator ==(const GLContextStateViewport& rhs) const {
		return memcmp(this, &rhs, sizeof(GLContextStateViewport)) == 0;
	}
};

struct GLContextStateScissor {
	GLboolean enabled;
	GLRect rect;
	
	inline GLContextStateScissor() {}
	
	inline GLContextStateScissor(const GLContextStateScissor& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateScissor));
	}
	
	inline GLContextStateScissor& operator =(const GLContextStateScissor& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateScissor));
		return *this;
	}
	
	inline bool operator ==(const GLContextStateScissor& rhs) const {
		return memcmp(this, &rhs, sizeof(GLContextStateScissor)) == 0;
	}
};

struct GLContextStateDepth {
	GLboolean enabled;
	GLclampf depth;
	GLclampf zNear;
	GLclampf zFar;
	GLenum func;
	
	inline GLContextStateDepth() {}
	
	inline GLContextStateDepth(const GLContextStateDepth& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateDepth));
	}
	
	inline GLContextStateDepth& operator =(const GLContextStateDepth& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateDepth));
		return *this;
	}
	
	inline bool operator ==(const GLContextStateDepth& rhs) const {
		return memcmp(this, &rhs, sizeof(GLContextStateDepth)) == 0;
	}
};

struct GLContextStateStencil {
	GLboolean enabled;
	GLenum func;
	GLint ref;
	GLuint mask;
	GLenum fail;
	GLenum zFail;
	GLenum zPass;
	GLint clear;

	inline GLContextStateStencil() {}
	
	inline GLContextStateStencil(const GLContextStateStencil& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateStencil));
	}
	
	inline GLContextStateStencil& operator =(const GLContextStateStencil& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateStencil));
		return *this;
	}

	inline bool operator ==(const GLContextStateStencil& rhs) const {
		return memcmp(this, &rhs, sizeof(GLContextStateStencil)) == 0;
	}
};

struct GLContextStateColorMask {
	union {
		struct {
			GLboolean red;
			GLboolean green;
			GLboolean blue;
			GLboolean alpha;
		};
		
		GLboolean mask[4];
	};

	inline GLContextStateColorMask() {}
	
	inline GLContextStateColorMask(const GLContextStateColorMask& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateColorMask));
	}
	
	inline GLContextStateColorMask& operator =(const GLContextStateColorMask& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateColorMask));
		return *this;
	}
	
	inline bool operator ==(const GLContextStateColorMask& rhs) const {
		return memcmp(this, &rhs, sizeof(GLContextStateColorMask)) == 0;
	}
};

struct GLContextStateBlend {
	GLboolean enabled;
	GLenum sFactor;
	GLenum dFactor;
	GLenum mode;

	inline GLContextStateBlend() {}
	
	inline GLContextStateBlend(const GLContextStateBlend& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateBlend));
	}
	
	inline GLContextStateBlend& operator =(const GLContextStateBlend& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateBlend));
		return *this;
	}
	
	inline bool operator ==(const GLContextStateBlend& rhs) const {
		return memcmp(this, &rhs, sizeof(GLContextStateBlend)) == 0;
	}
};

struct GLContextStateTexture {
	GLuint active;
	GLTexture* attached[GLActiveTextureLimit];
	
	inline GLContextStateTexture() {}
	
	inline GLContextStateTexture(const GLContextStateTexture& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateTexture));
	}
	
	inline GLContextStateTexture& operator =(const GLContextStateTexture& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateTexture));
		return *this;
	}
	
	inline bool operator ==(const GLContextStateTexture& rhs) const {
		return memcmp(this, &rhs, sizeof(GLContextStateTexture)) == 0;
	}
};

struct GLContextStateFrameBuffer {
	GLFrameBuffer* attached;

	inline GLContextStateFrameBuffer() {}
	
	inline GLContextStateFrameBuffer(const GLContextStateFrameBuffer& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateFrameBuffer));
	}
	
	inline GLContextStateFrameBuffer& operator =(const GLContextStateFrameBuffer& rhs) {
		memcpy(this, &rhs, sizeof(GLContextStateFrameBuffer));
		return *this;
	}
	
	inline bool operator ==(const GLContextStateFrameBuffer& rhs) const {
		return memcmp(this, &rhs, sizeof(GLContextStateFrameBuffer)) == 0;
	}
};

struct GLContextState {
	GLContextStateViewport viewport;
	GLContextStateScissor scissor;
	GLContextStateDepth depth;
	GLContextStateStencil stencil;
	GLContextStateColorMask colorMask;
	GLContextStateBlend blend;
	GLContextStateTexture textures;
	GLContextStateFrameBuffer frameBuffer;
	
	GLContextState();

	inline GLContextState(const GLContextState& current) {
		memcpy(this, &current, sizeof(struct GLContextState));
	}
	
	inline GLContextState& operator =(const GLContextState& rhs) {
		memcpy(this, &rhs, sizeof(struct GLContextState));
		return *this;
	}
	
	inline bool operator ==(const GLContextState& rhs) const {
		return memcmp(this, &rhs, sizeof(GLContextState)) == 0;
	}
	
	void restoreDefault();
	void setFrom(const GLContextState& restore);
	void setFrom(const GLContextStateViewport& restore);
	void setFrom(const GLContextStateScissor& restore);
	void setFrom(const GLContextStateDepth& restore);
	void setFrom(const GLContextStateStencil& restore);
	void setFrom(const GLContextStateColorMask& restore);
	void setFrom(const GLContextStateBlend& restore);
	void setFrom(const GLContextStateTexture& restore);
	void setFrom(const GLContextStateFrameBuffer& restore);
	
	/* Base Functions */
	
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
	
	GLuint attachTexture(GLTexture& texture, GLuint index = 0, GLint uniform = -1) throw(const char*);
	void detachTexture(GLTexture& texture) throw(const char*);
	bool reattachTexture(GLTexture& texture) throw(const char*);
	GLuint attachTemporaryTexture(GLuint handle);
	void detachTextureIndex(GLuint index);
};

#endif //JAPPSY_UGLCONTEXTSTATE_H
