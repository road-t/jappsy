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

#include "uGLObject.h"
#include <opengl/uGLScene.h>
#include <opengl/uGLRender.h>
#include <core/uMemory.h>
#include <opengl/uGLModel.h>
#include <opengl/uGLParticle.h>
#include <opengl/uGLDrawing.h>

GLObject::GLObject(GLScene* scene, const CString& key) {
	this->scene = scene;
	this->key = key;
	
	modelMatrix.identity();
	objectMatrix.identity();
}

GLObject::~GLObject() {
}

GLObject* GLObject::setModel(const CString& key) throw(const char*) {
	object = scene->context->models->get(key);
	objectType = OBJECT_MODEL;
	return this;
}

GLObject* GLObject::setFunc(GLFunc::onFuncCallback callback) throw(const char*) {
	object = scene->context->funcs->createFunc(key, callback);
	objectType = OBJECT_FUNC;
	return this;
}

GLObject* GLObject::setParticleSystem(const CString& key) throw(const char*) {
	object = scene->context->particles->get(key);
	objectType = OBJECT_PARTICLE;
	return this;
}

GLObject* GLObject::setDrawing(const CString& key, const GLfloat time) throw(const char*) {
	object = scene->context->drawings->get(key);
	this->time = time;
	objectType = OBJECT_DRAWING;
	return this;
}

bool GLObject::eventHandler(const CString& event, const GLTouchPoint* cur, const GLTouchPoint* delta, const GLTouchPoint* speed, void* userData) {
	GLObject* object= (GLObject*)userData;
	if (object->visible) {
		return object->onevent(object->scene->context->engine, event, (GLDrawing*)(object->object));
	}
	return false;
}

void GLObject::trackEvent(onEventCallback callback) {
	if (objectType == OBJECT_DRAWING) {
		GLDrawing* drawing = (GLDrawing*)object;
		Vec2 pos = drawing->sprite->getPosition(drawing->position, drawing->paint);
		GLfloat x = pos[0];
		GLfloat y = pos[1];
		GLfloat w = drawing->sprite->width;
		GLfloat h = drawing->sprite->height;
		onevent = callback;
		scene->context->touchScreen->clickEvent(key, x, y, w, h, eventHandler, this);
		scene->context->touchScreen->trackEvent(key, x, y, w, h, eventHandler, this);
	}
}

void GLObject::render() {
	if ((visible) && (object != NULL)) {
		object->render(this, time);
	}
}

GLObjects::GLObjects(GLScene* scene) throw(const char*) {
	this->scene = scene;
}

GLObjects::~GLObjects() {
	int32_t count = list.count();
	GLObject** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLObject* GLObjects::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLObject* GLObjects::createObject(const CString& key) throw(const char*) {
	try {
		list.removedelete(key);
		GLObject* object = new GLObject(scene, key);
		try {
			list.put(key, object);
		} catch (...) {
			delete object;
			throw;
		}
		return object;
	} catch (...) {
		throw;
	}
}

void GLObjects::trackEvents(const Vector<GLObject*>& group, GLObject::onEventCallback onevent) {
	int32_t count = group.count();
	GLObject** items = group.items();
	for (int i = 0; i < count; i++) {
		items[i]->trackEvent(onevent);
	}
}
