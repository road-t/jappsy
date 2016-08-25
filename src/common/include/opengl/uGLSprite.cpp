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

#include "uGLSprite.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>
#include <core/uSystem.h>
#include <math.h>

static void pushSquare(Vector<GLfloat>& v, GLfloat x, GLfloat y, GLfloat w, GLfloat h) {
	v.push(x);
	v.push(y);
	v.push(x);
	v.push(y+h);
	v.push(x+w);
	v.push(y+h);
	v.push(x+w);
	v.push(y);
}

static void pushSquareIndex(Vector<GLshort>& v, GLshort i) {
	v.push(i);
	v.push(i+1);
	v.push(i+2);
	v.push(i);
	v.push(i+2);
	v.push(i+3);
}

GLSprite::GLSprite(GLRender* context, const CString& key, const CString& textureKey, const Vec2& size, const GLuint frames, const Vec2* first, const Vec2* next) throw(const char*) {
	this->context = context;
	this->key = key;
	
	texture = context->textures->get(textureKey);
	width = size.x;
	height = size.y;
	this->frames = frames;
	
	GLfloat sw = width;
	GLfloat sh = height;
	GLfloat ox, oy, tw, th;
	if (first != NULL) {
		GLfloat w = texture->width;
		GLfloat h = texture->height;
		ox = first->x / w;
		oy = first->y / h;
		tw = sw / w;
		th = sh / h;
	} else {
		ox = oy = 0;
		tw = th = 1;
	}

	GLfloat vec[8] = {0};
	GLfloat tex[8] = {0};
	GLshort ind[6] = {0};
	Vector<GLfloat> vertices; vertices.growstep(8);
	Vector<GLfloat> textures; textures.growstep(8);
	Vector<GLshort> indexes; indexes.growstep(6);
	
	GLfloat tx = ox;
	GLfloat ty = oy;
	for (int i = 0; i < frames; i++) {
		pushSquare(vertices, 0.0, 0.0, sw, sh);
		pushSquare(textures, tx, ty, tw, th);
		pushSquareIndex(indexes, i * 4);
		
		if (next != NULL) {
			tx += tw * next->x;
			ty += th * next->y;
		} else {
			ty += th;
			if ((ty + th) > 1.0) {
				ty = oy;
				tx += tw;
			}
		}
	}
	
	try {
		glGenBuffers(1, &vertexBuffer);
		CheckGLError();
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.count() * sizeof(GLfloat), vertices.items(), GL_STATIC_DRAW);
		CheckGLError();
		
		glGenBuffers(1, &textureBuffer);
		CheckGLError();
		glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
		glBufferData(GL_ARRAY_BUFFER, textures.count() * sizeof(GLfloat), textures.items(), GL_STATIC_DRAW);
		CheckGLError();
	
		glGenBuffers(1, &indexBuffer);
		CheckGLError();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.count() * sizeof(GLshort), indexes.items(), GL_STATIC_DRAW);
		CheckGLError();
	} catch (...) {
		if (vertexBuffer != NULL) {
			glDeleteBuffers(1, &vertexBuffer);
		}
		if (textureBuffer != NULL) {
			glDeleteBuffers(1, &textureBuffer);
		}
		if (indexBuffer != NULL) {
			glDeleteBuffers(1, &indexBuffer);
		}
		throw;
	}
}

GLSprite::~GLSprite() {
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &vertexBuffer);
	
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &textureBuffer);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &indexBuffer);
}

Vec2 GLSprite::getPosition(const Vec2& position, const GLPaint* paint) {
	GLfloat x = position.x;
	GLfloat y = position.y;
	if (paint != NULL) {
		if (paint->m_alignX == GLAlignX::CENTER) {
			x -= (GLfloat)width / 2.0;
		} else if (paint->m_alignX > GLAlignX::CENTER) {
			x -= width;
		}
		if (paint->m_alignY == GLAlignY::MIDDLE) {
			x -= (GLfloat)height / 2.0;
		} else if (paint->m_alignY > GLAlignY::MIDDLE) {
			x -= height;
		}
	}
	
	return {x, y};
}

void GLSprite::render(const Vec2& position, const GLuint frame, const GLPaint* paint, const GLfloat time) {
	GLSpriteShader* shader = context->shaderSprite;
	GLCamera* cam = context->cameras->gui;
	cam->update();
	
	glEnable(GL_BLEND);
	glUseProgram(shader->program);
	
	if (paint != NULL) {
		glUniform4fv(shader->uLight, 1, paint->m_color4iv);
	} else {
		glUniform4fv(shader->uLight, 1, context->light.v);
	}
	
	if (!isnan(time)) {
		glUniform1f(shader->uTime, time);
	} else {
		GLfloat time = (GLfloat)(currentTimeMillis()) / 2000.0;
		glUniform1f(shader->uTime, time - floorf(time));
	}
	
	Vec2 pos = getPosition(position, paint);
	
	glUniformMatrix4fv(shader->uLayerProjectionMatrix, 1, GL_FALSE, cam->projection16fv);
	glUniform2fv(shader->uPosition, 1, pos.v);
	
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(shader->aVertexPosition, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(shader->aVertexPosition);
	
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glVertexAttribPointer(shader->aTextureCoord, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(shader->aTextureCoord);
	
	texture->bind(0, shader->uTexture);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (GLvoid*)(6 * frame * sizeof(GLshort)));
	
	glDisableVertexAttribArray(shader->aVertexPosition);
	glDisableVertexAttribArray(shader->aTextureCoord);
	
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLSprite::renderNumber(const Vec2& position, GLfloat step, const CString& value) {
	wchar_t* s = (wchar_t*)value;
	int p = value.m_length - 1;
	Vec2 pos = position;
	if (step > 0) {
		pos[0] += step * p;
	} else {
		pos[0] += step * 2.0;
		step = -step;
	}
	for (; p >= 0; p--) {
		int v = (int)(s[p]) - 48;
		render(pos, v);
		pos.x -= step;
	}
}

GLSprites::GLSprites(GLRender* context) throw(const char*) {
	this->context = context;
}

GLSprites::~GLSprites() {
	int32_t count = list.count();
	GLSprite** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLSprite* GLSprites::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLSprite* GLSprites::createSprite(const CString& key, const CString& textureKey, const Vec2& size, const GLuint frames, const Vec2* first, const Vec2* next) throw(const char*) {
	try {
		list.removedelete(key);
		GLSprite* sprite = new GLSprite(context, key, textureKey, size, frames, first, next);
		try {
			list.put(key, sprite);
		} catch (...) {
			delete sprite;
			throw;
		}
		return sprite;
	} catch (...) {
		throw;
	}
}

void GLSprites::renderSpriteNumber(const CString& key, const Vec2& position, GLfloat step, const CString& value) {
	list.get(key)->renderNumber(position, step, value);
}
