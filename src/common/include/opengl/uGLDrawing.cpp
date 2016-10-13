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

#include "uGLDrawing.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>

GLDrawing::GLDrawing(GLRender* context, const CString& key, const CString& spriteKey, const Vec2& position, const Vector<GLshort>* frameIndexes, const GLPaint* paint) throw(const char*) {
	this->context = context;
	this->key = key;
	this->sprite = context->sprites->get(spriteKey);
	if (paint != NULL) {
		this->paint = new GLPaint(*paint);
	} else {
		this->paint = new GLPaint();
		this->paint->setColor(0xFFFFFFFF).setAlignX(GLAlignX::LEFT).setAlignY(GLAlignY::TOP);
	}
	this->position.v[0] = position.v[0];
	this->position.v[1] = position.v[1];
	
	if (frameIndexes != NULL) {
		int32_t count = frameIndexes->count();
		GLshort* items = frameIndexes->items();
		if (count == 0) {
			background = 0;
		} else if (count == 1) {
			background = items[0];
		} else if (count > 2) {
			normal = items[0];
			hover = items[1];
			click = items[2];
			if (count > 3) {
				foreground = items[3];
			}
		} else {
			throw eInvalidParams;
		}
	} else {
		background = 0;
	}
}

GLDrawing::~GLDrawing() {
	context = NULL;
	if (animation != NULL) {
		delete animation;
	}
}

void GLDrawing::setPaint(const GLPaint& paint) throw(const char*) {
	if (this->paint != NULL) {
		delete this->paint;
	}
	this->paint = new GLPaint(paint);
}

void GLDrawing::render(GLObject* object, const GLfloat time, GLCamera* camera) {
	if (animation != NULL)
		animation->update();
	
	if (background >= 0)
		sprite->render(position, (GLuint)background, paint, time, camera);
	if (pressed) {
		if (click >= 0)
			sprite->render(position, (GLuint)click, paint, time, camera);
		else if (normal >= 0)
			sprite->render(position, (GLuint)normal, paint, time, camera);
	} else if (hovered) {
		if (hover >= 0)
			sprite->render(position, (GLuint)hover, paint, time, camera);
		else if (normal >= 0)
			sprite->render(position, (GLuint)normal, paint, time, camera);
	} else if (normal >= 0) {
		sprite->render(position, (GLuint)normal, paint, time, camera);
	}
	
	if (onrender != NULL)
		onrender(context->engine, this);
	
	if (foreground >= 0)
		sprite->render(position, (GLuint)foreground, paint, time, camera);
}

GLDrawings::GLDrawings(GLRender* context) throw(const char*) {
	this->context = context;
}

GLDrawings::~GLDrawings() {
	int32_t count = list.count();
	GLDrawing** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLDrawing* GLDrawings::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLDrawing* GLDrawings::createDrawing(const CString& key, const CString& spriteKey, const Vec2& position, const Vector<GLshort>* frameIndexes, const GLPaint* paint) throw(const char*) {
	try {
		list.removedelete(key);
		GLDrawing* drawing = new GLDrawing(context, key, spriteKey, position, frameIndexes, paint);
		try {
			list.put(key, drawing);
		} catch (...) {
			delete drawing;
			throw;
		}
		return drawing;
	} catch (...) {
		throw;
	}
}

void GLDrawings::renderDrawing(const CString& key, GLfloat time, GLCamera* camera) {
	GLDrawing* drawing = list.get(key);
	drawing->render(NULL, time, camera);
}
