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

#ifndef JAPPSY_UGLPAINT_H
#define JAPPSY_UGLPAINT_H

#include <platform.h>
#include <opengl/uOpenGL.h>
#include <opengl/uGLTypeface.h>
#include <opengl/uGLAlignX.h>
#include <opengl/uGLAlignY.h>
#include <data/uVector.h>
#include <data/uObject.h>

class GLPaint : public CObject {
public:
	GLfloat m_scale;
	bool m_antialias;
	GLTypeface m_typeface;
	GLAlignX m_alignX;
	GLAlignY m_alignY;
	GLfloat m_size;
	
	bool m_invisible;
	uint32_t m_color;
	Vec4 m_color4iv;
	
	bool m_strokeInvisible;
	GLfloat m_strokeOffset;
	GLfloat m_strokeWidth;
	uint32_t m_strokeColor;
	
	bool m_shadowInvisible;
	GLfloat m_shadow[2];
	GLfloat m_shadowWidth = 0;
	uint32_t m_shadowColor;
	
	bool m_sdfInvisible;
	uint32_t m_sdfColors[4];

	GLPaint();
	GLPaint(const GLPaint& paint);
	
	GLPaint& setSDFColors(const uint32_t* colors, uint32_t count);
	GLPaint& setAntialias(bool antialias);
	GLPaint& setTypeface(const GLTypeface& typeface);
	GLPaint& setSize(GLfloat size);
	GLPaint& setAlignX(GLAlignX align);
	GLPaint& setAlignY(GLAlignY align);
	GLPaint& setColor(uint32_t color);
	GLPaint& setLight(GLfloat light, GLfloat alpha);
	GLPaint& setStroke(GLfloat width, uint32_t color);
	GLPaint& setAlpha(uint8_t alpha);
	GLPaint& setShadow(GLfloat width, GLfloat dx, GLfloat dy, uint32_t color);
	GLPaint& setScale(GLfloat scale);
};

#endif //JAPPSY_UGLPAINT_H