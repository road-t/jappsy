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

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uHashMap.h>
#include <data/uVector.h>

class GLRender;

class RefGLTexture : public RefObject {
public:
	GLRender* context = NULL;
	GLuint* handles = NULL;
	GLint width = 0;
	GLint height = 0;
	GLint* handles1iv = NULL;
	GLuint count = 0;
	
	inline RefGLTexture() { throw eInvalidParams; }
	RefGLTexture(GLRender* context, GLuint* handles, GLuint count, GLint width, GLint height);
	~RefGLTexture();
	
	GLuint bind(GLint index, GLint uniform = -1);
};

class GLTexture : public Object {
public:
	constexpr static int SMOOTH = 1;
	constexpr static int REPEAT = 2;
	
	RefClass(GLTexture, RefGLTexture)
};

class GLTextures {
private:
	GLRender* context;
	HashMap<String, GLTexture> list;
	GLTexture defaultTexture;
	
public:
	GLTextures(GLRender* context) throw(const char*);
	~GLTextures();
	
	GLTexture& get(const wchar_t* key) throw(const char*);
	GLTexture& createSolidTexture(const wchar_t* key, const Vec4& rgba4fv) throw(const char*);
	GLTexture& createTexture(const wchar_t* key, GLuint* handles, GLuint count, GLint width, GLint height) throw(const char*);
	
	GLuint createTextureHandle(GLint width, GLint height, int style, void* data) throw(const char*);
	void releaseTextureHandle(GLuint handle);
};

#endif //JAPPSY_UGLTEXTURE_H