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

#include "uGLParticle.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>

GLParticle::GLParticle() {
}

GLParticle::~GLParticle() {
}

GLParticleSystem::GLParticleSystem(GLRender* context, const CString& key) {
	this->context = context;
	this->key = key;
	color.random(colorRange);
}

GLParticleSystem::~GLParticleSystem() {
	int32_t count = list.count();
	GLParticle** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
	
	if (indexCount != 0) {
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
		glDeleteBuffers(1, &vertexBuffer);
		
		glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
		glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
		glDeleteBuffers(1, &textureBuffer);
		
		glBindBuffer(GL_ARRAY_BUFFER, velocityBuffer);
		glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
		glDeleteBuffers(1, &velocityBuffer);
		
		glBindBuffer(GL_ARRAY_BUFFER, accelBuffer);
		glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
		glDeleteBuffers(1, &accelBuffer);
		
		glBindBuffer(GL_ARRAY_BUFFER, timeBuffer);
		glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
		glDeleteBuffers(1, &timeBuffer);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
		glDeleteBuffers(1, &indexBuffer);
	}
}

GLParticle* GLParticleSystem::createRocket(uint64_t startFrame, const Vec3& position) throw(const char*) {
	GLParticle* p = new GLParticle();
	p->position.set(position);
	p->velocity.random( {0.5, 0, 0.5} ).add( {0, 0.5, 0} ).multiply((GLfloat)rand() / (GLfloat)(RAND_MAX) * 2.0 + 4.0);
	p->acceleration.set( {0, -0.01, 0} );
	p->size = 2;
	p->startFrame = startFrame;
	p->lifeFrame = rand() % 50 + 50;
	p->style = L"rocket";
	list.push(p);
	
	uint64_t time = p->startFrame + p->lifeFrame;
	for (uint64_t i = 0; i < p->lifeFrame; i++) {
		if ((rand() % 4) >= 1) {
			Vec3 vV; vV.multiply(p->velocity, i);
			Vec3 vA; vA.multiply(p->acceleration, (GLfloat)(i * (i + 1)) / 2.0);
			Vec3 vP; vP.add(p->position, vV).add(vA);
			vV.multiply(p->acceleration, i).add(p->velocity);
			
			GLParticle* sub = createSubFlare(p->startFrame + i, vP, vV);
			uint64_t subtime = sub->startFrame + sub->lifeFrame;
			if (subtime > time) time = subtime;
		}
	}
	
	Vec3 vV; vV.multiply(p->velocity, p->lifeFrame);
	Vec3 vA; vA.multiply(p->acceleration, (GLfloat)(p->lifeFrame * (p->lifeFrame + 1)) / 2.0);
	Vec3 vP; vP.add(p->position, vV).add(vA);
	uint64_t c = rand() % 50 + 50;
	for (uint64_t i = 0; i < c; i++) {
		GLParticle* sub = createFlare(p->startFrame + p->lifeFrame, vP);
		uint64_t subtime = sub->startFrame + sub->lifeFrame;
		if (subtime > time) time = subtime;
	}
	
	if (maxFrame < time) maxFrame = time;
	
	return p;
}

GLParticle* GLParticleSystem::createFlare(uint64_t startFrame, const Vec3& position) throw(const char*) {
	GLParticle* p = new GLParticle();
	p->position.set(position);
	p->velocity.random( {0.5, 0.5, 0.5} ).multiply((GLfloat)rand() / (GLfloat)(RAND_MAX) * 2.0 + 1.0);
	p->acceleration.set( {0, -0.01, 0} );
	p->size = 5;
	p->startFrame = startFrame;
	p->lifeFrame = rand() % 100 + 10;
	p->style = L"flare";
	list.push(p);
	
	uint64_t time = p->startFrame + p->lifeFrame;
	for (uint64_t i = 0; i < p->lifeFrame; i++) {
		if ((rand() % 4) >= 3) {
			Vec3 vV; vV.multiply(p->velocity, i);
			Vec3 vA; vA.multiply(p->acceleration, (GLfloat)(i * (i + 1)) / 2.0);
			Vec3 vP; vP.add(p->position, vV).add(vA);
			vV.multiply(p->acceleration, i).add(p->velocity);
			
			GLParticle* sub = createSubFlare(p->startFrame + i, vP, vV);
			uint64_t subtime = sub->startFrame + sub->lifeFrame;
			if (subtime > time) time = subtime;
		}
	}
	
	if (maxFrame < time) maxFrame = time;
	
	return p;
}

