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

#ifndef JAPPSY_UGLCONTEXT_H
#define JAPPSY_UGLCONTEXT_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uVector.h>

#include <opengl/core/uGLTexture.h>
#include <opengl/core/uGLFramebuffer.h>

class GLContext : public CObject {
	friend class GLTexture;
	
public:
	GLContext();
	~GLContext();
	
	void reset();
	
	/** OpenGL Textures **/
	
private:
	GLuint activeTextureIndex = 0;
	GLTexture* attachedTextures[32] = {0};

public:
	GLuint attachTexture(GLTexture& texture, GLuint index = 0, GLint uniform = -1) throw(const char*);
	void detachTexture(GLTexture& texture) throw(const char*);
	bool reattachTexture(GLTexture& texture) throw(const char*);
	
private:
	GLuint attachTexture(GLuint handle);
	
	void detachTexture(GLuint index);
	
	/** OpenGL Framebuffers **/
	
private:
	GLFrameBuffer* defaultFrameBuffer = NULL;
	GLFrameBuffer* currentFrameBuffer = NULL;
	
public:
	void attachFramebuffer(GLFrameBuffer& buffer) throw(const char*);
	void detachFramebuffer() throw(const char*);
	bool isAttachedFramebuffer(GLFrameBuffer& buffer);
};

#endif //JAPPSY_UGLCONTEXT_H
