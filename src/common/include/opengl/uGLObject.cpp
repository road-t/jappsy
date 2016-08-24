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
#include <opengl/uGLFunc.h>
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
	return this;
}

GLObject* GLObject::setFunc(const CString& key) throw(const char*) {
	//object.func = new GLFunc(
	throw eOK;
}

GLObject* GLObject::setParticleSystem(const CString& key) throw(const char*) {
	object = scene->context->particles->get(key);
	return this;
}

GLObject* GLObject::setDrawing(const CString& key, const GLfloat* time) throw(const char*) {
	object = scene->context->drawings->get(key);
	this->time = time;
	return this;
}

void GLObject::render() {
	/*
	if (visible) {
		if (object.model != NULL) {
			object.model->render(this);
		} else if (object.func != NULL) {
			object.func->render(this);
		} else if (object.particle != NULL) {
			object.particle->render(this);
		} else if (object.)
	}
	 */
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