GLParticle* GLParticleSystem::createSubFlare(uint64_t startFrame, const Vec3& position, const Vec3& velocity) throw(const char*) {
	GLParticle* p = new GLParticle();
	p->position.set(position);
	p->velocity.random( {0.5, 0.5, 0.5} ).multiply((GLfloat)rand() / (GLfloat)(RAND_MAX) + 1.0).add(velocity).multiply(0.25);
	p->acceleration.set( {0, -0.01, 0} );
	p->size = 2;
	p->startFrame = startFrame;
	p->lifeFrame = rand() % 50 + 10;
	p->style = L"subflare";
	list.push(p);
	
	uint64_t time = p->startFrame + p->lifeFrame;
	
	if (maxFrame < time) maxFrame = time;
	
	return p;
}

void GLParticleSystem::generate() throw(const char*) {
	Vector<Vec4> vertex;
	Vector<Vec2> texture;
	Vector<Vec4> velocity;
	Vector<Vec4> accel;
	Vector<Vec4> time;
	Vector<Vec3i> index;
	
	const int __UNUSED__ = 0;
	
	int32_t count = list.count();
	GLParticle** items = list.items();
	for (int i = 0; i < count; i++) {
		GLParticle* p = items[i];
		for (int j = 0; j < 4; j++) {
			vertex.push( {p->position[0], p->position[1], p->position[2], (GLfloat)j} );
			velocity.push( {p->velocity[0], p->velocity[1], p->velocity[2], p->size} );
			accel.push( {p->acceleration[0], p->acceleration[1], p->acceleration[2], __UNUSED__} );
			time.push( {(GLfloat)p->startFrame, (GLfloat)p->lifeFrame, __UNUSED__, __UNUSED__} );
		}
		texture.push( {0,0} );
		texture.push( {0,1} );
		texture.push( {1,1} );
		texture.push( {1,0} );
		GLshort k = i*4;
		index.push( {k, (GLshort)(k+1), (GLshort)(k+2)} );
		index.push( {k, (GLshort)(k+2), (GLshort)(k+3)} );
	}
	
	try {
		glGenBuffers(1, &vertexBuffer);
		CheckGLError();
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertex.count() * sizeof(Vec4), vertex.items(), GL_STATIC_DRAW);
		CheckGLError();
	
		glGenBuffers(1, &textureBuffer);
		CheckGLError();
		glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
		glBufferData(GL_ARRAY_BUFFER, texture.count() * sizeof(Vec2), texture.items(), GL_STATIC_DRAW);
		CheckGLError();
	
		glGenBuffers(1, &velocityBuffer);
		CheckGLError();
		glBindBuffer(GL_ARRAY_BUFFER, velocityBuffer);
		glBufferData(GL_ARRAY_BUFFER, velocity.count() * sizeof(Vec4), velocity.items(), GL_STATIC_DRAW);
		CheckGLError();
	
		glGenBuffers(1, &accelBuffer);
		CheckGLError();
		glBindBuffer(GL_ARRAY_BUFFER, accelBuffer);
		glBufferData(GL_ARRAY_BUFFER, accel.count() * sizeof(Vec4), accel.items(), GL_STATIC_DRAW);
		CheckGLError();
	
		glGenBuffers(1, &timeBuffer);
		CheckGLError();
		glBindBuffer(GL_ARRAY_BUFFER, timeBuffer);
		glBufferData(GL_ARRAY_BUFFER, time.count() * sizeof(Vec4), time.items(), GL_STATIC_DRAW);
		CheckGLError();
	
		glGenBuffers(1, &indexBuffer);
		CheckGLError();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.count() * sizeof(Vec3i), index.items(), GL_STATIC_DRAW);
		CheckGLError();
		indexCount = index.count() * 3;
	} catch (...) {
		if (vertexBuffer != 0) {
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
			glDeleteBuffers(1, &vertexBuffer);
			vertexBuffer = 0;
		}
		
		if (textureBuffer != 0) {
			glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
			glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
			glDeleteBuffers(1, &textureBuffer);
			textureBuffer = 0;
		}
		
		if (velocityBuffer != 0) {
			glBindBuffer(GL_ARRAY_BUFFER, velocityBuffer);
			glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
			glDeleteBuffers(1, &velocityBuffer);
			velocityBuffer = 0;
		}
		
		if (accelBuffer != 0) {
			glBindBuffer(GL_ARRAY_BUFFER, accelBuffer);
			glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
			glDeleteBuffers(1, &accelBuffer);
			accelBuffer = 0;
		}
		
		if (timeBuffer != 0) {
			glBindBuffer(GL_ARRAY_BUFFER, timeBuffer);
			glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
			glDeleteBuffers(1, &timeBuffer);
			timeBuffer = 0;
		}
		
		if (indexBuffer != 0) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
			glDeleteBuffers(1, &indexBuffer);
			indexBuffer = 0;
		}
		
		indexCount = 0;
		
		throw;
	}
}

