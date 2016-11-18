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

#include "uGLContextState.h"
#include "uGLFramebuffer.h"

static GLenum GLTextureIndexes[32] = {
	GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7,
	GL_TEXTURE8, GL_TEXTURE9, GL_TEXTURE10, GL_TEXTURE11, GL_TEXTURE12, GL_TEXTURE13, GL_TEXTURE14, GL_TEXTURE15,
	GL_TEXTURE16, GL_TEXTURE17, GL_TEXTURE18, GL_TEXTURE19, GL_TEXTURE20, GL_TEXTURE21, GL_TEXTURE22, GL_TEXTURE23,
	GL_TEXTURE24, GL_TEXTURE25, GL_TEXTURE26, GL_TEXTURE27, GL_TEXTURE28, GL_TEXTURE29, GL_TEXTURE30, GL_TEXTURE31
};

GLContextState::GLContextState() {
	viewport.rect.set(0, 0, 0, 0);
	
	scissor.enabled = GL_FALSE;
	scissor.rect.set(0, 0, 0, 0);
	
	depth.enabled = GL_FALSE;
	depth.depth = 1.0;
	depth.zNear = 0.0;
	depth.zFar = 1.0;
	depth.func = GL_LEQUAL;

	stencil.func = GL_ALWAYS;
	stencil.ref = 1;
	stencil.mask = 1;
	stencil.fail = GL_KEEP;
	stencil.zFail = GL_KEEP;
	stencil.zPass = GL_REPLACE;
	stencil.clear = 0;

	colorMask.red = colorMask.green = colorMask.blue = colorMask.alpha = GL_TRUE;
	blend.enabled = GL_FALSE;
	blend.sFactor = GL_ONE; // GL_SRC_ALPHA
	blend.dFactor = GL_ONE_MINUS_SRC_ALPHA; // GL_ONE_MINUS_SRC_ALPHA
	blend.mode = GL_FUNC_ADD;
	
	textures.active = 0;
	for (GLuint i = 0; i < GLActiveTextureLimit; i++) {
		textures.attached[i] = NULL;
	}
	
	frameBuffer.attached = NULL;
}

void GLContextState::restoreDefault() {
	glDisable(GL_CULL_FACE);

	scissor.enabled = GL_FALSE;
	scissor.rect.set(0, 0, 0, 0);
	glDisable(GL_SCISSOR_TEST);
	
	depth.enabled = GL_FALSE;
	depth.depth = 1.0;
	depth.zNear = 0.0;
	depth.zFar = 1.0;
	depth.func = GL_LEQUAL;
	glDisable(GL_DEPTH_TEST);
	glClearDepthf(depth.depth);
	glDepthRangef(depth.zNear, depth.zFar);
	glDepthFunc(depth.func);
	
	stencil.enabled = GL_FALSE;
	stencil.func = GL_ALWAYS;
	stencil.ref = 1;
	stencil.mask = 1;
	stencil.fail = GL_KEEP;
	stencil.zFail = GL_KEEP;
	stencil.zPass = GL_REPLACE;
	stencil.clear = 0;
	glDisable(GL_STENCIL_TEST);
	glStencilFunc(stencil.func, stencil.ref, stencil.mask);
	glStencilOp(stencil.fail, stencil.zFail, stencil.zPass);
	glClearStencil(stencil.clear);
	
	colorMask.red = colorMask.green = colorMask.blue = colorMask.alpha = GL_TRUE;
	glColorMask(colorMask.red, colorMask.green, colorMask.blue, colorMask.alpha);
	
	blend.enabled = GL_FALSE;
	blend.sFactor = GL_ONE;
	blend.dFactor = GL_ONE_MINUS_SRC_ALPHA;
	blend.mode = GL_FUNC_ADD;
	glDisable(GL_BLEND);
	glBlendFunc(blend.sFactor, blend.dFactor);
	glBlendEquation(blend.mode);
	
	textures.active = 0;
	for (int i = GLActiveTextureLimit - 1; i >= 0; i--) {
		glActiveTexture(GLTextureIndexes[i]);
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
		textures.attached[i] = NULL;
	}
}

void GLContextState::setFrom(const GLContextState& restore) {
	setFrom(restore.frameBuffer);
	setFrom(restore.viewport);
	setFrom(restore.scissor);
	setFrom(restore.depth);
	setFrom(restore.stencil);
	setFrom(restore.colorMask);
	setFrom(restore.blend);
	setFrom(restore.textures);
}

void GLContextState::setFrom(const GLContextStateViewport& restore) {
	setViewport(restore.rect.left, restore.rect.top, restore.rect.right, restore.rect.bottom);
}

