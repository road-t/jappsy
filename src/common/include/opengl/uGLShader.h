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
#include <data/uHashMap.h>
#include <data/uVector.h>
#include <opengl/uGLReader.h>

class GLRender;

enum GLShaderDataType { NONE, STRING, TEXTURES, SHADER };

class GLShaderData : public RefObject {
private:
	GLRender* m_context = NULL;
	
	GLShaderDataType m_type = GLShaderDataType::NONE;
	bool m_reference = false;

	String m_target;
	Vector<GLuint> m_handles;
	
public:
	inline GLShaderData() { throw eInvalidParams; }
	inline GLShaderData(GLRender* context) { THIS.m_context = context; }
	~GLShaderData();
	
	GLShaderData& setTarget(const String& target, bool reference);
	GLShaderData& setTextures(const Vector<GLuint>& handles, bool reference);
	GLShaderData& setShader(const GLuint handle, bool reference);
	
	inline bool isReference() { return m_type == GLShaderDataType::STRING; }
	inline const wchar_t* getTarget() { return (wchar_t*)m_target; }
	inline GLuint getShader() { return m_handles[0]; }
	inline Vector<GLuint>& getTextures() { return m_handles; }
};

class RefGLShader : public RefObject {
public:
	
	GLRender* context = NULL;
	GLShaderData* vsh;
	GLShaderData* fsh;
	GLuint program = 0;
	Vector<GLShaderData*> textures;
	Vector<GLuint> handles;
	Vector<GLint> handles1iv;
	
	inline RefGLShader() { throw eInvalidParams; }
	RefGLShader(GLRender* context, GLShaderData* vsh, GLShaderData* fsh, GLuint program, Vector<GLShaderData*>& textures) throw(const char*);
	~RefGLShader();
	
	bool checkReady();
	GLuint bind(GLint index, GLint uniform);
	GLuint bind(GLint index, Vector<GLint>& uniforms);
};

class GLShader : public Object {
public:
	RefClass(GLShader, RefGLShader)
	
	inline bool checkReady() throw(const char*) { return THIS.ref().checkReady(); }
	inline GLuint bind(GLint index, GLint uniform) throw(const char*) { return THIS.ref().bind(index, uniform); }
	inline GLuint bind(GLint index, Vector<GLint>& uniforms) throw(const char*) { return THIS.ref().bind(index, uniforms); }
};

class GLShaders {
private:
	GLRender* context;
	HashMap<String, GLShader> list;
	
public:
	GLShaders(GLRender* context) throw(const char*);
	~GLShaders();
	
	GLShader& get(const wchar_t* key) throw(const char*);
	GLShader& createShader(const wchar_t* key, GLShaderData* vsh, GLShaderData* fsh, GLuint program, Vector<GLShaderData*>& textures) throw(const char*);
	
	GLuint createVertexShader(const char* vertexShaderSource) throw(const char*);
	GLuint createFragmentShader(const char* fragmentShaderSource) throw(const char*);
	void releaseShader(GLuint shader);
	
	GLuint createProgram(GLuint vertexShader, GLuint fragmentShader) throw(const char*);
	void releaseProgram(GLuint program);

private: // Thread Safe
	static void* CreateVertexShaderCallback(void* threadData);
	static void* CreateFragmentShaderCallback(void* threadData);
	
public: // Thread Safe

	GLShader& createVertexShader(const wchar_t* key, const char* vertexShaderSource) throw(const char*);
	GLShader& createFragmentShader(const wchar_t* key, const char* fragmentShaderSource) throw(const char*);

};

#endif //JAPPSY_UGLSHADER_H