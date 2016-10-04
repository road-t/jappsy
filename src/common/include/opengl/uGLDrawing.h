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

#ifndef JAPPSY_UGLDRAWING_H
#define JAPPSY_UGLDRAWING_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uVector.h>
#include <opengl/uGLAnimation.h>
#include <opengl/uGLPaint.h>
#include <opengl/uGLSprite.h>
#include <opengl/uGLCamera.h>

class GLRender;
class GLEngine;

class GLDrawing : public GLAnimationTarget {
public:
	GLRender* context = NULL;
	CString key;
	GLSprite* sprite = NULL;
	Vec2 position;
	
	int background = -1;
	int normal = -1;
	int hover = -1;
	int click = -1;
	int foreground = -1;
	
	jbool hovered = false;
	jbool pressed = false;
	
	typedef void (*onRenderCallback)(GLEngine* engine, GLDrawing* drawing);
	
	onRenderCallback onrender = NULL;
	
	GLDrawing(GLRender* context, const CString& key, const CString& spriteKey, const Vec2& position, const Vector<GLshort>* frameIndexes = NULL, const GLPaint* paint = NULL) throw(const char*);
	~GLDrawing();
	
	void setPaint(const GLPaint& paint) throw(const char*);
	
	void render(GLObject* object = NULL, const GLfloat time = NAN, GLCamera* camera = NULL);
};

class GLDrawings : public CObject {
private:
	GLRender* context = NULL;
	VectorMap<CString&, GLDrawing*> list;
	
public:
	GLDrawings(GLRender* context) throw(const char*);
	~GLDrawings();
	
	GLDrawing* get(const CString& key) throw(const char*);
	GLDrawing* createDrawing(const CString& key, const CString& spriteKey, const Vec2& position, const Vector<GLshort>* frameIndexes = NULL, const GLPaint* paint = NULL) throw(const char*);
	void renderDrawing(const CString& key, GLfloat time = NAN, GLCamera* camera = NULL);
};

#endif //JAPPSY_UGLDRAWING_H
