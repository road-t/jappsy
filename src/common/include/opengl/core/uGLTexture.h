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

#ifndef JAPPSY_UGLTEXTURE_H
#define JAPPSY_UGLTEXTURE_H

#include <opengl/core/uGLTypes.h>

struct GLContextState;
class GLContext;

class GLTexture : public CObject {
	friend struct GLContextState;
	friend class GLProgramTexture;
	
protected:
	GLContext* context = NULL;
	uint32_t state = 0;
	GLuint handle;
	GLuint rectArrayBuffer;
	
	GLuint width;
	GLuint height;
	GLuint bufferWidth;
	GLuint bufferHeight;
	GLRect dirtyRect;
	
protected:
	// Create OpenGL Empty Texture
	GLTexture(GLContext& context);
	
public:
	// Create OpenGL Texture
	GLTexture(GLContext& context, uint32_t rgba) throw(const char*);
	GLTexture(GLContext& context, const Vec4& rgba) throw(const char*);
	GLTexture(GLContext& context, GLint width, GLint height, uint32_t mode = (GLRepeatNone | GLSmooth), void* data = NULL) throw(const char*);
	~GLTexture();
	
protected:
	void resize(GLint width, GLint height) throw(const char*);
	void release();
	
public:
	void setMode(uint32_t mode) throw(const char*);
private:
	void validate();

protected:
	onRectCallback updateRect = NULL;
	void* updateRectUserData = NULL;
	
public:
	void setOnUpdateRectCallback(onRectCallback callback, void* userData = NULL);
	
	void dirty();
	void dirty(GLint left, GLint top, GLint right, GLint bottom);
	
	void update();
	
	void attachTexture(GLuint index);
	static void detachTexture(GLuint index);
};


#endif //JAPPSY_UGLTEXTURE_H
