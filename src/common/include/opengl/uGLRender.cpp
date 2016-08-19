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

#include "uGLRender.h"

#include <opengl/uGLCamera.h>
#include <opengl/uGLEngine.h>

const char* GLRender::extensions = NULL;

bool GLRender::isExtensionSupported(const char *extension) {
	if (extensions == NULL)
		return false;
	
	const char *start;
	const char *where, *terminator;
	
	where = strchr(extension, ' ');
	if ( where || *extension == '\0' )
		return false;
	
	for ( start = extensions; ; ) {
		where = strstr( start, extension );
		
		if ( !where )
			break;
		
		terminator = where + strlen( extension );
		
		if ( where == start || *(where - 1) == ' ' )
			if ( *terminator == ' ' || *terminator == '\0' )
				return true;
		
		start = terminator;
	}
	
	return false;
}

GLRender::GLRender(GLEngine& engine, uint32_t width, uint32_t height, GLFrame::onFrameCallback onframe, RefGLTouchScreen::onTouchCallback ontouch) {
	THIS.engine = engine;
	THIS.width = width;
	THIS.height = height;
	
	frame = memNew(frame, GLFrame(engine, this, onframe));
	touchScreen = new GLTouchScreen(this, ontouch);
	loader = new Loader(this);
	
	textures = memNew(textures, GLTextures(this));
	shaders = memNew(shaders, GLShaders(this));
	sprites = memNew(sprites, GLSprites(this));
	scenes = memNew(scenes, GLScenes(this));
	cameras = memNew(cameras, GLCameras(this));
	models = memNew(models, GLModels(this));
	particles = memNew(particles, GLParticles(this));
	drawings = memNew(drawings, GLDrawings(this));
	
	cameras->createCamera(L"gui").ref().size(width, height).layer(0, 0);
	lightsMaxCount = 6;
	
	shaderSprite = NULL;
	shaderParticle = NULL;
	shaderModel = NULL;
	shaderSquareFill = NULL;
	shaderSquareStroke = NULL;
	shaderSquareTexture = NULL;
	
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	
	extensions = (const char*)glGetString(GL_EXTENSIONS);
	isNPOTSupported = isExtensionSupported("GL_OES_texture_npot");
	
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearDepthf(1.0);
	glDepthFunc(GL_LEQUAL);
	resetBlend();
	
	glGenBuffers(1, &m_squareBuffer);
	glGenBuffers(1, &m_textureBuffer);
	glGenBuffers(1, &m_normalBuffer);
	glGenBuffers(1, &m_indexBuffer);
	
	light.set(1.0);
}

GLRender::~GLRender() {
	loader.release();
	touchScreen.release();
	touchScreen = null;
	if (frame != NULL) {
		memDelete(frame);
		frame = NULL;
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_squareBuffer);
	glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &m_squareBuffer);
	m_squareBuffer = 0;

	glBindBuffer(GL_ARRAY_BUFFER, m_textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &m_textureBuffer);
	m_textureBuffer = 0;

	glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &m_normalBuffer);
	m_normalBuffer = 0;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &m_indexBuffer);
	m_indexBuffer = 0;
	
	shaderSprite = NULL;
	shaderParticle = NULL;
	shaderModel = NULL;
	shaderSquareFill = NULL;
	shaderSquareStroke = NULL;
	shaderSquareTexture = NULL;
	
	memDelete(drawings);
	memDelete(particles);
	memDelete(models);
	memDelete(cameras);
	memDelete(scenes);
	memDelete(sprites);
	memDelete(shaders);
	memDelete(textures);
	
	drawings = NULL;
	particles = NULL;
	models = NULL;
	cameras = NULL;
	scenes = NULL;
	sprites = NULL;
	textures = NULL;
	shaders = NULL;
	
	engine = null;
}

void GLRender::resetBlend() {
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
}

void GLRender::activeTexture(GLint index) {
	switch (index) {
		case 1: glActiveTexture(GL_TEXTURE1); break;
		case 2: glActiveTexture(GL_TEXTURE2); break;
		case 3: glActiveTexture(GL_TEXTURE3); break;
		default: glActiveTexture(GL_TEXTURE0); break;
	}
}

void GLRender::cleanup(GLint index) {
	for (GLint i = 0; i < index; i++) {
		activeTexture(i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	glUseProgram(0);
}

GLfloat* GLRender::makeRect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
	m_square[0] = m_square[4] = x1;
	m_square[1] = m_square[3] = y1;
	m_square[2] = m_square[6] = x2;
	m_square[5] = m_square[7] = y2;
	return m_square;
}

GLfloat* GLRender::makeLine(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
	m_line[0] = x1;
	m_line[1] = y1;
	m_line[2] = x2;
	m_line[3] = y2;
	return m_line;
}

GLfloat* GLRender::makeColor(uint32_t* color, uint32_t count) {
	if (count > 5) count = 5;
	for (uint32_t i = 0; i < count; i++) {
		m_color[i * 4 + 0] = (GLfloat)((color[i] >> 16) & 0xFF) / 255.0;	// R
		m_color[i * 4 + 1] = (GLfloat)((color[i] >> 8) & 0xFF) / 255.0;	// G
		m_color[i * 4 + 2] = (GLfloat)(color[i] & 0xFF) / 255.0;			// B
		m_color[i * 4 + 3] = (GLfloat)((color[i] >> 24) & 0xFF) / 255.0;	// A
	}
	return m_color;
}

void GLRender::fill(uint32_t color) {
	GLfloat* c = makeColor(&color, 1);
	glClearColor(c[0], c[1], c[2], c[3]);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GLRender::fillAlpha(uint8_t alpha) {
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	glClearColor(0.0, 0.0, 0.0, (GLfloat)alpha / 255.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void GLRender::fillDepth() {
	glClear(GL_DEPTH_BUFFER_BIT);
}

void GLRender::drawRect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const GLPaint& paint) {
	// TODO:
}

void GLRender::drawTexture(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const wchar_t* key) {
	// TODO:
}

void GLRender::drawEffect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const wchar_t* key, GLfloat localTime, GLfloat worldTime) {
	// TODO:
}

void GLRender::drawEffectMobile(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const wchar_t* key, GLfloat localTime, GLfloat worldTime) {
	// TODO:
}
