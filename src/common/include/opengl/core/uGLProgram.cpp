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

#include "uGLProgram.h"
#include "uGLContext.h"

const static char saVertexPosition[] = "aVertexPosition";
const static char saTextureCoord[] = "aTextureCoord";
const static char suLayerProjectionMatrix[] = "uLayerProjectionMatrix";
const static char suSize[] = "uSize";
const static char suPosition[] = "uPosition";
const static char suTexture[] = "uTexture";

GLuint GLProgram::createVertexShader(const char* vertexShaderSource) throw(const char*) {
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	CheckGLError();
	
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	
	GLint status = GL_FALSE;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
#ifdef DEBUG
		GLint logLen = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0) {
			GLchar* log = memAlloc(GLchar, log, (size_t)logLen + 1);
			glGetShaderInfoLog(vertexShader, logLen, &logLen, log);
			CString::format(L"OpenGL Vertex Shader Log:\r\n%s", (char*)log).log();
			memFree(log);
		}
#endif
		glDeleteShader(vertexShader);
		throw eOpenGL;
	}
	
	return vertexShader;
}

GLuint GLProgram::createFragmentShader(const char* fragmentShaderSource) throw(const char*) {
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	CheckGLError();
	
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	
	GLint status = GL_FALSE;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
#ifdef DEBUG
		GLint logLen = 0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0) {
			GLchar* log = memAlloc(GLchar, log, (size_t)logLen + 1);
			glGetShaderInfoLog(fragmentShader, logLen, &logLen, log);
			CString::format(L"OpenGL Fragment Shader Log:\r\n%s", (char*)log).log();
			memFree(log);
		}
#endif
		glDeleteShader(fragmentShader);
		throw eOpenGL;
	}
	
	return fragmentShader;
}

void GLProgram::releaseShader(GLuint shader) {
	glDeleteShader(shader);
}

GLuint GLProgram::createProgram(GLuint vertexShader, GLuint fragmentShader) throw(const char*) {
	GLuint program = glCreateProgram();
	CheckGLError();
	
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	
	GLint status = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
#ifdef DEBUG
		GLint logLen = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0) {
			GLchar* log = memAlloc(GLchar, log, (size_t)logLen + 1);
			glGetProgramInfoLog(program, logLen, &logLen, log);
			CString::format(L"OpenGL Program Log:\r\n%s", (char*)log).log();
			memFree(log);
		}
#endif
		glDeleteProgram(program);
		throw eOpenGL;
	}
	
	return program;
}

void GLProgram::releaseProgram(GLuint program) {
	glDeleteProgram(program);
}

GLProgram::GLProgram(GLContext& context, const char* vsh, const char* fsh, const GLProgramVariable *attributes, const GLProgramVariable *uniforms) throw(const char*) {
	this->context = &context;

	try {
		vertexShader = createVertexShader(vsh);
		fragmentShader = createFragmentShader(fsh);
		handle = createProgram(vertexShader, fragmentShader);
	} catch (...) {
		release();
		throw;
	}
	
	if (attributes != NULL) {
		while (attributes->target != NULL) {
			*((GLuint*)(attributes->target)) = (GLuint)glGetAttribLocation(handle, attributes->name);
			attributes++;
		}
	}

	if (uniforms != NULL) {
		while (uniforms->target != NULL) {
			*((GLuint*)(uniforms->target)) = (GLuint)glGetUniformLocation(handle, uniforms->name);
			uniforms++;
		}
	}
}

GLProgram::~GLProgram() {
	release();
}

void GLProgram::release() {
	if (handle != GL_NONE) {
		releaseProgram(handle);
		handle = GL_NONE;
	}
	
	if (fragmentShader != GL_NONE) {
		releaseShader(fragmentShader);
		fragmentShader = GL_NONE;
	}
	
	if (vertexShader != GL_NONE) {
		releaseShader(vertexShader);
		vertexShader = GL_NONE;
	}
}

static const char *GLProgramTexture_VSH =
	#include "programs/q0_vsh_texture.res"
;

static const char *GLProgramTexture_FSH =
	#include "programs/q0_fsh_texture.res"
;

GLProgramTextureVariables::GLProgramTextureVariables() :
attributes{
	{ saVertexPosition, &aVertexPosition },
	{ saTextureCoord, &aTextureCoord },
	{ NULL, NULL }
},
uniforms{
	{ suLayerProjectionMatrix, &uLayerProjectionMatrix },
	{ suSize, &uSize },
	{ suPosition, &uPosition },
	{ suTexture, &uTexture },
	{ NULL, NULL }
}
{}

GLProgramTexture::GLProgramTexture(GLContext& context) : GLProgram(context, GLProgramTexture_VSH, GLProgramTexture_FSH, attributes, uniforms) {
}

void GLProgramTexture::render(GLTexture& texture, const Mat4& projection, const Vec2& size, const Vec2& position) {
	context->stackBlend.push(context->state.blend);
	context->state.enableBlend();
	glUseProgram(handle);
	glUniformMatrix4fv(uLayerProjectionMatrix, 1, GL_FALSE, projection.v);
	glUniform2fv(uSize, 1, size.v);
	glUniform2fv(uPosition, 1, position.v);
	
	glBindBuffer(GL_ARRAY_BUFFER, context->rectArrayBuffers[texture.state & GLFlipMask]);
	glVertexAttribPointer(aVertexPosition, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, texture.rectArrayBuffer);
	glVertexAttribPointer(aTextureCoord, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(aVertexPosition);
	glEnableVertexAttribArray(aTextureCoord);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.handle);
	glUniform1i(uTexture, 0);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	
	glDisableVertexAttribArray(aTextureCoord);
	glDisableVertexAttribArray(aVertexPosition);
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
	
	glUseProgram(GL_NONE);
	context->state.setFrom(context->stackBlend.pop());
}
