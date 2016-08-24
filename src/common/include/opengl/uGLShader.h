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

#ifndef JAPPSY_UGLSHADER_H
#define JAPPSY_UGLSHADER_H

#include <opengl/uOpenGL.h>
#include <data/uObject.h>
#include <data/uString.h>
#include <data/uVector.h>
#include <opengl/uGLObjectData.h>

class GLRender;

class GLShader : public CObject {
public:
	GLRender* context = NULL;
	
	GLObjectData* vsh;
	GLObjectData* fsh;
	GLuint program = 0;
	Vector<GLObjectData*> textures;
	Vector<GLuint> handles;
	Vector<GLint> handles1iv;
	
	GLShader(GLRender* context, GLObjectData* vsh, GLObjectData* fsh, GLuint program, Vector<GLObjectData*>& textures) throw(const char*);
	~GLShader();
	
	bool checkReady();
	GLuint bind(GLint index, GLint uniform = -1);
	GLuint bind(GLint index, Vector<GLint>& uniforms);
};

class GLShaders : public CObject {
public:
	GLRender* context = NULL;
	VectorMap<CString&, GLShader*> list;

public:
	GLShaders(GLRender* context) throw(const char*);
	~GLShaders();
	
	GLShader* get(const CString& key) throw(const char*);
	GLShader* createShader(const CString& key, GLObjectData* vsh, GLObjectData* fsh, GLuint program, Vector<GLObjectData*>& textures) throw(const char*);
	
	GLuint createVertexShader(const char* vertexShaderSource) throw(const char*);
	GLuint createFragmentShader(const char* fragmentShaderSource) throw(const char*);
	void releaseShader(GLuint shader);
	
	GLuint createProgram(GLuint vertexShader, GLuint fragmentShader) throw(const char*);
	void releaseProgram(GLuint program);

private: // Thread Safe
	static void* CreateVertexShaderCallback(void* threadData);
	static void* CreateFragmentShaderCallback(void* threadData);
	
public: // Thread Safe

	GLShader* createVertexShader(const CString& key, const char* vertexShaderSource) throw(const char*);
	GLShader* createFragmentShader(const CString& key, const char* fragmentShaderSource) throw(const char*);
	GLShader* createShader(const CString& key, const wchar_t* vshReference, const wchar_t* fshReference, void* library = NULL) throw(const char*);
};

#endif //JAPPSY_UGLSHADER_H