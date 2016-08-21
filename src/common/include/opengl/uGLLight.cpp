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

RefGLLight::RefGLLight(GLScene& scene) {
	THIS.scene = scene;
}

RefGLLight::~RefGLLight() {
	THIS.scene = null;
}

RefGLLight& RefGLLight::omni(const Vec3& position, const Vec3& color, const GLfloat intensivity, const GLfloat radius, const GLfloat falloff, const bool fixed) {
	THIS.position.set(position);
	THIS.target.set(0.0);
	THIS.color.set(color);
	THIS.intensivity = intensivity;
	THIS.hotspot = radius;
	THIS.falloff = falloff;
	THIS.fixed = fixed;
	THIS.style = GLLightStyle::OMNI;
	THIS.invalid = true;
	return *this;
}

RefGLLight& RefGLLight::spot(const Vec3& position, const Vec3& target, const Vec3& color, const GLfloat intensivity, const GLfloat angle, const GLfloat falloff, const bool fixed) {
	THIS.position.set(position);
	THIS.target.set(target);
	THIS.color.set(color);
	THIS.intensivity = intensivity;
	THIS.hotspot = angle;
	THIS.falloff = falloff;
	THIS.fixed = fixed;
	THIS.style = GLLightStyle::SPOT;
	THIS.invalid = true;
	return *this;
}

RefGLLight& RefGLLight::direct(const Vec3& position, const Vec3& target, const Vec3& color, const GLfloat intensivity, const GLfloat radius, const GLfloat falloff, const bool fixed) {
	THIS.position.set(position);
	THIS.target.set(target);
	THIS.color.set(color);
	THIS.intensivity = intensivity;
	THIS.hotspot = radius;
	THIS.falloff = falloff;
	THIS.fixed = fixed;
	THIS.style = GLLightStyle::DIRECT;
	THIS.invalid = true;
	return *this;
}

void RefGLLight::update() {
	if (invalid) {
		if (fixed) {
			position3fv.set(position);
			target3fv.set(target);
		} else {
			position3fv.transform(position, scene.ref().camera.ref().view16fv);
		}
	}
}


GLLights::GLLights(RefGLScene* scene) throw(const char*) {
	THIS.scene = scene;
	list = new JHashMap<JString, GLLight>();
}

GLLights::~GLLights() {
	list = null;
	scene = null;
}

GLLight& GLLights::get(const JString& key) throw(const char*) {
	return (GLLight&)list.get(key);
}

GLLight& GLLights::create(const JString& key) throw(const char*) {
	try {
		list.remove(key);
		GLLight* light = &(list.put(key, new RefGLLight(scene)));
		return *light;
	} catch (...) {
		throw;
	}
}
