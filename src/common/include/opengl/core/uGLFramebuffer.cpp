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
	state |= GLFrameBufferGrabbed;

#if defined(__IOS__)
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&frameBuffer);
	glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint*)&renderBuffer);

	GLint type = GL_NONE;

	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
	if (type != GL_NONE) {
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, (GLint*)&depthRenderBuffer);
	} else {
		depthRenderBuffer = GL_NONE;
	}

	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
	if (type != GL_NONE) {
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, (GLint*)&stencilRenderBuffer);
	} else {
		stencilRenderBuffer = GL_NONE;
	}

	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
	if (type == GL_NONE) {
		throw eOpenGL;
	}
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, (GLint*)&colorRenderBuffer);

	if (type == GL_TEXTURE) {
		if (depthRenderBuffer != GL_NONE) {
			glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
			glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, (GLint*)&width);
			glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, (GLint*)&height);
		
			glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
		} else if (stencilRenderBuffer != NULL) {
			glBindRenderbuffer(GL_RENDERBUFFER, stencilRenderBuffer);
			glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, (GLint*)&width);
			glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, (GLint*)&height);
			
			glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
		} else {
			throw eOpenGL;
		}
	} else {
		glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, (GLint*)&width);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, (GLint*)&height);
		
		glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
	}

	bufferWidth = width;
	bufferHeight = height;
	
	if (depthRenderBuffer != GL_NONE) state |= GLAttachmentDepth;
	if (stencilRenderBuffer != GL_NONE) state |= GLAttachmentStencil;
	state |= GLAttachmentColor | GLDirty;
	
	dirtyRect.set(0, 0, width, height);

	context.state.restoreDefault();
#elif defined(__JNI__)
	newSize.set(0, 0);
	frameBuffer = renderBuffer = depthRenderBuffer = stencilRenderBuffer = colorRenderBuffer = GL_NONE;
	state |= GLFrameBufferInvalid | GLAttachmentColor | GLDirtyResize | GLFrameBufferRestore;
#else
	#error Unsupported platform!
#endif
	
	projection16fv.set(0);
	projection16fv[0] = 2.0f / bufferWidth;
	projection16fv[5] = -2.0f / bufferHeight;
	projection16fv[12] = -1.0f;
	projection16fv[13] = 1.0f;
	projection16fv[10] = projection16fv[15] = 1.0f;
}

GLFrameBuffer::GLFrameBuffer(GLContext& context, GLint width, GLint height, uint32_t style, void* data) throw(const char*) : GLTexture(context, width, -height, style, data) {
	renderBuffer = GL_NONE;
	glGenFramebuffers(1, &frameBuffer);
	CheckGLError();
	
	GLuint restoreFrameBuffer;
	GLuint restoreRenderBuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&restoreFrameBuffer);
	glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint*)&restoreRenderBuffer);
	
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle, 0);
	
	if ((style & GLAttachmentStencil) != 0) {
		glGenRenderbuffers(1, &stencilRenderBuffer);
		try {
			CheckGLError();
		} catch (...) {
			glBindFramebuffer(GL_FRAMEBUFFER, restoreFrameBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, restoreRenderBuffer);
			release();
			throw;
		}
		glBindRenderbuffer(GL_RENDERBUFFER, stencilRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, bufferWidth, bufferHeight);
		try {
			CheckGLError();
		} catch (...) {
			glBindFramebuffer(GL_FRAMEBUFFER, restoreFrameBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, restoreRenderBuffer);
			release();
			throw;
		}
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRenderBuffer);
	}
	
	if ((style & GLAttachmentDepth) != 0) {
		glGenRenderbuffers(1, &depthRenderBuffer);
		try {
			CheckGLError();
		} catch (...) {
			glBindFramebuffer(GL_FRAMEBUFFER, restoreFrameBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, restoreRenderBuffer);
			release();
			throw;
		}
		glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, bufferWidth, bufferHeight);
		try {
			CheckGLError();
		} catch (...) {
			glBindFramebuffer(GL_FRAMEBUFFER, restoreFrameBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, restoreRenderBuffer);
			release();
			throw;
		}
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, restoreFrameBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, restoreRenderBuffer);
	
	state |= (style & (GLAttachmentStencil | GLAttachmentDepth)) | GLAttachmentColor;
	
	projection16fv.set(0);
	projection16fv[0] = 2.0f / bufferWidth;
	projection16fv[5] = -2.0f / bufferHeight;
	projection16fv[12] = -1.0f;
	projection16fv[13] = 1.0f;
	projection16fv[10] = projection16fv[15] = 1.0f;
}

GLFrameBuffer::~GLFrameBuffer() {
	release();
}

