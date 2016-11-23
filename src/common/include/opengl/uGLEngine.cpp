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
	context->mainFrameBuffer->setOnPrepareRectCallback((onRectCallback)onPrepareRectCallback, this);
	context->mainFrameBuffer->setOnUpdateRectCallback((onRectCallback)onUpdateRectCallback, this);

#ifdef DEMO
	testFrameBuffer = new GLFrameBuffer(*context, 100, 100, GLRepeatNone | GLSmooth, NULL);
	testFrameBuffer->setOnUpdateRectCallback((onRectCallback)onTestUpdateRectCallback, this);
	
	testTexture = new GLTexture(*context, 32, 20, GLRepeatNone | GLSmooth, NULL);
#endif
}

#if defined(__JNI__)

struct ReleaseGLEngineThreadData {
	void* onUpdateStateUserData;
	void* onwebUserData;
};

void* onReleaseGLEngineThread(void* userData) {
	struct ReleaseGLEngineThreadData* thread = (struct ReleaseGLEngineThreadData*)userData;

	JNIEnv* env = GetThreadEnv();

	if (thread->onUpdateStateUserData != NULL) {
		env->DeleteGlobalRef((jobject)thread->onUpdateStateUserData);
	}

	if (thread->onwebUserData != NULL) {
		env->DeleteGlobalRef((jobject)thread->onwebUserData);
	}

	ReleaseThreadEnv();

	return NULL;
}

#endif

GLEngine::~GLEngine() {
#ifdef DEMO
	delete testTexture;
	delete testFrameBuffer;
#endif
	
	if (context != NULL) {
		delete context;
	}
	if (cache != NULL) {
		delete cache;
	}
#if defined(__JNI__)
	struct ReleaseGLEngineThreadData thread;
	thread.onUpdateStateUserData = onUpdateStateUserData;
	thread.onwebUserData = onwebUserData;
	MainThreadSync(onReleaseGLEngineThread, &thread);
#endif
}

void GLEngine::shutdown() {
	context->loader->release();

	LOG("GLEngine::Shutdown");

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

void GLEngine::mixerVolume(GLfloat volume) {
	LOG("Volume: %d%%", (int)roundf(volume * 100.0f));
	context->mixer->mixMaxVolume(volume, 1.0f);
}

bool GLEngine::mixerPlaying() {
	bool result = context->mixer->mixPlaying();
	LOG("Mixer: %s", (result ? "Playing" : "Stopped"));
	return result;
}

void GLEngine::updateState(int state) {
	if (onUpdateState != NULL)
		onUpdateState(state, onUpdateStateUserData);
}

void GLEngine::setOnUpdateState(onUpdateStateCallback onUpdateState, void* userData) {
	this->onUpdateState = onUpdateState;
	onUpdateStateUserData = userData;
}

void GLEngine::webLocation(int index, const CString& url) {
	if (onweblocation) {
		onweblocation(index, url, onwebUserData);
	}
}

void GLEngine::webScript(int index, const CString& script) {
	if (onwebscript) {
		onwebscript(index, script, onwebUserData);
	}
}

void GLEngine::setWebCallbacks(onWebLocationCallback onweblocation, onWebScriptCallback onwebscript, void* userData) {
	this->onweblocation = onweblocation;
	this->onwebscript = onwebscript;
	onwebUserData = userData;
}

#ifdef DEMO
void GLEngine::onTestUpdateRectCallback(GLFrameBuffer* target, const GLRect& rect, GLEngine* engine) {
	glClearColor(1.0f, 0.5f, 0.0f, 0.7f);
	glClear(GL_COLOR_BUFFER_BIT);

	engine->context->programTexture->render(*engine->testTexture, target->projection16fv, Vec2(64, 64), Vec2(10, 10));
}

int testHeight = 0;
#endif

void GLEngine::onPrepareRectCallback(GLFrameBuffer* target, const GLRect& rect, GLEngine* engine) {
	// Prepare Textures and Framebuffers for render

#ifdef DEMO
	testHeight++;
	if (testHeight == 100) testHeight = 0;
	
	engine->testFrameBuffer->resize(100, -(100 + abs(testHeight - 50)));
	engine->testFrameBuffer->dirty();
	engine->testFrameBuffer->update();
#endif
}

void GLEngine::onUpdateRectCallback(GLFrameBuffer* target, const GLRect& rect, GLEngine* engine) {
	// Render rect
	engine->context->frame->loop();
#ifdef DEMO
	engine->context->programTexture->render(*engine->testFrameBuffer, target->projection16fv, Vec2(100, 100), Vec2(10, 10));
	engine->context->programTexture->render(*engine->testTexture, target->projection16fv, Vec2(100, 100), Vec2(60, 60));
#endif
}

bool GLEngine::onRender() {
	// Temporary dirty each frame
	context->mainFrameBuffer->dirty();
	return context->mainFrameBuffer->update();
}

void GLEngine::onUpdate(int width, int height) {
	context->resize(width, height);
	onResize(width, height);
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