void GLContextState::setFrom(const GLContextStateScissor& restore) {
	if (restore.enabled == GL_TRUE) { enableScissor(); } else { disableScissor(); }
	setScissor(restore.rect.left, restore.rect.top, restore.rect.right, restore.rect.bottom);
}

void GLContextState::setFrom(const GLContextStateDepth& restore) {
	if (restore.enabled == GL_TRUE) { enableDepth(); } else { disableDepth(); }
	setDepth(restore.depth, restore.zNear, restore.zFar, restore.func);
}

void GLContextState::setFrom(const GLContextStateStencil& restore) {
	if (restore.enabled == GL_TRUE) { enableStencil(); } else { disableStencil(); }
	setStencil(restore.func, restore.ref, restore.mask, restore.fail, restore.zFail, restore.zPass);
	setClearStencil(restore.clear);
}

void GLContextState::setFrom(const GLContextStateColorMask& restore) {
	setColorMask(restore.red, restore.green, restore.blue, restore.alpha);
}

void GLContextState::setFrom(const GLContextStateBlend& restore) {
	if (restore.enabled == GL_TRUE) { enableBlend(); } else { disableBlend(); }
	setBlend(restore.sFactor, restore.dFactor, restore.mode);
}

void GLContextState::setFrom(const GLContextStateTexture& restore) {
	for (int index = GLActiveTextureLimit - 1; index >= 0; index--) {
		GLTexture* target = restore.attached[index];

		if (textures.attached[index] != target) {
			if (textures.active != index) {
				textures.active = index;
				glActiveTexture(GLTextureIndexes[index]);
			}
			
			if (target == NULL) {
				glBindTexture(GL_TEXTURE_2D, GL_NONE);
			} else {
				glBindTexture(GL_TEXTURE_2D, target->handle);
			}
			
			textures.attached[index] = target;
		}
	}

	if (textures.active != restore.active) {
		textures.active = restore.active;
		glActiveTexture(GLTextureIndexes[restore.active]);
	}
}

void GLContextState::setFrom(const GLContextStateFrameBuffer& restore) {
	if (frameBuffer.attached != restore.attached) {
		if ((frameBuffer.attached != NULL) && (restore.attached != NULL)) {
			glBindFramebuffer(GL_FRAMEBUFFER, restore.attached->frameBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, restore.attached->renderBuffer);
		}

		frameBuffer.attached = restore.attached;
	}
}

void GLContextState::setViewport(GLint left, GLint top, GLint right, GLint bottom) {
	if ((viewport.rect.left != left) || (viewport.rect.top != top) || (viewport.rect.right != right) || (viewport.rect.bottom != bottom)) {
		viewport.rect.set(left, top, right, bottom);
		glViewport(left, top, right - left, bottom - top);
	}
}

void GLContextState::enableScissor() {
	if (scissor.enabled == GL_FALSE) {
		scissor.enabled = GL_TRUE;
		glEnable(GL_SCISSOR_TEST);
	}
}

void GLContextState::disableScissor() {
	if (scissor.enabled == GL_TRUE) {
		scissor.enabled = GL_FALSE;
		glDisable(GL_SCISSOR_TEST);
	}
}

void GLContextState::setScissor(GLint left, GLint top, GLint right, GLint bottom) {
	if ((scissor.rect.left != left) || (scissor.rect.top != top) || (scissor.rect.right != right) || (scissor.rect.bottom != bottom)) {
		scissor.rect.set(left, top, right, bottom);
		glScissor(left, top, right - left, bottom - top);
	}
}

void GLContextState::enableDepth() {
	if (depth.enabled == GL_FALSE) {
		depth.enabled = GL_TRUE;
		glEnable(GL_DEPTH_TEST);
	}
}

void GLContextState::disableDepth() {
	if (depth.enabled == GL_TRUE) {
		depth.enabled = GL_FALSE;
		glDisable(GL_DEPTH_TEST);
	}
}

void GLContextState::setDepth(GLclampf depth, GLclampf zNear, GLclampf zFar, GLenum func) {
	if ((this->depth.depth != depth) || (this->depth.zNear != zNear) || (this->depth.zFar != zFar) || (this->depth.func != func)) {
		this->depth.depth = depth;
		this->depth.zNear = zNear;
		this->depth.zFar = zFar;
		this->depth.func = func;
		glClearDepthf(depth);
		glDepthRangef(zNear, zFar);
		glDepthFunc(func);
	}
}

void GLContextState::enableStencil() {
	if (stencil.enabled == GL_FALSE) {
		stencil.enabled = GL_TRUE;
		glEnable(GL_STENCIL_TEST);
	}
}

void GLContextState::disableStencil() {
	if (stencil.enabled == GL_TRUE) {
		stencil.enabled = GL_FALSE;
		glDisable(GL_STENCIL_TEST);
	}
}

