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
	
protected:
	GLContext* context = NULL;
	uint32_t state = 0;
	GLuint handle;
	uint32_t attached;
	GLint width;
	GLint height;
	GLRect dirtyRect;
	
protected:
	// Create OpenGL Empty Texture
	GLTexture(GLContext& context);
	
public:
	// Create OpenGL Texture
	GLTexture(GLContext& context, uint32_t rgba) throw(const char*);
	GLTexture(GLContext& context, const Vec4& rgba) throw(const char*);
	GLTexture(GLContext& context, uint32_t width, uint32_t height, uint32_t mode = (GLRepeat | GLSmooth), void* data = NULL) throw(const char*);
	~GLTexture();
	
protected:
	void resize(uint32_t width, uint32_t height) throw(const char*);
	void destroy();
	
public:
	void setMode(uint32_t mode) throw(const char*);
private:
	void validate();

private:
	onUpdateRect updateRect = NULL;
	void* updateRectUserData = NULL;
	
public:
	void setOnUpdateRectCallback(onUpdateRect callback, void* userData = NULL);
	
	void dirty();
	void dirty(GLint left, GLint top, GLint right, GLint bottom);
	
	
};


#endif //JAPPSY_UGLTEXTURE_H
