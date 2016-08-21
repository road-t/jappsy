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

void onFrameCallback(GLRender* context, JObject& userData) {
	GLEngine* engine = (GLEngine*)(&userData);
	engine->ref().onFrame(context);
}

void onTouchCallback(const wchar_t* event, JObject& userData) {
	GLEngine* engine = (GLEngine*)(&userData);
	engine->ref().onTouch(event);
}

void onFileCallback(const JString& url, const JObject& object, JObject userData) {
	GLEngine* engine = (GLEngine*)(&userData);
	engine->ref().onFile(url, object);
}

void onStatusCallback(const LoaderStatus& status, JObject userData) {
	GLEngine* engine = (GLEngine*)(&userData);
	engine->ref().onStatus(status);
}

void onReadyCallback(const JSONObject& result, JObject userData) {
	GLEngine* engine = (GLEngine*)(&userData);
	engine->ref().onReady(result);
}

void onErrorCallback(const JString& error, JObject userData) {
	GLEngine* engine = (GLEngine*)(&userData);
	engine->ref().onError(error);
}

RefGLEngine::RefGLEngine() {
	context = memNew(context, GLRender(this, 1920, 1080, ::onFrameCallback, ::onTouchCallback));
	context->loader.ref().setCallbacks(onFileCallback, onStatusCallback, onReadyCallback, onErrorCallback);
}

RefGLEngine::~RefGLEngine() {
	release();
}

void RefGLEngine::release() {
	if (context != NULL) {
		memDelete(context);
		context = NULL;
	}
}

void RefGLEngine::setBasePath(const JString& basePath) {
	context->loader.ref().basePath = basePath;
}

void RefGLEngine::load(const char* json) {
	context->loader.ref().load(json);
}

void RefGLEngine::onRender() {
	THIS.context->frame->loop();
}

void RefGLEngine::onUpdate(int width, int height) {
	THIS.context->frame->width = width;
	THIS.context->frame->height = height;
}

void RefGLEngine::onTouch(MotionEvent* event) {
	switch (event->actionEvent) {
		case MotionEvent::ACTION_DOWN:
			THIS.context->touchScreen.ref().onTouchStart(event);
			break;
			
		case MotionEvent::ACTION_MOVE:
			THIS.context->touchScreen.ref().onTouchMove(event);
			break;
			
		case MotionEvent::ACTION_UP:
			THIS.context->touchScreen.ref().onTouchEnd(event);
			break;
		
		default:;
	}
}
