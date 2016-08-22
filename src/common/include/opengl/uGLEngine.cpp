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

#include "uGLEngine.h"
#include <opengl/uGLRender.h>

void onFrameCallback(GLRender* context, void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	engine->onFrame(context);
}

void onTouchCallback(const wchar_t* event, void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	engine->onTouch(event);
}

void onFileCallback(const CString& url, void* object, void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	engine->onFile(url, object);
}

void onStatusCallback(const LoaderStatus& status, void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	engine->onStatus(status);
}

void onReadyCallback(void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	engine->onReady();
}

void onErrorCallback(const CString& error, void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	engine->onError(error);
}

GLEngine::GLEngine() {
	context = new GLRender(this, 1920, 1080, ::onFrameCallback, ::onTouchCallback);
	context->loader->setCallbacks(onFileCallback, onStatusCallback, onReadyCallback, onErrorCallback);
}

GLEngine::~GLEngine() {
	if (context != NULL) {
		delete context;
	}
}

void GLEngine::setBasePath(const CString& basePath) {
	context->loader->basePath = basePath;
}

void GLEngine::load(const char* json) {
	context->loader->load(json);
}

void GLEngine::onRender() {
	this->context->frame->loop();
}

void GLEngine::onUpdate(int width, int height) {
	context->frame->width = width;
	context->frame->height = height;
}

void GLEngine::onTouch(MotionEvent* event) {
	switch (event->actionEvent) {
		case MotionEvent::ACTION_DOWN:
			context->touchScreen->onTouchStart(event);
			break;
			
		case MotionEvent::ACTION_MOVE:
			context->touchScreen->onTouchMove(event);
			break;
			
		case MotionEvent::ACTION_UP:
			context->touchScreen->onTouchEnd(event);
			break;
		
		default:;
	}
}
