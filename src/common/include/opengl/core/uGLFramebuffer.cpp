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

#include "uGLFramebuffer.h"
#include "uGLContext.h"

GLFrameBuffer::GLFrameBuffer(GLContext& context) throw(const char*) : GLTexture(context) {
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&frameBuffer);
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, (GLint*)&colorRenderBuffer);
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, (GLint*)&depthRenderBuffer);
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, (GLint*)&stencilRenderBuffer);
	
	state |= GLFrameBufferGrabbed;

	if (colorRenderBuffer == GL_NONE) {
		throw eOpenGL;
	}
	
	GLuint restoreRenderBuffer;
	glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint*)&restoreRenderBuffer);
		
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
	
	glBindRenderbuffer(GL_RENDERBUFFER, restoreRenderBuffer);
		
	defaultViewport.set(0, 0, width, height);

	if (depthRenderBuffer != GL_NONE) state |= GLAttachmentDepth;
	if (stencilRenderBuffer != GL_NONE) state |= GLAttachmentStencil;
	state |= GLAttachmentColor | GLDirty;
	
	dirtyRect.set(0, 0, width, height);
}

GLFrameBuffer::GLFrameBuffer(GLContext& context, GLint width, GLint height, uint32_t style, void* data) throw(const char*) : GLTexture(context, width, height, style, data) {
	glGenFramebuffers(1, &frameBuffer);
	CheckGLError();
	context.attachFramebuffer(*this);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle, 0);
	
	if ((style & GLAttachmentStencil) != 0) {
		glGenRenderbuffers(1, &stencilRenderBuffer);
		try {
			CheckGLError();
		} catch (...) {
			destroy();
			throw;
		}
		glBindRenderbuffer(GL_RENDERBUFFER, stencilRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
		try {
			CheckGLError();
		} catch (...) {
			destroy();
			throw;
		}
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRenderBuffer);
	}
	
	if ((style & GLAttachmentDepth) != 0) {
		glGenRenderbuffers(1, &depthRenderBuffer);
		try {
			CheckGLError();
		} catch (...) {
			destroy();
			throw;
		}
		glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
		try {
			CheckGLError();
		} catch (...) {
			destroy();
			throw;
		}
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
	}

	defaultViewport.set(0, 0, width, height);
	
	state |= (style & (GLAttachmentStencil | GLAttachmentDepth)) | GLAttachmentColor;
}

GLFrameBuffer::~GLFrameBuffer() {
	destroy();
}

void GLFrameBuffer::destroy() {
	if ((state & GLFrameBufferGrabbed) == 0) {
		if ((context != NULL) && (frameBuffer != GL_NONE)) {
			context->detachFramebuffer();
		
			if (depthRenderBuffer != GL_NONE) {
				glDeleteRenderbuffers(1, &depthRenderBuffer);
				depthRenderBuffer = GL_NONE;
			}
		
			if (stencilRenderBuffer != GL_NONE) {
				glDeleteRenderbuffers(1, &stencilRenderBuffer);
				stencilRenderBuffer = GL_NONE;
			}
		
			glDeleteFramebuffers(1, &frameBuffer);
			frameBuffer = GL_NONE;
		}
	}
}

void GLFrameBuffer::resize(GLint newWidth, GLint newHeight) {
	if ((width != newWidth) || (height != newHeight) || ((state & GLFrameBufferGrabbed) != 0)) {
		defaultViewport.set(0, 0, newWidth, newHeight);
	
		state |= GLFrameBufferInvalid | GLDirtyResize;
		
		if (context->isAttachedFramebuffer(*this)) {
			update();
		}
	} else if ((defaultViewport.right != newWidth) || (defaultViewport.bottom != newHeight)) {
		defaultViewport.set(0, 0, newWidth, newHeight);
		
		state &= (~(GLFrameBufferInvalid | GLDirtyResize));
	}
}

void GLFrameBuffer::update() throw(const char*) {
	if ((state & GLFrameBufferInvalid) != 0) {
		state = (state & (~GLFrameBufferInvalid));
		
		if ((state & GLFrameBufferGrabbed) != 0) {
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&frameBuffer);
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, (GLint*)&colorRenderBuffer);
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, (GLint*)&depthRenderBuffer);
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, (GLint*)&stencilRenderBuffer);
			
			if (colorRenderBuffer == GL_NONE) {
				throw eOpenGL;
			}
			
			state = (state & (~GLAttachmentDepth)) | ((depthRenderBuffer != GL_NONE) ? GLAttachmentDepth : 0);
			state = (state & (~GLAttachmentStencil)) | ((stencilRenderBuffer != GL_NONE) ? GLAttachmentStencil : 0);
		} else {
			try {
				GLTexture::resize(defaultViewport.right, defaultViewport.bottom);
			} catch (...) {
				destroy();
				throw;
			}
			
			if (depthRenderBuffer != GL_NONE) {
				glDeleteRenderbuffers(1, &depthRenderBuffer);
				depthRenderBuffer = GL_NONE;
			}
			
			if (stencilRenderBuffer != GL_NONE) {
				glDeleteRenderbuffers(1, &stencilRenderBuffer);
				stencilRenderBuffer = GL_NONE;
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle, 0);
			
			if ((state & GLAttachmentStencil) != 0) {
				glGenRenderbuffers(1, &stencilRenderBuffer);
				try {
					CheckGLError();
				} catch (...) {
					destroy();
					throw;
				}
				glBindRenderbuffer(GL_RENDERBUFFER, stencilRenderBuffer);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
				try {
					CheckGLError();
				} catch (...) {
					destroy();
					throw;
				}
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRenderBuffer);
			}
			
			if ((state & GLAttachmentDepth) != 0) {
				glGenRenderbuffers(1, &depthRenderBuffer);
				try {
					CheckGLError();
				} catch (...) {
					destroy();
					throw;
				}
				glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
				try {
					CheckGLError();
				} catch (...) {
					destroy();
					throw;
				}
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
			}
		}
	}
}

void GLFrameBuffer::invalidate() {
	dirtyRect.set(0, 0, width, height);
	state |= GLDirty;
}

void GLFrameBuffer::invalidate(GLint left, GLint top, GLint right, GLint bottom) {
	if ((state & GLDirty) == 0) {
		dirtyRect.set(left, top, right, bottom);
	} else {
		dirtyRect |= GLRect(left, top, right, bottom);
	}
	dirtyRect &= defaultViewport;
	if (!dirtyRect.isEmpty()) {
		state |= GLDirty;
	}
}
