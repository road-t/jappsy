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

#include "uGLPaint.h"
#include <math.h>

static GLfloat defaultColor[4] = { 0, 0, 0, 1 };

GLPaint::GLPaint() {
	m_scale = 1.0;
	m_antialias = false;
	m_typeface = GLTypeface::DEFAULT;
	m_alignX = GLAlignX::LEFT;
	m_alignY = GLAlignY::BASELINE;
	m_size = 16.0;
	
	m_invisible = false;
	m_color = 0xFF000000;
	Vec4SetV(m_color4iv.v, defaultColor);
	
	m_strokeInvisible = true;
	m_strokeOffset = 0;
	m_strokeWidth = 0;
	m_strokeColor = 0x00000000;
	
	m_shadowInvisible = true;
	m_shadow[0] = 0; m_shadow[1] = 0;
	m_shadowWidth = 0;
	m_shadowColor = 0x00000000;
	
	m_sdfInvisible = true;
	for (int i = 0; i < 4; i++)
		m_sdfColors[i] = 0x00000000;
}

GLPaint::GLPaint(const GLPaint& paint) {
	m_scale = paint.m_scale;
	m_antialias = paint.m_antialias;
	m_typeface = paint.m_typeface;
	m_alignX = paint.m_alignX;
	m_alignY = paint.m_alignY;
	m_size = paint.m_size;
	
	m_invisible = paint.m_invisible;
	m_color = paint.m_color;
	m_color4iv.set(paint.m_color4iv);
	
	m_strokeInvisible = paint.m_strokeInvisible;
	m_strokeOffset = paint.m_strokeOffset;
	m_strokeWidth = paint.m_strokeWidth;
	m_strokeColor = paint.m_strokeColor;
	
	m_shadowInvisible = paint.m_shadowInvisible;
	m_shadow[0] = paint.m_shadow[0];
	m_shadow[1] = paint.m_shadow[1];
	m_shadowWidth = paint.m_shadowWidth;
	m_shadowColor = paint.m_shadowColor;
	
	m_sdfInvisible = paint.m_sdfInvisible;
	for (int i = 0; i < 4; i++)
		m_sdfColors[i] = paint.m_sdfColors[i];
}

GLPaint& GLPaint::setSDFColors(const uint32_t* colors, uint32_t count) {
	for (uint32_t i = 0; i < count; i++)
		m_sdfColors[i] = colors[i];
	for (uint32_t i = count; i < 4; i++)
		m_sdfColors[i] = 0x00000000;
	m_sdfInvisible = (count == 0);
	return *this;
}

GLPaint& GLPaint::setAntialias(bool antialias) {
	m_antialias = antialias;
	return *this;
}

GLPaint& GLPaint::setTypeface(const GLTypeface& typeface) {
	m_typeface = typeface;
	return *this;
}

GLPaint& GLPaint::setSize(GLfloat size) {
	m_size = size;
	return *this;
}

GLPaint& GLPaint::setAlignX(GLAlignX align) {
	m_alignX = align;
	return *this;
}

GLPaint& GLPaint::setAlignY(GLAlignY align) {
	m_alignY = align;
	return *this;
}

GLPaint& GLPaint::setColor(uint32_t color) {
	m_color = color;
	m_invisible = (color & 0xFF000000) == 0;
	m_color4iv[0] = ((color >> 16) & 0xFF) / 255.0;		// R
	m_color4iv[1] = ((color >> 8) & 0xFF) / 255.0;		// G
	m_color4iv[2] = (color & 0xFF) / 255.0;				// B
	m_color4iv[3] = ((color >> 24) & 0xFF) / 255.0;		// A
	return *this;
}

GLPaint& GLPaint::setLight(GLfloat light, GLfloat alpha) {
	m_color4iv[0] = m_color4iv[1] = m_color4iv[2] = light;
	m_color4iv[3] = alpha;
	m_color =
		(uint32_t)floorf(clamp(m_color4iv[2] * 255.0, 0, 255)) |
		((uint32_t)floorf(clamp(m_color4iv[1] * 255.0, 0, 255)) << 8) |
		((uint32_t)floorf(clamp(m_color4iv[0] * 255.0, 0, 255)) << 16) |
		((uint32_t)floorf(clamp(m_color4iv[3] * 255.0, 0, 255)) << 24);
	m_invisible = (m_color & 0xFF000000) == 0;
	return *this;
}

GLPaint& GLPaint::setStroke(GLfloat width, uint32_t color) {
	m_strokeOffset = -width;
	m_strokeWidth = width;
	m_strokeColor = color;
	m_strokeInvisible = ((color & 0xFF000000) == 0) || (width == 0);
	return *this;
}

GLPaint& GLPaint::setAlpha(uint8_t alpha) {
	m_color = (m_color & 0x00FFFFFF) | ((uint32_t)(alpha & 0xFF) << 24);
	m_invisible = (m_color & 0xFF000000) == 0;
	m_color4iv[3] = (alpha & 0xFF) / 255.0;				// A
	return *this;
}

GLPaint& GLPaint::setShadow(GLfloat width, GLfloat dx, GLfloat dy, uint32_t color) {
	m_shadow[0] = dx;
	m_shadow[1] = dy;
	m_shadowWidth = width;
	m_shadowColor = color;
	m_shadowInvisible = ((color & 0xFF000000) == 0) || ((dx == 0) && (dy == 0) && (width == 0));
	return *this;
}

GLPaint& GLPaint::setScale(GLfloat scale) {
	m_scale = scale;
	return *this;
}
