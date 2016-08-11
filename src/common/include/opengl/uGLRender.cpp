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

GLRender::GLRender(GLEngine* engine, uint32_t width, uint32_t height, GLFrame::onFrameCallback onframe, GLTouchScreen::onTouchCallback ontouch) {
	this->engine = engine;
	this->width = width;
	this->height = height;
	
	frame = new GLFrame(engine, this, onframe);
	touchScreen = new GLTouchScreen(this, ontouch);
	// TODO: loader =
	
	//textures = new GLTextures(this);
	//shaders = new GLShaders(this);
	//sprites = new GLSprites(this);
	//scenes = new GLScenes(this);
	cameras = new GLCameras(this);
	//models = new GLModels(this);
	//particles = new GLParticles(this);
	//drawings = new GLDrawings(this);
}

GLRender::~GLRender() {
	
}
