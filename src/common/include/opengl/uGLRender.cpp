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
	this->sceneRatio = this->ratio = (GLfloat)width / (GLfloat)height;
	this->ratio16fv.identity();
	
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
	funcs = new GLFuncs(this);
	mixer = new GLSoundMixer(this);
	
	cameras->createCamera(L"gui")->size(width, height)->layer(0, 0);
	cameras->createCamera(L"background")->size(width, height)->background();
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
	LOG("GLRender Create");
}

GLRender::~GLRender() {
	LOG("GLRender Destroy");
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

	delete mixer;
	delete funcs;
	delete drawings;
	delete particles;
	delete models;
	delete cameras;
	delete scenes;
	delete sprites;
	delete shaders;
	delete textures;
}

void GLRender::updateRatio(GLfloat width, GLfloat height) {
	GLfloat ratio = width / height;
	if (this->ratio != ratio) {
		if (this->sceneRatio > ratio) { // Экран уже чем требуется
			this->ratio16fv[0] = 1.0;
			this->ratio16fv[5] = ratio / this->sceneRatio;
			this->ratio16fv[12] = 0.0;
			this->ratio16fv[13] = 0.0;
			this->ratio16fv[10] = this->ratio16fv[15] = 1.0;
		} else { // Экран шире чем требуется
			this->ratio16fv[0] = this->sceneRatio / ratio;
			this->ratio16fv[5] = 1.0;
			this->ratio16fv[12] = 0.0;
			this->ratio16fv[13] = 0.0;
			this->ratio16fv[10] = this->ratio16fv[15] = 1.0;
		}
		this->ratio = ratio;
		this->cameras->forceUpdate();
	}
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
		m_color[i * 4 + 0] = (GLfloat)((color[i] >> 16) & 0xFF) / 255.0f;	// R
		m_color[i * 4 + 1] = (GLfloat)((color[i] >> 8) & 0xFF) / 255.0f;	// G
		m_color[i * 4 + 2] = (GLfloat)(color[i] & 0xFF) / 255.0f;			// B
		m_color[i * 4 + 3] = (GLfloat)((color[i] >> 24) & 0xFF) / 255.0f;	// A
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
	glClearColor(0.0, 0.0, 0.0, (GLfloat)alpha / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void GLRender::fillDepth() {
	glClear(GL_DEPTH_BUFFER_BIT);
}

void GLRender::drawRect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const GLPaint& paint, GLCamera* camera) {
	GLCamera* cam = (camera != NULL) ? camera : this->cameras->gui;
	cam->update();
	GLfloat* projection16fv = cam->projection16fv.v;
	
	if (((paint.m_color & 0xFF000000) != 0) && (shaderSquareFill != NULL)) {
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
		glVertexAttribPointer(shaderSquareFill->aVertexPosition, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(shaderSquareFill->aVertexPosition);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glDisableVertexAttribArray(shaderSquareFill->aVertexPosition);
		glDisable(GL_BLEND);
		glUseProgram(0);
	}
	
	if (((paint.m_strokeColor & 0xFF000000) != 0) && (shaderSquareStroke != NULL)) {
		glEnable(GL_BLEND);
		glUseProgram(shaderSquareStroke->program);
		
		glUniformMatrix4fv(shaderSquareStroke->uLayerProjectionMatrix, 1, GL_FALSE, projection16fv);
		
		GLfloat half = floorf((GLfloat)(paint.m_strokeWidth) / 2.0f);
		GLfloat corners[4] = { x1 - half, y1 - half, x2 + half, y2 + half };
		glUniform2fv(shaderSquareStroke->uCorners, 2, corners);
		glUniform1f(shaderSquareStroke->uBorder, paint.m_strokeWidth);
		
		GLfloat* c = makeColor(&paint.m_strokeColor, 1);
		glUniform4fv(shaderSquareStroke->uColor, 1, c);
		
		glBindBuffer(GL_ARRAY_BUFFER, m_squareBuffer);
		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(x1-half, y1-half, x2+half, y2+half), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(shaderSquareStroke->aVertexPosition, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(shaderSquareStroke->aVertexPosition);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glDisableVertexAttribArray(shaderSquareStroke->aVertexPosition);
		glDisable(GL_BLEND);
		glUseProgram(0);
	}
}

void GLRender::drawTexture(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const CString& key, GLCamera* camera) {
	GLCamera* cam = (camera != NULL) ? camera : this->cameras->gui;
	cam->update();
	GLfloat* projection16fv = cam->projection16fv.v;
	
	if (shaderSquareTexture == NULL)
		return;
	
	glEnable(GL_BLEND);
	glUseProgram(shaderSquareTexture->program);
	
	glUniformMatrix4fv(shaderSquareTexture->uLayerProjectionMatrix, 1, GL_FALSE, projection16fv);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_squareBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(x1, y1, x2, y2), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(shaderSquareTexture->aVertexPosition, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(shaderSquareTexture->aVertexPosition);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(0, 0, 1, 1), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(shaderSquareTexture->aTextureCoord, 2, GL_FLOAT, GL_FALSE, 0, NULL);
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

	GLShader* shader;
	try {
		shader = shaders->get(key);
	} catch (...) {
		return;
	}
	GLuint program = shader->program;
	GLuint uLayerProjectionMatrix = (GLuint)glGetUniformLocation(program, "uLayerProjectionMatrix");
	GLuint aVertexPosition = (GLuint)glGetAttribLocation(program, "aVertexPosition");
	GLuint aTextureCoord = (GLuint)glGetAttribLocation(program, "aTextureCoord");
	GLuint uTime = (GLuint)glGetUniformLocation(program, "uTime");
	GLuint uWorldTime = (GLuint)glGetUniformLocation(program, "uWorldTime");
	GLuint uTexture = (GLuint)glGetUniformLocation(program, "uTexture");
	
	glEnable(GL_BLEND);
	GLuint index = shader->bind(0, uTexture);
	
	glUniform1f(uTime, localTime - floorf(localTime));
	glUniform1f(uWorldTime, worldTime - floorf(worldTime));
	
	glUniformMatrix4fv(uLayerProjectionMatrix, 1, GL_FALSE, projection16fv);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_squareBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(x1, y1, x2, y2), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(aVertexPosition, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(aVertexPosition);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(0, 0, 1, 1), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(aTextureCoord, 2, GL_FLOAT, GL_FALSE, 0, NULL);
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

	GLShader* shader;
	try {
		shader = shaders->get(key);
	} catch (...) {
		return;
	}
	GLuint program = shader->program;
	GLuint uLayerProjectionMatrix = (GLuint)glGetUniformLocation(program, "uLayerProjectionMatrix");
	GLuint aVertexPosition = (GLuint)glGetAttribLocation(program, "aVertexPosition");
	GLuint aTextureCoord = (GLuint)glGetAttribLocation(program, "aTextureCoord");
	GLuint uTime = (GLuint)glGetUniformLocation(program, "uTime");
	GLuint uTexture0 = (GLuint)glGetUniformLocation(program, "uTexture0");
	GLuint uTexture1 = (GLuint)glGetUniformLocation(program, "uTexture1");
 
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
	glVertexAttribPointer(aVertexPosition, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(aVertexPosition);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), makeRect(0, 0, 1, 1), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(aTextureCoord, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(aTextureCoord);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
 
	glDisableVertexAttribArray(aVertexPosition);
	glDisableVertexAttribArray(aTextureCoord);
 
	cleanup(index);
	glDisable(GL_BLEND);
}

bool GLRender::createShaders(JSONObject* shaders, void* library) throw(const char*) {
	{
		if ((shaders != NULL) && (shaders->root != NULL)) {
			int32_t count = shaders->root->count();
			if (count > 0) {
				try {
					const struct JsonNode** keys = shaders->root->keys();
					const struct JsonNode** items = shaders->root->items();
					for (int i = 0; i < count; i++) {
						CString key = keys[i]->toString();
						
						int32_t subcount = items[i]->count();
						if (subcount >= 2) {
							const struct JsonNode** subitems = items[i]->items();

							CString vsh = subitems[0]->toString();
							CString fsh = subitems[1]->toString();
							
							this->shaders->createShader(key, (wchar_t*)vsh, (wchar_t*)fsh, library);
						}
					}
				} catch (...) {
					return false;
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
		
		if (key.equals(L"q0_sprite")) {
			shaderSprite = new GLSpriteShader(shader);
			shaderSprite->program = program;
				
			shaderSprite->uLayerProjectionMatrix = (GLuint)glGetUniformLocation(program, "uLayerProjectionMatrix");
			shaderSprite->uPosition = (GLuint)glGetUniformLocation(program, "uPosition");
			shaderSprite->uTexture = (GLuint)glGetUniformLocation(program, "uTexture");
			shaderSprite->aVertexPosition = (GLuint)glGetAttribLocation(program, "aVertexPosition");
			shaderSprite->aTextureCoord = (GLuint)glGetAttribLocation(program, "aTextureCoord");
			
			shaderSprite->uLight = (GLuint)glGetUniformLocation(program, "uLight");
			shaderSprite->uTime = (GLuint)glGetUniformLocation(program, "uTime");
		} else if (key.equals(L"q0_particle")) {
			shaderParticle = new GLParticleShader(shader);
			shaderParticle->program = program;
			
			shaderParticle->uModelViewProjectionMatrix = (GLuint)glGetUniformLocation(program, "uModelViewProjectionMatrix");
				
			shaderParticle->uPixelX = (GLuint)glGetUniformLocation(program, "uPixelX");
			shaderParticle->uPixelY = (GLuint)glGetUniformLocation(program, "uPixelY");
			shaderParticle->uTime = (GLuint)glGetUniformLocation(program, "uTime");
			shaderParticle->uTexture = (GLuint)glGetUniformLocation(program, "uTexture");
			shaderParticle->uColor = (GLuint)glGetUniformLocation(program, "uColor");
				
			shaderParticle->aVertexPosition = (GLuint)glGetAttribLocation(program, "aVertexPosition");
			shaderParticle->aTextureCoord = (GLuint)glGetAttribLocation(program, "aTextureCoord");
			shaderParticle->aVelocity = (GLuint)glGetAttribLocation(program, "aVelocity");
			shaderParticle->aAcceleration = (GLuint)glGetAttribLocation(program, "aAcceleration");
			shaderParticle->aTime = (GLuint)glGetAttribLocation(program, "aTime");
		} else if (key.equals(L"q0_model") || key.equals(L"q1_model")) {
			shaderModel = new GLModelShader(shader);
			shaderModel->program = program;

			shaderModel->uModelViewProjectionMatrix = (GLuint)glGetUniformLocation(program, "uModelViewProjectionMatrix");
			shaderModel->uModelViewMatrix = (GLuint)glGetUniformLocation(program, "uModelViewMatrix");
			shaderModel->uNormalMatrix = (GLuint)glGetUniformLocation(program, "uNormalMatrix");
			
			shaderModel->uAmbientLightColor = (GLuint)glGetUniformLocation(program, "uAmbientLightColor");
				
			shaderModel->uLightsCount = (GLuint)glGetUniformLocation(program, "uLightsCount");
			shaderModel->uLights = (GLuint)glGetUniformLocation(program, "uLights");
				
			shaderModel->uColors = (GLuint)glGetUniformLocation(program, "uColors");
			shaderModel->uTexture = (GLuint)glGetUniformLocation(program, "uTexture");
				
			shaderModel->aVertexPosition = (GLuint)glGetAttribLocation(program, "aVertexPosition");
			shaderModel->aTextureCoord = (GLuint)glGetAttribLocation(program, "aTextureCoord");
			shaderModel->aVertexNormal = (GLuint)glGetAttribLocation(program, "aVertexNormal");
		} else if (key.equals(L"q0_square_fill")) {
			shaderSquareFill = new GLSquareFillShader(shader);
			shaderSquareFill->program = program;

			shaderSquareFill->uLayerProjectionMatrix = (GLuint)glGetUniformLocation(program, "uLayerProjectionMatrix");
			shaderSquareFill->uColor = (GLuint)glGetUniformLocation(program, "uColor");
			shaderSquareFill->aVertexPosition = (GLuint)glGetAttribLocation(program, "aVertexPosition");
		} else if (key.equals(L"q0_square_stroke")) {
			shaderSquareStroke = new GLSquareStrokeShader(shader);
			shaderSquareStroke->program = program;

			shaderSquareStroke->uLayerProjectionMatrix = (GLuint)glGetUniformLocation(program, "uLayerProjectionMatrix");
			shaderSquareStroke->uCorners = (GLuint)glGetUniformLocation(program, "uCorners");
			shaderSquareStroke->uBorder = (GLuint)glGetUniformLocation(program, "uBorder");
			shaderSquareStroke->uColor = (GLuint)glGetUniformLocation(program, "uColor");
			shaderSquareStroke->aVertexPosition = (GLuint)glGetAttribLocation(program, "aVertexPosition");
		} else if (key.equals(L"q0_square_texture")) {
			shaderSquareTexture = new GLSquareTextureShader(shader);
			shaderSquareTexture->program = program;

			shaderSquareTexture->uLayerProjectionMatrix = (GLuint)glGetUniformLocation(program, "uLayerProjectionMatrix");
			shaderSquareTexture->uTexture = (GLuint)glGetUniformLocation(program, "uTexture");
			shaderSquareTexture->aVertexPosition = (GLuint)glGetAttribLocation(program, "aVertexPosition");
			shaderSquareTexture->aTextureCoord = (GLuint)glGetAttribLocation(program, "aTextureCoord");
		}
	}
	
	return true;
}

bool GLRender::createModels(JSONObject models, void* library) throw(const char*) {
	int32_t count = this->models->list.count();
	GLModel** items = this->models->list.items();
	for (int i = 0; i < count; i++) {
		if (!items[i]->checkReady()) {
			return false;
		}
	}
	
	return true;
}

bool GLRender::createSprites(JSONObject sprites) throw(const char*) {
	if (sprites.root != NULL) {
		int32_t count = sprites.root->count();
		if (count > 0) {
			try {
				const struct JsonNode** keys = sprites.root->keys();
				const struct JsonNode** items = sprites.root->items();
				for (int i = 0; i < count; i++) {
					const struct JsonNode* info = items[i];
					if (info->isArray()) {
						CString key = keys[i]->toString();
					
						CString textureKey = info->get(0)->toString();
						Vec2 size;
						size.x = (GLfloat)info->get(1)->get(0)->toDouble();
						size.y = (GLfloat)info->get(1)->get(1)->toDouble();
						GLuint frames = (GLuint)info->optInt(3, 1);
						
						Vec2 textureOfs;
						Vec2* first = NULL;
						try {
							if (info->get(2)->isArray()) {
								first = &textureOfs;
								first->x = (GLfloat)info->get(2)->get(0)->toDouble();
								first->y = (GLfloat)info->get(2)->get(1)->toDouble();
							}
						} catch (...) {
						}
						
						Vec2 textureStep;
						Vec2* next = NULL;
						try {
							if (info->get(4)->isArray()) {
								next = &textureStep;
								next->x = (GLfloat)info->get(4)->get(0)->toDouble();
								next->y = (GLfloat)info->get(4)->get(1)->toDouble();
							}
						} catch (...) {
						}
						
						this->sprites->createSprite(key, textureKey, size, frames, first, next);
					}
				}
			} catch (...) {
				return false;
			}
		}
	}
	
	return true;
}

bool GLRender::createDrawings(JSONObject drawings) throw(const char*) {
	if (drawings.root != NULL) {
		int32_t count = drawings.root->count();
		if (count > 0) {
			try {
				const struct JsonNode** keys = drawings.root->keys();
				const struct JsonNode** items = drawings.root->items();
				for (int i = 0; i < count; i++) {
					const struct JsonNode* info = items[i];
					if (info->isArray()) {
						CString key = keys[i]->toString();
						
						CString spriteKey = info->get(0)->toString();
						Vec2 position;
						position.x = (GLfloat)info->get(1)->get(0)->toDouble();
						position.y = (GLfloat)info->get(1)->get(1)->toDouble();
						
						Vector<GLshort> frameIndexes;
						try {
							const struct JsonNode* frames = info->get(2);
							if (frames->isArray()) {
								int32_t count2 = frames->count();
								const struct JsonNode** items2 = frames->items();
								for (int j = 0; j < count2; j++) {
									frameIndexes.push( (GLshort)items2[j]->toInt() );
								}
							} else {
								frameIndexes.push( (GLshort)frames->toInt() );
							}
						} catch (...) {
						}
						if (frameIndexes.count() == 0) {
							frameIndexes.push(0);
						}
						
						this->drawings->createDrawing(key, spriteKey, position, &frameIndexes, NULL);
					}
				}
			} catch (...) {
				return false;
			}
		}
	}

	return true;
}
