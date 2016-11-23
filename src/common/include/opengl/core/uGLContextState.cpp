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
}

void GLContextState::setSize(GLuint width, GLuint height) {
	size.x = width;
	size.y = height;
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
	
	glActiveTexture(GL_TEXTURE0);
}

void GLContextState::setFrom(const GLContextState& restore) {
	setFrom(restore.viewport);
	setFrom(restore.scissor);
	setFrom(restore.depth);
	setFrom(restore.stencil);
	setFrom(restore.colorMask);
	setFrom(restore.blend);
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

void GLContextState::setViewport(GLint left, GLint top, GLint right, GLint bottom) {
	if ((viewport.rect.left != left) || (viewport.rect.top != top) || (viewport.rect.right != right) || (viewport.rect.bottom != bottom)) {
		viewport.rect.set(left, top, right, bottom);
		glViewport(left, size.y - bottom, right - left, bottom - top);
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
		glScissor(left, size.y - bottom, right - left, bottom - top);
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
