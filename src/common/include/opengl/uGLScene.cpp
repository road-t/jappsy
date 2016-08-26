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

#include "uGLScene.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>
#include <opengl/uGLLight.h>
#include <opengl/uGLObject.h>

GLScene::GLScene(GLRender* context) {
	this->context = context;

	lights = new GLLights(this);
	objects = new GLObjects(this);
	
	lights16fvv = memAlloc(GLfloat, lights16fvv, context->lightsMaxCount * 16 * sizeof(GLfloat));
	modelViewProjection16fv.identity();
	modelView16fv.identity();
	normal16fv.identity();
}

GLScene::~GLScene() {
	delete objects;
	delete lights;
	
	int32_t count = layers.count();
	Vector<GLObject*>** items = layers.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
	
	memFree(lights16fvv);
}

GLScene* GLScene::rotateGroup(Vector<GLObject*>& group, const Vec3& vec, GLfloat angle, bool permanent) {
	if (permanent) {
		Mat4 rotate; rotate.rotate(vec, angle);
		int32_t count = group.count();
		GLObject** items = group.items();
		for (int i = 0; i < count; i++) {
			items[i]->objectMatrix.multiply(rotate);
		}
	} else {
		int32_t count = group.count();
		GLObject** items = group.items();
		for (int i = 0; i < count; i++) {
			items[i]->modelMatrix.rotate(vec, angle);
		}
	}
	return this;
}

GLScene* GLScene::visibleGroup(Vector<GLObject*>& group, bool visible) {
	int32_t count = group.count();
	GLObject** items = group.items();
	for (int i = 0; i < count; i++) {
		items[i]->visible = visible;
	}
	return this;
}

void GLScene::startParticlesGroup(Vector<GLObject*>& group, int repeat) {
	uint64_t currentFrame = ((currentTimeMillis() / 10) * 6) / 10;
	
	int32_t count = group.count();
	GLObject** items = group.items();
	for (int i = 0; i < count; i++) {
		if (items[i]->objectType == OBJECT_PARTICLE) {
			GLParticleSystem* s = (GLParticleSystem*)(items[i]->object);
			if (s->startFrame == 0) {
				s->startFrame = currentFrame + rand() % s->startFrameRange;
			}
			s->repeat = repeat;
			s->color.random(s->colorRange);
		}
	}
}

Vector<GLObject*>* GLScene::createLayer() throw(const char*) {
	Vector<GLObject*>* layer = new Vector<GLObject*>();
	try {
		layers.push(layer);
	} catch (...) {
		delete layer;
		throw;
	}
	return layer;
}

GLObject* GLScene::createLayerObject(Vector<GLObject*>* layer, const CString& key) throw(const char*) {
	GLObject* object = objects->createObject(key);
	layer->push(object);
	return object;
}

GLObject* GLScene::createLayerDrawing(Vector<GLObject*>* layer, const CString& key, const GLfloat time) throw(const char*) {
	GLObject* object = objects->createObject(key);
	layer->push(object);
	object->setDrawing(key, time);
	return object;
}

void GLScene::update() {
	bool invalidateLights = camera->update();
	
	lights1i = 0;
	int32_t count = lights->list.count();
	GLLight** items = lights->list.items();
	for (int i = 0; i < count; i++) {
		GLLight* light = items[i];
		if (light->active) {
			if (invalidateLights) light->invalidate();
			light->update();
			Mat4SetV(lights16fvv + 16 * lights1i, light->light16fv.v);
			lights1i++;
			if (lights1i >= context->lightsMaxCount) break;
		}
	}
}

void GLScene::render() {
	update();
	
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	
	int32_t count = layers.count();
	Vector<GLObject*>** items = layers.items();
	for (int i = 0; i < count; i++) {
		int32_t subcount = items[i]->count();
		GLObject** subitems = items[i]->items();
		for (int j = 0; j < subcount; j++) {
			subitems[j]->render();
		}
	}
}

GLScenes::GLScenes(GLRender* context) throw(const char*) {
	this->context = context;
}

GLScenes::~GLScenes() {
	int32_t count = list.count();
	GLScene** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLScene* GLScenes::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLScene* GLScenes::createScene(const CString& key) throw(const char*) {
	try {
		list.removedelete(key);
		GLScene* scene = new GLScene(context);
		try {
			list.put(key, scene);
		} catch (...) {
			delete scene;
			throw;
		}
		return scene;
	} catch (...) {
		throw;
	}
}
