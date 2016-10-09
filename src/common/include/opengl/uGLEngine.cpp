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
	try {
		engine->onFrame(context);
	} catch (const char* e) {
		LOG("Jappsy Engine Fatal Error (onFrame): %s", e);
	} catch (...) {
		LOG("Jappsy Engine Unknown Fatal Error (onFrame)");
	}
}

void* onTouchCallback(const CString& event, void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	try {
		engine->onTouch(event);
	} catch (const char* e) {
		LOG("Jappsy Engine Fatal Error (onTouch): %s", e);
	} catch (...) {
		LOG("Jappsy Engine Unknown Fatal Error (onTouch)");
	}
	return NULL;
}

void* onFileCallback(const Loader::File& info, void* object, void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	try {
		engine->onFile(info, object);
	} catch (const char* e) {
		LOG("Jappsy Engine Fatal Error (onFile): %s", e);
	} catch (...) {
		LOG("Jappsy Engine Unknown Fatal Error (onFile)");
	}
	return NULL;
}

void* onStatusCallback(const LoaderStatus& status, void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	try {
		engine->onStatus(status);
	} catch (const char* e) {
		LOG("Jappsy Engine Fatal Error (onStatus): %s", e);
	} catch (...) {
		LOG("Jappsy Engine Unknown Fatal Error (onStatus)");
	}
	return NULL;
}

void* onPreloadCallback(void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	try {
		engine->onPreload();
	} catch (const char* e) {
		LOG("Jappsy Engine Fatal Error (onPreload): %s", e);
	} catch (...) {
		LOG("Jappsy Engine Unknown Fatal Error (onPreload)");
	}
	return NULL;
}

void* onLoadCallback(void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	try {
		engine->onLoad();
	} catch (const char* e) {
		LOG("Jappsy Engine Fatal Error (onLoad): %s", e);
	} catch (...) {
		LOG("Jappsy Engine Unknown Fatal Error (onLoad)");
	}
	return NULL;
}

void* onErrorCallback(const CString& error, void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	try {
		engine->onError(error);
	} catch (const char* e) {
		LOG("Jappsy Engine Fatal Error (onError): %s", e);
	} catch (...) {
		LOG("Jappsy Engine Unknown Fatal Error (onError)");
	}
	return NULL;
}

void* onRetryCallback(void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	try {
		engine->onRetry();
	} catch (const char* e) {
		LOG("Jappsy Engine Fatal Error (onRetry): %s", e);
	} catch (...) {
		LOG("Jappsy Engine Unknown Fatal Error (onRetry)");
	}
	return NULL;
}

void* onFatalCallback(const CString& error, void* userData) {
	GLEngine* engine = (GLEngine*)userData;
	try {
		engine->onFatal(error);
	} catch (const char* e) {
		LOG("Jappsy Engine Fatal Error (onFatal): %s", e);
	} catch (...) {
		LOG("Jappsy Engine Unknown Fatal Error (onFatal)");
	}
	return NULL;
}

GLEngine::GLEngine() {
	context = new GLRender(this, 1920, 1080, ::onFrameCallback, ::onTouchCallback);
	onminimize = NULL;
}

GLEngine::~GLEngine() {
	if (context != NULL) {
		delete context;
	}
	if (cache != NULL) {
		delete cache;
	}
}

void GLEngine::shutdown() {
	context->loader->release();
	
	// TODO: Функция вызывается в ОМ для завершения игры, но при этом завершение не происходит... (разобраться)
}

void GLEngine::setBasePath(const CString& basePath) {
	context->loader->basePath = basePath;
}

void GLEngine::preload(const char* json) {
	context->loader->setCallbacks(onStatusCallback, onFileCallback, onErrorCallback, onRetryCallback, onPreloadCallback, onFatalCallback);
	context->loader->load(json);
}

void GLEngine::load(const char* json) {
	context->loader->setCallbacks(onStatusCallback, onFileCallback, onErrorCallback, onRetryCallback, onLoadCallback, onFatalCallback);
	context->loader->load(json);
}

void GLEngine::minimize(bool minimize, bool animate) {
	minimized = minimize;
	if (onminimize != NULL)
		onminimize(minimize, animate, onminimizeUserData);
}

void GLEngine::setOnMinimize(onMinimizeCallback onminimize, void* userData) {
	this->onminimize = onminimize;
	onminimizeUserData = userData;
}

void GLEngine::onRender() {
	this->context->frame->loop();
}

void GLEngine::onUpdate(int width, int height) {
	onResize(width, height, minimized);
	context->frame->width = width;
	context->frame->height = height;
	context->updateRatio(width, height);
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
