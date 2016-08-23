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

#include "uGLLight.h"
#include <core/uMemory.h>

GLLight::GLLight(GLScene* scene, const CString& key) {
	this->scene = scene;
	this->key = key;
}

GLLight* GLLight::omni(const Vec3& position, const Vec3& color, const GLfloat intensivity, const GLfloat radius, const GLfloat falloff, const bool fixed) {
	this->position.set(position);
	this->target.set(0.0);
	this->color.set(color);
	this->intensivity = intensivity;
	this->hotspot = radius;
	this->falloff = falloff;
	this->fixed = fixed;
	this->style = GLLightStyle::OMNI;
	this->invalid = true;
	return this;
}

GLLight* GLLight::spot(const Vec3& position, const Vec3& target, const Vec3& color, const GLfloat intensivity, const GLfloat angle, const GLfloat falloff, const bool fixed) {
	this->position.set(position);
	this->target.set(target);
	this->color.set(color);
	this->intensivity = intensivity;
	this->hotspot = angle;
	this->falloff = falloff;
	this->fixed = fixed;
	this->style = GLLightStyle::SPOT;
	this->invalid = true;
	return this;
}

GLLight* GLLight::direct(const Vec3& position, const Vec3& target, const Vec3& color, const GLfloat intensivity, const GLfloat radius, const GLfloat falloff, const bool fixed) {
	this->position.set(position);
	this->target.set(target);
	this->color.set(color);
	this->intensivity = intensivity;
	this->hotspot = radius;
	this->falloff = falloff;
	this->fixed = fixed;
	this->style = GLLightStyle::DIRECT;
	this->invalid = true;
	return this;
}

#include <math.h>

bool GLLight::update() {
	if (invalid) {
		if (fixed) {
			position3fv.set(position);
			target3fv.set(target);
		} else {
			position3fv.transform(position, scene->camera->view16fv);
			target3fv.transform(target, scene->camera->view16fv);
		}
		
		light16fv[0] = position3fv[0];
		light16fv[1] = position3fv[1];
		light16fv[2] = position3fv[2];
		light16fv[8] = color[0] * intensivity;
		light16fv[9] = color[1] * intensivity;
		light16fv[10] = color[2] * intensivity;
		
		if (style == GLLightStyle::OMNI) {
			light16fv[3] = hotspot;
			light16fv[4] = light16fv[5] = light16fv[6] = 0;
			light16fv[7] = falloff;
			light16fv[11] = 0;
		} else {
			light16fv[4] = target3fv[0];
			light16fv[5] = target3fv[1];
			light16fv[6] = target3fv[2];
			
			if (style == GLLightStyle::SPOT) {
				light16fv[3] = hotspot * M_PI / 360.0;
				light16fv[7] = falloff * M_PI / 360.0;
				light16fv[11] = 1;
			} else {
				light16fv[3] = hotspot;
				light16fv[7] = falloff;
				light16fv[11] = 2;
			}
		}
		light16fv[12] = light16fv[13] = light16fv[14] = light16fv[15] = 0;
		
		invalid = false;
		return true;
	}
	
	return false;
}

GLLights::GLLights(GLScene* scene) throw(const char*) {
	this->scene = scene;
}

GLLights::~GLLights() {
	int32_t count = list.count();
	GLLight** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLLight* GLLights::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLLight* GLLights::createLight(const CString& key) throw(const char*) {
	try {
		list.removedelete(key);
		GLLight* light = new GLLight(scene, key);
		try {
			list.put(key, light);
		} catch (...) {
			delete light;
			throw;
		}
		return light;
	} catch (...) {
		throw;
	}
}
