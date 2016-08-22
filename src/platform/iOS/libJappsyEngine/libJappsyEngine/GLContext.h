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

#ifndef JAPPSY_GLRENDERCONTEXT_H
#define JAPPSY_GLRENDERCONTEXT_H

#import <UIKit/UIKit.h>
#include <opengl/uOpenGL.h>
#include <opengl/uGLEngine.h>
#include <event/uMotionEvent.h>

class GLContext {
private:
	EAGLContext* context = NULL;
	GLEngine* engine = NULL;
	
	GLuint frameBuffer = 0;
	GLuint colorRenderBuffer = 0;
	GLuint depthRenderBuffer = 0;
	
	void release();
public:
	GLint width;
	GLint height;
	
	GLContext(EAGLContext* context, CAEAGLLayer* layer, float scaleFactor) throw(const char*);
	~GLContext();
	void update(CAEAGLLayer* layer) throw(const char*);
	void render();
	void touch(MotionEvent* event);
	
	void initialize(GLEngine* engine);
};

#endif //JAPPSY_GLRENDERCONTEXT_H