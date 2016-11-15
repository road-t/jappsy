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

#ifndef JAPPSY_UGLTEXTURESET_H
#define JAPPSY_UGLTEXTURESET_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uVector.h>

class GLRender;

class GLTextureSet : public CObject {
public:
	constexpr static int NONE = 0;
	constexpr static int SMOOTH = 1;
	constexpr static int REPEAT = 2;

	GLRender* context = NULL;
	Vector<GLuint> handles;
	GLint width = 0;
	GLint height = 0;
	Vector<GLint> handles1iv;
	
	GLTextureSet(GLRender* context, Vector<GLuint>& handles, GLint width, GLint height) throw(const char*);
	~GLTextureSet();
	
	GLuint bind(GLuint index, GLint uniform = -1);
};

class GLTextures : public CObject {
private:
	GLRender* context = NULL;
	VectorMap<CString&, GLTextureSet*> list;
	GLTextureSet* defaultTexture;

	//void remove(const CString& key);
public:
	GLuint defaultTextureHandle = 0;

	GLTextures(GLRender* context) throw(const char*);
	~GLTextures();
	
	GLTextureSet* get(const CString& key) throw(const char*);
	GLTextureSet* createSolidTexture(const CString& key, const Vec4& rgba4fv) throw(const char*);
	GLTextureSet* createTexture(const CString& key, Vector<GLuint>& handles, GLint width, GLint height) throw(const char*);
	
	GLuint createTextureHandle(GLint width, GLint height, int style, void* data) throw(const char*);
	void releaseTextureHandle(GLuint handle);
};

#endif //JAPPSY_UGLTEXTURESET_H
