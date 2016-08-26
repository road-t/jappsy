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

#ifndef JAPPSY_UGLPARTICLE_H
#define JAPPSY_UGLPARTICLE_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uVector.h>
#include <opengl/uGLSceneObject.h>

class GLRender;

class GLParticle : public CObject {
public:
	CString style;
	
	Vec3 position = { 0, 0, 0 };
	Vec3 velocity = { 0, 0, 0 };
	Vec3 acceleration = { 0, 0, 0 };
	GLfloat size = 1.0;
	uint64_t startFrame = 0;
	uint64_t lifeFrame = 0;
	
	GLParticle();
	~GLParticle();
};

class GLParticleSystem : public GLSceneObject {
public:
	GLRender* context = NULL;
	CString key;
	Vector<GLParticle*> list;
	
	uint64_t startFrame = 0;
	uint64_t maxFrame = 0;
	int32_t repeat = -1;
	uint64_t startFrameRange = 1;
	
	Vec3 color = { 1.0, 1.0, 1.0 };
	Vec3 colorRange = { -0.5, -0.5, -0.5 };
	
	GLuint vertexBuffer = 0;
	GLuint textureBuffer = 0;
	GLuint velocityBuffer = 0;
	GLuint accelBuffer = 0;
	GLuint timeBuffer = 0;
	GLuint indexBuffer = 0;
	GLuint indexCount = 0;
	
	GLParticleSystem(GLRender* context, const CString& key);
	~GLParticleSystem();
	
	GLParticle* createRocket(uint64_t startFrame, const Vec3& position) throw(const char*);
	GLParticle* createFlare(uint64_t startFrame, const Vec3& position) throw(const char*);
	GLParticle* createSubFlare(uint64_t startFrame, const Vec3& position, const Vec3& velocity) throw(const char*);
	
	void generate() throw(const char*);
	void render(GLObject* object, const GLfloat time = NAN);
};

class GLParticles : public CObject {
private:
	GLRender* context;
	VectorMap<CString&, GLParticleSystem*> list;
	
public:
	GLParticles(GLRender* context) throw(const char*);
	~GLParticles();
	
	GLParticleSystem* get(const CString& key) throw(const char*);
	GLParticleSystem* createParticleSystem(const CString& key) throw(const char*);
};

#endif //JAPPSY_UGLPARTICLE_H