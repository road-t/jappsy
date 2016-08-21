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

#ifndef JAPPSY_UGLRENDER_H
#define JAPPSY_UGLRENDER_H

#include <platform.h>
#include <net/uLoader.h>
#include <opengl/uOpenGL.h>
#include <data/uVector.h>

#include <opengl/uGLEngine.h>
#include <opengl/uGLFrame.h>
#include <opengl/uGLTouchScreen.h>
#include <opengl/uGLPaint.h>
#include <opengl/uGLTexture.h>
#include <opengl/uGLShader.h>
#include <opengl/uGLSprite.h>
#include <opengl/uGLScene.h>
#include <opengl/uGLCamera.h>
#include <opengl/uGLModel.h>
#include <opengl/uGLParticle.h>
#include <opengl/uGLDrawing.h>

class GLSpriteShader : public GLShader {
public:
	JRefClassEx(GLSpriteShader, GLShader, RefGLShader);
	
	GLuint program = 0;
	
	int uLayerProjectionMatrix;
	int uPosition;
	int uTexture;
	int aVertexPosition;
	int aTextureCoord;
	
	int uLight;
	int uTime;
};

class GLParticleShader : public GLShader {
public:
	JRefClassEx(GLParticleShader, GLShader, RefGLShader);
	
	GLuint program = 0;
	
	int uModelViewProjectionMatrix;
	
	int uPixelX;
	int uPixelY;
	int uTime;
	int uTexture;
	int uColor;
	
	int aVertexPosition;
	int aTextureCoord;
	int aVelocity;
	int aAcceleration;
	int aTime;
};

class GLModelShader : public GLShader {
public:
	JRefClassEx(GLModelShader, GLShader, RefGLShader);

	GLuint program = 0;
	
	int uModelViewProjectionMatrix;
	int uModelViewMatrix;
	int uNormalMatrix;
	
	int uAmbientLightColor;
	
	int uLightsCount;
	int uLights;
	
	int uColors;
	int uTexture;
	
	int aVertexPosition;
	int aTextureCoord;
	int aVertexNormal;
};

class GLSquareFillShader : public GLShader {
public:
	JRefClassEx(GLSquareFillShader, GLShader, RefGLShader);
	
	GLuint program = 0;
	
	int uLayerProjectionMatrix;
	int uColor;
	int aVertexPosition;
};

class GLSquareStrokeShader : public GLShader {
public:
	JRefClassEx(GLSquareStrokeShader, GLShader, RefGLShader);
	
	GLuint program = 0;
	
	int uLayerProjectionMatrix;
	int uCorners;
	int uBorder;
	int uColor;
	int aVertexPosition;
};

class GLSquareTextureShader : public GLShader {
public:
	JRefClassEx(GLSquareTextureShader, GLShader, RefGLShader);
	
	GLuint program = 0;
	
	int uLayerProjectionMatrix;
	int uTexture;
	int aVertexPosition;
	int aTextureCoord;
};

class GLRender {
private:
	static const char* extensions;
	static bool isExtensionSupported(const char *extension);
	
public:
	GLEngine engine;
	
	uint32_t width;
	uint32_t height;
	
	GLFrame* frame;
	GLTouchScreen touchScreen;
	Loader loader;
	
	GLfloat m_square[8];
	GLfloat m_line[4];
	GLfloat m_color[20];
	
	GLTextures* textures;
	GLShaders* shaders;
	GLSprites* sprites;
	GLScenes* scenes;
	GLCameras* cameras;
	GLModels* models;
	GLParticles* particles;
	GLDrawings* drawings;
	
	uint32_t lightsMaxCount = 6;
	
	GLSpriteShader shaderSprite;
	GLParticleShader shaderParticle;
	GLModelShader shaderModel;
	GLSquareFillShader shaderSquareFill;
	GLSquareStrokeShader shaderSquareStroke;
	GLSquareTextureShader shaderSquareTexture;
	
	GLint maxTextureSize;
	bool isNPOTSupported;
	
	GLuint m_squareBuffer;
	GLuint m_textureBuffer;
	GLuint m_normalBuffer;
	GLuint m_indexBuffer;
	
	Vec4 light;
	
	GLRender(GLEngine engine, uint32_t width, uint32_t height, GLFrame::onFrameCallback onframe, RefGLTouchScreen::onTouchCallback ontouch);
	~GLRender();
	
	void resetBlend();
	void activeTexture(GLint index);
	void cleanup(GLint index);
	
	GLfloat* makeRect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
	GLfloat* makeLine(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
	GLfloat* makeColor(const uint32_t* color, uint32_t count);
	
	void fill(uint32_t color);
	void fillAlpha(uint8_t alpha);
	void fillDepth();
	
	void drawRect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const GLPaint& paint);
	void drawTexture(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const JString& key);
	void drawEffect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const JString& key, GLfloat localTime, GLfloat worldTime);
	void drawEffectMobile(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const JString& key, GLfloat localTime, GLfloat worldTime);
	
	bool createShaders(JSONObject shaders);
	void createModels(JSONObject models);
	void createSprites(JSONObject sprites);
	void createDrawings(JSONObject drawings);
};

#endif //JAPPSY_UGLRENDER_H