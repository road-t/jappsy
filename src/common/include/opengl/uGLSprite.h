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

#ifndef JAPPSY_UGLSPRITE_H
#define JAPPSY_UGLSPRITE_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uVector.h>
#include <opengl/uGLTextureSet.h>
#include <opengl/uGLPaint.h>
#include <opengl/uGLCamera.h>

class GLRender;

class GLSprite : public CObject {
public:
	GLRender* context = NULL;
	GLTextureSet* texture = NULL;
	CString key;
	GLint width = 0;
	GLint height = 0;
	GLuint frames = 0;
	
	GLuint vertexBuffer = 0;
	GLuint textureBuffer = 0;
	GLuint indexBuffer = 0;
	
	GLSprite(GLRender* context, const CString& key, const CString& textureKey, const Vec2& size, const GLuint frames = 1, const Vec2* first = NULL, const Vec2* next = NULL) throw(const char*);
	~GLSprite();
	
	Vec2 getPosition(const Vec2& position, const GLPaint* paint = NULL);
	
	void render(const Vec2& position, const GLuint frame, const GLPaint* paint = NULL, const GLfloat time = NAN, GLCamera* camera = NULL);
	void renderNumber(const Vec2& position, GLfloat step, const CString& value);
};

class GLSprites : public CObject {
private:
	GLRender* context;
	VectorMap<CString&, GLSprite*> list;
	
public:
	GLSprites(GLRender* context) throw(const char*);
	~GLSprites();
	
	GLSprite* get(const CString& key) throw(const char*);
	GLSprite* createSprite(const CString& key, const CString& textureKey, const Vec2& size, const GLuint frames = 1, const Vec2* first = NULL, const Vec2* next = NULL) throw(const char*);
	
	inline void renderSprite(const CString& key, const Vec2& position, const GLuint frame, const GLPaint* paint = NULL, const GLfloat time = NAN, GLCamera* camera = NULL) { list.get(key)->render(position, frame, paint, time, camera); }
	
	void renderSpriteNumber(const CString& key, const Vec2& position, GLfloat step, const CString& value);
};

#endif //JAPPSY_UGLSPRITE_H
