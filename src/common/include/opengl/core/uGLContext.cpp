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
	memDelete(defaultFrameBuffer);
	
	for (GLuint index = 0; index < 32; index++) {
		detachTexture(index);
	}
}

void GLContext::reset() {
	if (defaultFrameBuffer != NULL) {
		memDelete(defaultFrameBuffer);
	}
	currentFrameBuffer = defaultFrameBuffer = memNew(defaultFrameBuffer, GLFrameBuffer(*this));

	for (int i = 0; i < 32; i++) {
		attachedTextures[i] = NULL;
	}
	
	activeTextureIndex = 0;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

static GLenum GLTextureIndexes[32] = {
	GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7,
	GL_TEXTURE8, GL_TEXTURE9, GL_TEXTURE10, GL_TEXTURE11, GL_TEXTURE12, GL_TEXTURE13, GL_TEXTURE14, GL_TEXTURE15,
	GL_TEXTURE16, GL_TEXTURE17, GL_TEXTURE18, GL_TEXTURE19, GL_TEXTURE20, GL_TEXTURE21, GL_TEXTURE22, GL_TEXTURE23,
	GL_TEXTURE24, GL_TEXTURE25, GL_TEXTURE26, GL_TEXTURE27, GL_TEXTURE28, GL_TEXTURE29, GL_TEXTURE30, GL_TEXTURE31
};

GLuint GLContext::attachTexture(GLTexture& texture, GLuint index, GLint uniform) throw(const char*) {
	if (texture.handle == GL_NONE) {
		throw eOpenGL;
	}
	
	if (index >= 32) {
		index = 0;
	}
	
	if (attachedTextures[index] == &texture) {
		goto attachTexture_setUniform;
	}
	
	if (activeTextureIndex != index) {
		activeTextureIndex = index;
		glActiveTexture(GLTextureIndexes[index]);
	}
	
	glBindTexture(GL_TEXTURE_2D, texture.handle);
	
	attachedTextures[index] = &texture;
	
attachTexture_setUniform:
	if (uniform != -1) {
		glUniform1i(uniform, index);
	}
	
	return index;
}

void GLContext::detachTexture(GLTexture& texture) throw(const char*) {
	if (texture.handle == GL_NONE) {
		throw eOpenGL;
	}
	
	for (GLuint index = 0; index < 32; index++) {
		if (attachedTextures[index] == &texture) {
			detachTexture(index);
		}
	}
}

bool GLContext::reattachTexture(GLTexture& texture) throw(const char*) {
	if (texture.handle == GL_NONE) {
		throw eOpenGL;
	}
	
	bool result = false;
	
	for (GLuint index = 0; index < 32; index++) {
		if (attachedTextures[index] == &texture) {
			if (activeTextureIndex != index) {
				activeTextureIndex = index;
				glActiveTexture(GLTextureIndexes[index]);
			}
			
			glBindTexture(GL_TEXTURE_2D, texture.handle);
			
			result = true;
		}
	}
	
	return result;
}

GLuint GLContext::attachTexture(GLuint handle) {
	glBindTexture(GL_TEXTURE_2D, handle);
	
	if (attachedTextures[activeTextureIndex] != NULL) {
		return attachedTextures[activeTextureIndex]->handle;
	}
	
	return GL_NONE;
}

void GLContext::detachTexture(GLuint index) {
	if (index >= 32) {
		index = 0;
	}

	if (attachedTextures[index] != NULL) {
		attachedTextures[index] = NULL;
	
		if (activeTextureIndex != index) {
			activeTextureIndex = index;
			glActiveTexture(GLTextureIndexes[index]);
		}
		
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
	}
}

void GLContext::attachFramebuffer(GLFrameBuffer& buffer) throw(const char*) {
	if (buffer.frameBuffer == GL_NONE) {
		throw eOpenGL;
	}
	
	if (currentFrameBuffer != &buffer) {
		detachTexture(buffer);
		if (defaultFrameBuffer != &buffer) {
			buffer.update();
		}
		currentFrameBuffer = &buffer;
		glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
		if (defaultFrameBuffer == &buffer) {
			buffer.update();
		}
	}
}

void GLContext::detachFramebuffer() throw(const char*) {
	if (currentFrameBuffer != defaultFrameBuffer) {
		currentFrameBuffer = defaultFrameBuffer;
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer->frameBuffer);
		defaultFrameBuffer->update();
	}
}

bool GLContext::isAttachedFramebuffer(GLFrameBuffer& buffer) {
	return (currentFrameBuffer == &buffer);
}
