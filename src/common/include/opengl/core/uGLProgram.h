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

#ifndef JAPPSY_UGLPROGRAM_H
#define JAPPSY_UGLPROGRAM_H

#include <opengl/core/uGLContextState.h>

struct GLProgramVariable {
	const char* name;
	const GLuint* target;
};

class GLProgram : public CObject {
	friend struct GLContextState;

	friend class GLObjectData;
	friend class GLReader;
	friend class GLShader;
	friend class GLShaders;
	
protected:
	static GLuint createVertexShader(const char* vertexShaderSource) throw(const char*);
	static GLuint createFragmentShader(const char* fragmentShaderSource) throw(const char*);
	static void releaseShader(GLuint shader);
	
	static GLuint createProgram(GLuint vertexShader, GLuint fragmentShader) throw(const char*);
	static void releaseProgram(GLuint program);

protected:
	GLContext* context = NULL;
	
	GLuint vertexShader = GL_NONE;
	GLuint fragmentShader = GL_NONE;
	GLuint handle = GL_NONE;
	
public:
	GLProgram(GLContext& context, const char* vsh, const char* fsh, const GLProgramVariable* attributes = NULL, const GLProgramVariable* uniforms = NULL) throw(const char*);
	~GLProgram();
	
protected:
	void release();
};

class GLProgramTextureVariables {
protected:
	GLuint uLayerProjectionMatrix;
	GLuint aVertexPosition;
	GLuint aTextureCoord;
	GLuint uSize;
	GLuint uPosition;
	GLuint uTexture;
	
	const GLProgramVariable attributes[3];
	const GLProgramVariable uniforms[5];
	
	GLProgramTextureVariables();
};

class GLProgramTexture : private GLProgramTextureVariables, public GLProgram {
public:
	GLProgramTexture(GLContext& context);
	
	void render(GLTexture& texture, const Mat4& projection, const Vec2& size, const Vec2& position);
};

#endif
