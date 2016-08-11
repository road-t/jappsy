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

#ifndef JAPPSY_UGLFRAME_H
#define JAPPSY_UGLFRAME_H

#include "../platform.h"
#include "../data/uSmoothValue.h"
#include "uOpenGL.h"

class GLEngine;
class GLRender;

class GLFrame {
public:
	typedef void (*onFrameCallback)(GLRender* context);
	
	GLEngine* engine;
	GLRender* context;
	
	GLint width;
	GLint height;
	
	uint64_t currentTime;
	uint64_t lastFrame;
	SmoothValue* frames;
	uint32_t fps;
	
	onFrameCallback onFrame;
	
	GLFrame(GLEngine* engine, GLRender* context, onFrameCallback callback);
	~GLFrame();
	
	void loop();
};

#endif //JAPPSY_UGLFRAME_H