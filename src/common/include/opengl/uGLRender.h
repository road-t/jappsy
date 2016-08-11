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

#include "../platform.h"
//TODO: #include "uLoader.h"
#include "uOpenGL.h"
#include "../data/uVector.h"

class GLEngine;

#include "uGLFrame.h"
#include "uGLTouchScreen.h"

class GLTextures;
class GLShaders;
class GLShader;
class GLSprites;
class GLScenes;
class GLCameras;
class GLModels;
class GLParticles;
class GLDrawings;

class GLRender {
public:
	GLEngine* engine;
	
	uint32_t width;
	uint32_t height;
	
	GLFrame* frame;
	GLTouchScreen* touchScreen;
	//TODO: loader
	
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
	
	GLShader* shaderSprite;
	GLShader* shaderParticle;
	GLShader* shaderModel;
	GLShader* shaderSquareFill;
	GLShader* shaderSquareStroke;
	GLShader* shaderSquareTexture;
	
	uint32_t maxTextureSize;
	bool isNPOTSupported;
	
	GLuint m_squareBuffer;
	GLuint m_textureBuffer;
	GLuint m_normalBuffer;
	GLuint m_indexBuffer;
	
	Vec4 light;
	
	GLRender(GLEngine* engine, uint32_t width, uint32_t height, GLFrame::onFrameCallback onframe, GLTouchScreen::onTouchCallback ontouch);
	~GLRender();
};

#endif //JAPPSY_UGLRENDER_H