void GLParticleSystem::render(GLObject* object, const GLfloat time, GLCamera* camera) {
	if (startFrame == 0)
		return;
	
	uint64_t currentFrame = ((currentTimeMillis() / 10) * 6) / 10;
	if ((startFrame + maxFrame) < currentFrame) {
		if (repeat == 0) {
			startFrame = 0;
			return;
		}
		
		if (repeat > 0)
			repeat--;
		startFrame = currentFrame + rand() % startFrameRange;
		color.random(colorRange);
	}
	
	if (currentFrame < startFrame)
		return;
	
	GLScene* scene = object->scene;
	GLParticleShader* shader = context->shaderParticle;
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Искры
	//gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA); // Капли краски
	//gl.blendFunc(gl.SRC_COLOR, gl.ONE_MINUS_SRC_COLOR); // Пузыри
	//gl.blendFunc(gl.ONE_MINUS_DST_COLOR, gl.ONE_MINUS_SRC_COLOR); // Хлопья / Конфети
	//gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA); gl.blendEquation(gl.FUNC_REVERSE_SUBTRACT);
	
	GLuint index = shader->shader->bind(0);
	
	Vec3 vCam; vCam.subtract(scene->camera->position, scene->camera->target).normalize();
	Vec3 vX; vX.cross(vCam, scene->camera->head).normalize();
	Vec3 vY; vY.cross(vX, vCam).normalize();
	
	scene->modelView16fv.multiply(scene->camera->view16fv, object->objectMatrix).multiply(object->modelMatrix);
	scene->modelViewProjection16fv.multiply(scene->camera->projection16fv, scene->modelView16fv);
	
	glUniformMatrix4fv(shader->uModelViewProjectionMatrix, 1, GL_FALSE, scene->modelViewProjection16fv);
	
	glEnableVertexAttribArray(shader->aVertexPosition);
	glEnableVertexAttribArray(shader->aTextureCoord);
	glEnableVertexAttribArray(shader->aVelocity);
	glEnableVertexAttribArray(shader->aAcceleration);
	glEnableVertexAttribArray(shader->aTime);
	
	glUniform3fv(shader->uPixelX, 1, vX.v);
	glUniform3fv(shader->uPixelY, 1, vY.v);
	
	{
		Vec3 time = { (GLfloat)(currentFrame - startFrame), (GLfloat)maxFrame, 0 };
		glUniform3fv(shader->uTime, 1, time.v);
		glUniform3fv(shader->uColor, 1, color.v);
		
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(shader->aVertexPosition, 4, GL_FLOAT, GL_FALSE, 0, NULL);
		
		glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
		glVertexAttribPointer(shader->aTextureCoord, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		
		glBindBuffer(GL_ARRAY_BUFFER, velocityBuffer);
		glVertexAttribPointer(shader->aVelocity, 4, GL_FLOAT, GL_FALSE, 0, NULL);
		
		glBindBuffer(GL_ARRAY_BUFFER, accelBuffer);
		glVertexAttribPointer(shader->aAcceleration, 4, GL_FLOAT, GL_FALSE, 0, NULL);
		
		glBindBuffer(GL_ARRAY_BUFFER, timeBuffer);
		glVertexAttribPointer(shader->aTime, 4, GL_FLOAT, GL_FALSE, 0, NULL);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);
	}
	
	glDisableVertexAttribArray(shader->aVertexPosition);
	glDisableVertexAttribArray(shader->aTextureCoord);
	glDisableVertexAttribArray(shader->aVelocity);
	glDisableVertexAttribArray(shader->aAcceleration);
	glDisableVertexAttribArray(shader->aTime);
	
	context->cleanup(index);
	context->resetBlend();
}

GLParticles::GLParticles(GLRender* context) throw(const char*) {
	this->context = context;
}

GLParticles::~GLParticles() {
	int32_t count = list.count();
	GLParticleSystem** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLParticleSystem* GLParticles::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLParticleSystem* GLParticles::createParticleSystem(const CString& key) throw(const char*) {
	try {
		list.removedelete(key);
		GLParticleSystem* particle = new GLParticleSystem(context, key);
		try {
			list.put(key, particle);
		} catch (...) {
			delete particle;
			throw;
		}
		return particle;
	} catch (...) {
		throw;
	}
}