void GLContextState::setStencil(GLenum func, GLint ref, GLuint mask, GLenum fail, GLenum zFail, GLenum zPass) {
	if ((stencil.func != func) || (stencil.ref != ref) || (stencil.mask != mask) || (stencil.fail != fail) || (stencil.zFail != zFail) || (stencil.zPass != zPass)) {
		stencil.func = func;
		stencil.ref = ref;
		stencil.mask = mask;
		stencil.fail = fail;
		stencil.zFail = zFail;
		stencil.zPass = zPass;
		glStencilFunc(func, ref, mask);
		glStencilOp(fail, zFail, zPass);
	}
}

void GLContextState::setClearStencil(GLint s) {
	if (stencil.clear != s) {
		stencil.clear = s;
		glClearStencil(s);
	}
}

void GLContextState::setColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
	if ((colorMask.red != red) || (colorMask.green != green) || (colorMask.blue != blue) || (colorMask.alpha != alpha)) {
		colorMask.red = red;
		colorMask.green = green;
		colorMask.blue = blue;
		colorMask.alpha = alpha;
		glColorMask(red, green, blue, alpha);
	}
}

void GLContextState::enableBlend() {
	if (blend.enabled == GL_FALSE) {
		blend.enabled = GL_TRUE;
		glEnable(GL_BLEND);
	}
}

void GLContextState::disableBlend() {
	if (blend.enabled == GL_TRUE) {
		blend.enabled = GL_FALSE;
		glDisable(GL_BLEND);
	}
}

void GLContextState::setBlend(GLenum sFactor, GLenum dFactor, GLenum mode) {
	if ((blend.sFactor != sFactor) || (blend.dFactor != dFactor) || (blend.mode != mode)) {
		blend.sFactor = sFactor;
		blend.dFactor = dFactor;
		blend.mode = mode;
		glBlendFunc(sFactor, dFactor);
		glBlendEquation(mode);
	}
}

GLuint GLContextState::attachTexture(GLTexture& texture, GLuint index, GLint uniform) throw(const char*) {
	if (texture.handle == GL_NONE) {
		throw eOpenGL;
	}
	
	if (index >= GLActiveTextureLimit) {
		index = 0;
	}
	
	GLTexture* target = textures.attached[index];
	uint32_t indexBit;
	
	if (target == &texture) {
		goto attachTexture_setUniform;
	}
	
	indexBit = (uint32_t)(1 << index);
	
	if (target != NULL) {
		target->attached &= ~indexBit;
	}
	
	if (textures.active != index) {
		textures.active = index;
		glActiveTexture(GLTextureIndexes[index]);
	}
	
	glBindTexture(GL_TEXTURE_2D, texture.handle);
	
	textures.attached[index] = &texture;
	texture.attached |= indexBit;
	
attachTexture_setUniform:
	if (uniform != -1) {
		glUniform1i(uniform, index);
	}
	
	return index;
}

void GLContextState::detachTexture(GLTexture& texture) throw(const char*) {
	if (texture.handle == GL_NONE) {
		throw eOpenGL;
	}
	
	uint32_t bits = texture.attached;
	GLuint index = 0;
	while (bits != 0) {
		if ((bits & 1) != 0) {
			detachTextureIndex(index);
		}

		index++;
		bits >>= 1;
	}
	
	texture.attached = 0;
}

bool GLContextState::reattachTexture(GLTexture& texture) throw(const char*) {
	if (texture.handle == GL_NONE) {
		throw eOpenGL;
	}
	
	uint32_t bits = texture.attached;
	if (bits != 0) {
		GLuint index = 0;
		while (bits != 0) {
			if ((bits & 1) != 0) {
				if (textures.active != index) {
					textures.active = index;
					glActiveTexture(GLTextureIndexes[index]);
				}
			
				glBindTexture(GL_TEXTURE_2D, texture.handle);
			}
			
			index++;
			bits >>= 1;
		}
		
		return true;
	}
	
	return false;
}

GLuint GLContextState::attachTemporaryTexture(GLuint handle) {
	glBindTexture(GL_TEXTURE_2D, handle);
	
	if (textures.attached[textures.active] != NULL) {
		return textures.attached[textures.active]->handle;
	}
	
	return GL_NONE;
}

void GLContextState::detachTextureIndex(GLuint index) {
	if (index >= GLActiveTextureLimit) {
		index = 0;
	}
	
	if (textures.attached[index] != NULL) {
		textures.attached[index] = NULL;
		
		if (textures.active != index) {
			textures.active = index;
			glActiveTexture(GLTextureIndexes[index]);
		}
		
		glBindTexture(GL_TEXTURE_2D, GL_NONE);
	}
}
