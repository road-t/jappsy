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

#include <math.h>

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

GLRender::GLRender(GLEngine* engine, uint32_t width, uint32_t height, GLFrame::onFrameCallback onframe, GLTouchScreen::onTouchCallback ontouch) {
	this->engine = engine;
	this->width = width;
	this->height = height;
	
	frame = new GLFrame(engine, this, onframe, engine);
	touchScreen = new GLTouchScreen(this, ontouch, engine);
	loader = new Loader(this, engine);
	
	textures = new GLTextures(this);
	shaders = new GLShaders(this);
	sprites = new GLSprites(this);
	scenes = new GLScenes(this);
	cameras = new GLCameras(this);
	models = new GLModels(this);
	particles = new GLParticles(this);
	drawings = new GLDrawings(this);
	
	cameras->createCamera(L"gui")->size(width, height)->layer(0, 0);
	lightsMaxCount = 6;
	
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
	delete loader;
	delete touchScreen;
	delete frame;

	glBindBuffer(GL_ARRAY_BUFFER, m_squareBuffer);
	glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &m_squareBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, m_textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &m_textureBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &m_normalBuffer);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &m_indexBuffer);
	
	if (shaderSprite != NULL) {
		delete shaderSprite;
	}
	if (shaderParticle != NULL) {
		delete shaderParticle;
	}
	if (shaderModel != NULL) {
		delete shaderModel;
	}
	if (shaderSquareFill != NULL) {
		delete shaderSquareFill;
	}
	if (shaderSquareStroke != NULL) {
		delete shaderSquareStroke;
	}
	if (shaderSquareTexture != NULL) {
		delete shaderSquareTexture;
	}
	
	delete drawings;
	delete particles;
	delete models;
	delete cameras;
	delete scenes;
	delete sprites;
	delete shaders;
	delete textures;
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

