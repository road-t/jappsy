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

#ifndef JAPPSY_UGLTYPES_H
#define JAPPSY_UGLTYPES_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uVector.h>

#define GLRepeatNone			0x00000000
#define GLRepeatX				0x00000001
#define GLRepeatMirroredX		0x00000003
#define GLRepeatY				0x00000004
#define GLRepeatMirroredY		0x0000000C
#define GLRepeat				(GLRepeatX | GLRepeatY)
#define GLRepeatMirrored		(GLRepeatMirroredX | GLRepeatMirroredY)
#define GLRepeatMask			0x0000000F

#define GLSmoothNone			0x00000000
#define GLSmoothMin				0x00000010
#define GLSmoothMag				0x00000020
#define GLSmooth				(GLSmoothMin | GLSmoothMag)

#define GLDirty					0x01000000		// Dirty Content
#define GLDirtyResize			0x02000000		// Dirty Content On Resize
#define GLTextureInvalid		0x04000000		// Invalid Texture State

#define GLAttachmentColor		0x00010000		// This flag means its frame buffer
#define GLAttachmentDepth		0x00020000
#define GLAttachmentStencil		0x00040000

#define GLFrameBufferInvalid	0x10000000		// Invalid Framebuffer State

#define GLFrameBufferRestore	0x40000000		// Restore Default State
#define GLFrameBufferGrabbed	0x80000000

#define GLActiveTextureLimit	8

struct GLSize {
	GLint width;
	GLint height;
	
	inline GLSize() {}
	
	inline GLSize(GLint width, GLint height) {
		this->width = width; this->height = height;
	}
	
	inline GLSize(const GLSize& size) {
		width = size.width; height = size.height;
	}
	
	inline void set(GLint width, GLint height) {
		this->width = width; this->height = height;
	}
	
	inline GLSize& operator = (const GLSize& rhs) {
		width = rhs.width; height = rhs.height;
		return *this;
	}
};

struct GLRect {
	GLint left;
	GLint top;
	GLint right;
	GLint bottom;
	
	inline GLRect() {}
	
	inline GLRect(GLint left, GLint top, GLint right, GLint bottom) {
		this->left = left; this->top = top; this->right = right; this->bottom = bottom;
	}
	
	inline GLRect(const GLRect& rect) {
		left = rect.left; right = rect.right; top = rect.top; bottom = rect.bottom;
	}
	
	inline void set(GLint left, GLint top, GLint right, GLint bottom) {
		this->left = left; this->top = top; this->right = right; this->bottom = bottom;
	}
	
	//GLRect& operator =(GLRect rhs);
	GLRect& operator =(const GLRect& rhs);
	
	GLRect& operator |=(const GLRect& rhs);
	friend GLRect operator |(GLRect lhs, const GLRect& rhs);
	
	GLRect& operator &=(const GLRect& rhs);
	friend GLRect operator &(GLRect lhs, const GLRect& rhs);
	
	inline bool isEmpty() {
		return ((left >= right) || (top >= bottom));
	}
};

typedef void (*onUpdateRect)(const GLRect& updateRect, void* userData);

#endif //JAPPSY_UGLTYPES_H