void GLFrameBuffer::release() {
	if ((state & GLFrameBufferGrabbed) == 0) {
		if ((context != NULL) && (frameBuffer != GL_NONE)) {
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
		newSize.set(newWidth, newHeight);
		state |= GLFrameBufferInvalid;
	} else if ((state & (GLFrameBufferInvalid | GLDirtyResize)) == (GLFrameBufferInvalid | GLDirtyResize)) {
		if ((width == newWidth) && (height != newHeight)) {
			state &= ~GLFrameBufferInvalid;
		}
	}
}

void GLFrameBuffer::validate() throw(const char*) {
	if ((state & GLFrameBufferInvalid) != 0) {
		state = (state & (~GLFrameBufferInvalid)) | GLDirtyResize;
		
		if ((state & GLFrameBufferGrabbed) != 0) {
			if ((state & GLFrameBufferRestore) != 0) {
				state &= ~GLFrameBufferRestore;
				context->state.restoreDefault();
			}

			glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&frameBuffer);
			glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint*)&renderBuffer);

#if defined(__IOS__)
			GLint type = GL_NONE;

			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
			if (type != GL_NONE) {
				glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, (GLint*)&depthRenderBuffer);
			} else {
				depthRenderBuffer = GL_NONE;
			}

			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
			if (type != GL_NONE) {
				glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, (GLint*)&stencilRenderBuffer);
			} else {
				stencilRenderBuffer = GL_NONE;
			}

			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
			if (type == GL_NONE) {
				throw eOpenGL;
			}
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, (GLint*)&colorRenderBuffer);
			
			if (type == GL_TEXTURE) {
				if (depthRenderBuffer != GL_NONE) {
					glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
					glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, (GLint*)&width);
					glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, (GLint*)&height);
					
					glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
				} else if (stencilRenderBuffer != NULL) {
					glBindRenderbuffer(GL_RENDERBUFFER, stencilRenderBuffer);
					glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, (GLint*)&width);
					glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, (GLint*)&height);
					
					glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
				} else {
					throw eOpenGL;
				}
			} else {
				glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffer);
				glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, (GLint*)&width);
				glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, (GLint*)&height);
				
				glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
			}
			
			bufferWidth = width;
			bufferHeight = height;

			state = (state & (~GLAttachmentDepth)) | ((depthRenderBuffer != GL_NONE) ? GLAttachmentDepth : 0);
			state = (state & (~GLAttachmentStencil)) | ((stencilRenderBuffer != GL_NONE) ? GLAttachmentStencil : 0);
#elif defined(__JNI__)
			bufferWidth = width = newSize.width;
			bufferHeight = height = newSize.height;
			state |= GLAttachmentDepth;
#else
			#error Unsupported platform!
#endif
		} else {
			try {
				GLTexture::resize(newSize.width, newSize.height);
			} catch (...) {
				release();
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

			//glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle, 0);
			
			if ((state & GLAttachmentStencil) != 0) {
				glGenRenderbuffers(1, &stencilRenderBuffer);
				try {
					CheckGLError();
				} catch (...) {
					release();
					throw;
				}
				glBindRenderbuffer(GL_RENDERBUFFER, stencilRenderBuffer);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, bufferWidth, bufferHeight);
				try {
					CheckGLError();
				} catch (...) {
					release();
					throw;
				}
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRenderBuffer);
			}
			
			if ((state & GLAttachmentDepth) != 0) {
				glGenRenderbuffers(1, &depthRenderBuffer);
				try {
					CheckGLError();
				} catch (...) {
					release();
					throw;
				}
				glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, bufferWidth, bufferHeight);
				try {
					CheckGLError();
				} catch (...) {
					release();
					throw;
				}
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
			}
			
			glBindRenderbuffer(GL_RENDERBUFFER, GL_NONE);
		}

		projection16fv.set(0);
		projection16fv[0] = 2.0f / bufferWidth;
		projection16fv[5] = -2.0f / bufferHeight;
		projection16fv[12] = -1.0f;
		projection16fv[13] = 1.0f;
		projection16fv[10] = projection16fv[15] = 1.0f;
	}
}

void GLFrameBuffer::setOnPrepareRectCallback(onRectCallback callback, void* userData) {
	prepareRect = callback;
	prepareRectUserData = userData;
}

bool GLFrameBuffer::update() throw(const char*) {
	if ((state & (GLDirty | GLDirtyResize | GLFrameBufferInvalid)) != 0) {
		if (updateRect != NULL) {
			if ((state & GLFrameBufferInvalid) != 0) {
				dirtyRect.set(0, 0, abs(newSize.width), abs(newSize.height));
			} else if ((state & GLDirtyResize) != 0) {
				dirtyRect.set(0, 0, width, height);
			}
			
			if (prepareRect != NULL) {
				if ((state & GLFrameBufferGrabbed) != 0) {
					GLuint restoreFrameBuffer;
					GLuint restoreRenderBuffer;
					glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&restoreFrameBuffer);
					glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint*)&restoreRenderBuffer);
				
					prepareRect(this, dirtyRect, prepareRectUserData);
				
					glBindFramebuffer(GL_FRAMEBUFFER, restoreFrameBuffer);
					glBindRenderbuffer(GL_RENDERBUFFER, restoreRenderBuffer);
				} else {
					prepareRect(this, dirtyRect, prepareRectUserData);
				}
			}
			
			if ((state & GLFrameBufferGrabbed) == 0) {
				glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
			}
			try {
				validate();
			} catch (...) {
				glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
				throw;
			}
			context->state.setSize(width, height);
			context->state.setViewport(0, 0, bufferWidth, bufferHeight);
			
			context->stackScissor.push(context->state.scissor);
			context->state.setScissor(dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom);
			context->state.enableScissor();

			updateRect(this, dirtyRect, updateRectUserData);
			
			context->state.setFrom(context->stackScissor.pop());
			
			glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

			dirtyRect.set(0, 0, 0, 0);
			state &= ~(GLDirty | GLDirtyResize);
			
			return true;
		}
	}
	
	return false;
}