GLfloat* GLRender::makeColor(const uint32_t* color, uint32_t count) {
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
	GLCamera* cam = this->cameras->gui;
	cam->update();
	GLfloat* projection16fv = cam->projection16fv.v;
	
	if ((paint.m_color & 0xFF000000) != 0) {
		if ((paint.m_color & 0xFF000000) != 0xFF000000)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
		
		glUseProgram(shaderSquareFill->program);
		
		glUniformMatrix4fv(shaderSquareFill->uLayerProjectionMatrix, 1, GL_FALSE, projection16fv);
		
		GLfloat* c = makeColor(&paint.m_color, 1);
		glUniform4fv(shaderSquareFill->uColor, 1, c);
		
		glBindBuffer(GL_ARRAY_BUFFER, m_squareBuffer);
		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(x1, y1, x2, y2), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(shaderSquareFill->aVertexPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(shaderSquareFill->aVertexPosition);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glDisableVertexAttribArray(shaderSquareFill->aVertexPosition);
		glDisable(GL_BLEND);
		glUseProgram(0);
	}
	
	if ((paint.m_strokeColor & 0xFF000000) != 0) {
		glEnable(GL_BLEND);
		glUseProgram(shaderSquareStroke->program);
		
		glUniformMatrix4fv(shaderSquareStroke->uLayerProjectionMatrix, 1, GL_FALSE, projection16fv);
		
		GLfloat half = floorf((GLfloat)(paint.m_strokeWidth) / 2.0);
		GLfloat corners[4] = { x1 - half, y1 - half, x2 + half, y2 + half };
		glUniform2fv(shaderSquareStroke->uCorners, 2, corners);
		glUniform1f(shaderSquareStroke->uBorder, paint.m_strokeWidth);
		
		GLfloat* c = makeColor(&paint.m_strokeColor, 1);
		glUniform4fv(shaderSquareStroke->uColor, 1, c);
		
		glBindBuffer(GL_ARRAY_BUFFER, m_squareBuffer);
		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(x1-half, y1-half, x2+half, y2+half), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(shaderSquareStroke->aVertexPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(shaderSquareStroke->aVertexPosition);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glDisableVertexAttribArray(shaderSquareStroke->aVertexPosition);
		glDisable(GL_BLEND);
		glUseProgram(0);
	}
}

void GLRender::drawTexture(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const CString& key) {
	GLCamera* cam = this->cameras->gui;
	cam->update();
	GLfloat* projection16fv = cam->projection16fv.v;
	
	glEnable(GL_BLEND);
	glUseProgram(shaderSquareTexture->program);
	
	glUniformMatrix4fv(shaderSquareTexture->uLayerProjectionMatrix, 1, GL_FALSE, projection16fv);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_squareBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(x1, y1, x2, y2), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(shaderSquareTexture->aVertexPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(shaderSquareTexture->aVertexPosition);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(0, 0, 1, 1), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(shaderSquareTexture->aTextureCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(shaderSquareTexture->aTextureCoord);
	
	GLuint index = textures->get((wchar_t*)key)->bind(0, shaderSquareTexture->uTexture);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	glDisableVertexAttribArray(shaderSquareTexture->aVertexPosition);
	glDisableVertexAttribArray(shaderSquareTexture->aTextureCoord);
	
	cleanup(index);
}

void GLRender::drawEffect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const CString& key, GLfloat localTime, GLfloat worldTime) {
	GLCamera* cam = this->cameras->gui;
	cam->update();
	GLfloat* projection16fv = cam->projection16fv.v;

	GLShader* shader = shaders->get((wchar_t*)key);
	GLuint program = shader->program;
	int uLayerProjectionMatrix = glGetUniformLocation(program, "uLayerProjectionMatrix");
	int aVertexPosition = glGetAttribLocation(program, "aVertexPosition");
	int aTextureCoord = glGetAttribLocation(program, "aTextureCoord");
	int uTime = glGetUniformLocation(program, "uTime");
	int uWorldTime = glGetUniformLocation(program, "uWorldTime");
	int uTexture = glGetUniformLocation(program, "uTexture");
	
	glEnable(GL_BLEND);
	GLuint index = shader->bind(0, uTexture);
	
	glUniform1f(uTime, localTime - floorf(localTime));
	glUniform1f(uWorldTime, worldTime - floorf(worldTime));
	
	glUniformMatrix4fv(uLayerProjectionMatrix, 1, GL_FALSE, projection16fv);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_squareBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(x1, y1, x2, y2), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(aVertexPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(aVertexPosition);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(0, 0, 1, 1), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(aTextureCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(aTextureCoord);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	glDisableVertexAttribArray(aVertexPosition);
	glDisableVertexAttribArray(aTextureCoord);
	
	cleanup(index);
	glDisable(GL_BLEND);
}

void GLRender::drawEffectMobile(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const CString& key, GLfloat localTime, GLfloat worldTime) {
	GLCamera* cam = this->cameras->gui;
	cam->update();
	GLfloat* projection16fv = cam->projection16fv.v;

	GLShader* shader = shaders->get((wchar_t*)key);
	GLuint program = shader->program;
	int uLayerProjectionMatrix = glGetUniformLocation(program, "uLayerProjectionMatrix");
	int aVertexPosition = glGetAttribLocation(program, "aVertexPosition");
	int aTextureCoord = glGetAttribLocation(program, "aTextureCoord");
	int uTime = glGetUniformLocation(program, "uTime");
	int uTexture0 = glGetUniformLocation(program, "uTexture0");
	int uTexture1 = glGetUniformLocation(program, "uTexture1");
 
	static Vector<GLint> effectTexturesVector = Vector<GLint>(2);
	static GLint* effectTextures = effectTexturesVector.items();
	effectTextures[0] = uTexture0;
	effectTextures[1] = uTexture1;
	glEnable(GL_BLEND);
	GLuint index = shader->bind(0, effectTexturesVector);
	
	glUniform2f(uTime, localTime - floorf(localTime), worldTime - floorf(worldTime));
 
	glUniformMatrix4fv(uLayerProjectionMatrix, 1, GL_FALSE, projection16fv);
 
	glBindBuffer(GL_ARRAY_BUFFER, m_squareBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(x1, y1, x2, y2), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(aVertexPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(aVertexPosition);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(0, 0, 1, 1), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(aTextureCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(aTextureCoord);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
 
	glDisableVertexAttribArray(aVertexPosition);
	glDisableVertexAttribArray(aTextureCoord);
 
	cleanup(index);
	glDisable(GL_BLEND);
}

bool GLRender::createShaders(JSONObject shaders) throw(const char*) {
	{
		if (shaders.root != NULL) {
			int32_t count = shaders.root->count();
			if (count > 0) {
				try {
					JsonNode** keys = shaders.root->keys();
					JsonNode** items = shaders.root->items();
					for (int i = 0; i < count; i++) {
						CString key = keys[i]->toString();
						
						int32_t subcount = items[i]->count();
						if (subcount >= 2) {
							JsonNode** subitems = items[i]->items();

							CString vsh = subitems[0]->toString();
							CString fsh = subitems[1]->toString();
							
							this->shaders->createShader((wchar_t*)key, (wchar_t*)vsh, (wchar_t*)fsh);
						}
					}
				} catch (...) {
				}
			}
		}
	}

	int32_t count = this->shaders->list.count();
	GLShader** items = this->shaders->list.items();
	for (int i = 0; i < count; i++) {
		if (!items[i]->checkReady()) {
			return false;
		}
	}
	CString** keys = this->shaders->list.keys();
	for (int i = 0; i < count; i++) {
		CString key = *(keys[i]);
		GLShader* shader = items[i];
		GLuint program = shader->program;
		
		if (key.equals(L"sprite")) {
			shaderSprite = new GLSpriteShader(shader);
			shaderSprite->program = program;
				
			shaderSprite->uLayerProjectionMatrix = glGetUniformLocation(program, "uLayerProjectionMatrix");
			shaderSprite->uPosition = glGetUniformLocation(program, "uPosition");
			shaderSprite->uTexture = glGetUniformLocation(program, "uTexture");
			shaderSprite->aVertexPosition = glGetAttribLocation(program, "aVertexPosition");
			shaderSprite->aTextureCoord = glGetAttribLocation(program, "aTextureCoord");
			
			shaderSprite->uLight = glGetUniformLocation(program, "uLight");
			shaderSprite->uTime = glGetUniformLocation(program, "uTime");
		} else if (key.equals(L"particle")) {
			shaderParticle = new GLParticleShader(shader);
			shaderParticle->program = program;
			
			shaderParticle->uModelViewProjectionMatrix = glGetUniformLocation(program, "uModelViewProjectionMatrix");
				
			shaderParticle->uPixelX = glGetUniformLocation(program, "uPixelX");
			shaderParticle->uPixelY = glGetUniformLocation(program, "uPixelY");
			shaderParticle->uTime = glGetUniformLocation(program, "uTime");
			shaderParticle->uTexture = glGetUniformLocation(program, "uTexture");
			shaderParticle->uColor = glGetUniformLocation(program, "uColor");
				
			shaderParticle->aVertexPosition = glGetAttribLocation(program, "aVertexPosition");
			shaderParticle->aTextureCoord = glGetAttribLocation(program, "aTextureCoord");
			shaderParticle->aVelocity = glGetAttribLocation(program, "aVelocity");
			shaderParticle->aAcceleration = glGetAttribLocation(program, "aAcceleration");
			shaderParticle->aTime = glGetAttribLocation(program, "aTime");
		} else if (key.equals(L"model")) {
			shaderModel = new GLModelShader(shader);
			shaderModel->program = program;

			shaderModel->uModelViewProjectionMatrix = glGetUniformLocation(program, "uModelViewProjectionMatrix");
			shaderModel->uModelViewMatrix = glGetUniformLocation(program, "uModelViewMatrix");
			shaderModel->uNormalMatrix = glGetUniformLocation(program, "uNormalMatrix");
				
			shaderModel->uAmbientLightColor = glGetUniformLocation(program, "uAmbientLightColor");
				
			shaderModel->uLightsCount = glGetUniformLocation(program, "uLightsCount");
			shaderModel->uLights = glGetUniformLocation(program, "uLights");
				
			shaderModel->uColors = glGetUniformLocation(program, "uColors");
			shaderModel->uTexture = glGetUniformLocation(program, "uTexture");
				
			shaderModel->aVertexPosition = glGetAttribLocation(program, "aVertexPosition");
			shaderModel->aTextureCoord = glGetAttribLocation(program, "aTextureCoord");
			shaderModel->aVertexNormal = glGetAttribLocation(program, "aVertexNormal");
		} else if (key.equals(L"square_fill")) {
			shaderSquareFill = new GLSquareFillShader(shader);
			shaderSquareFill->program = program;

			shaderSquareFill->uLayerProjectionMatrix = glGetUniformLocation(program, "uLayerProjectionMatrix");
			shaderSquareFill->uColor = glGetUniformLocation(program, "uColor");
			shaderSquareFill->aVertexPosition = glGetAttribLocation(program, "aVertexPosition");
		} else if (key.equals(L"square_stroke")) {
			shaderSquareStroke = new GLSquareStrokeShader(shader);
			shaderSquareStroke->program = program;

			shaderSquareStroke->uLayerProjectionMatrix = glGetUniformLocation(program, "uLayerProjectionMatrix");
			shaderSquareStroke->uCorners = glGetUniformLocation(program, "uCorners");
			shaderSquareStroke->uBorder = glGetUniformLocation(program, "uBorder");
			shaderSquareStroke->uColor = glGetUniformLocation(program, "uColor");
			shaderSquareStroke->aVertexPosition = glGetAttribLocation(program, "aVertexPosition");
		} else if (key.equals(L"square_texture")) {
			shaderSquareTexture = new GLSquareTextureShader(shader);
			shaderSquareTexture->program = program;

			shaderSquareTexture->uLayerProjectionMatrix = glGetUniformLocation(program, "uLayerProjectionMatrix");
			shaderSquareTexture->uTexture = glGetUniformLocation(program, "uTexture");
			shaderSquareTexture->aVertexPosition = glGetAttribLocation(program, "aVertexPosition");
			shaderSquareTexture->aTextureCoord = glGetAttribLocation(program, "aTextureCoord");
		}
	}
	
	return true;
}

void GLRender::createSprites(JSONObject sprites) throw(const char*) {
	
}

void GLRender::createDrawings(JSONObject drawings) throw(const char*) {
	
}
