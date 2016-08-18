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

#include "uGLShader.h"
#include <opengl/uGLRender.h>
#include <core/uMemory.h>

GLShaderData::~GLShaderData() {
	if (!m_reference) {
		if (m_type == GLShaderDataType::TEXTURES) {
			uint32_t count = m_handles.count();
			if (count > 0) {
				GLuint* items = m_handles.items();
				for (int i = 0; i < count; i++) {
					m_context->textures->releaseTextureHandle(items[i]);
				}
			}
		} else if (m_type == GLShaderDataType::SHADER) {
			m_context->shaders->releaseShader(m_handles[0]);
		}
	}
	m_target = null;
	m_handles.clear();
	m_reference = false;
	m_type = GLShaderDataType::NONE;
}

GLShaderData& GLShaderData::setTarget(const String& target, bool reference) {
	m_target = target;
	m_handles.clear();
	m_reference = reference;
	m_type = GLShaderDataType::STRING;
	return THIS;
}

GLShaderData& GLShaderData::setTextures(const Vector<GLuint>& handles, bool reference) {
	m_target = null;
	m_handles.clear();
	uint32_t count = handles.count();
	if (count > 0) {
		GLuint* items = handles.items();
		for (int i = 0; i < count; i++) {
			m_handles.push(items[i]);
		}
	}
	m_reference = reference;
	m_type = GLShaderDataType::TEXTURES;
	return THIS;
}

GLShaderData& GLShaderData::setShader(const GLuint handle, bool reference) {
	m_target = null;
	m_handles.clear();
	m_handles.push(handle);
	m_reference = reference;
	m_type = GLShaderDataType::SHADER;
	return THIS;
}

RefGLShader::RefGLShader(GLRender* context, GLShaderData* vsh, GLShaderData* fsh, GLuint program, Vector<GLShaderData*>& textures) throw(const char*) {
	THIS.context = context;
	THIS.vsh = vsh;
	THIS.fsh = fsh;
	THIS.program = program;

	uint32_t count = textures.count();
	if (count > 0) {
		THIS.textures.resize(count);
		memcpy(THIS.textures.items(), textures.items(), count * sizeof(GLShaderData*));
	}
}

RefGLShader::~RefGLShader() {
	uint32_t count = textures.count();
	if (count > 0) {
		for (int i = 0; i < count; i++) {
			GLShaderData* texture = textures[i];
			memDelete(texture);
		}
		textures.clear();
	}
	
	handles.clear();
	handles1iv.clear();
	
	if (program != 0) {
		context->shaders->releaseProgram(program);
		program = 0;
	}
	
	if (vsh != NULL) {
		memDelete(vsh);
		vsh = NULL;
	}
	
	if (fsh != NULL) {
		memDelete(fsh);
		fsh = NULL;
	}
	
	THIS.context = NULL;
}

bool RefGLShader::checkReady() {
	if ((THIS.program == 0) && (THIS.vsh != NULL) && (THIS.fsh != NULL)) {
		if (THIS.vsh->isReference()) {
			GLShaderData* vsh = THIS.vsh;
			do {
				GLShader* shader;
				try {
					shader = &(context->shaders->get(vsh->getTarget()));
					vsh = shader->ref().vsh;
				} catch (...) {
					vsh = NULL;
					break;
				}
			} while (vsh->isReference());
			if (vsh != NULL) {
				THIS.vsh->setShader(vsh->getShader(), true);
			} else {
				return false;
			}
		}

		if (THIS.fsh->isReference()) {
			GLShaderData* fsh = THIS.fsh;
			do {
				GLShader* shader;
				try {
					shader = &(context->shaders->get(fsh->getTarget()));
					fsh = shader->ref().fsh;
				} catch (...) {
					fsh = NULL;
					break;
				}
			} while (fsh->isReference());
			if (fsh != NULL) {
				THIS.fsh->setShader(fsh->getShader(), true);
			} else {
				return false;
			}
		}
		
		GLuint vertexShader = THIS.vsh->getShader();
		GLuint frameShader = THIS.fsh->getShader();
		
		try {
			THIS.program = context->shaders->createProgram(vertexShader, frameShader);
		} catch (...) {
			return false;
		}
	}
	
	uint32_t handlesCount = 0;
	uint32_t count = THIS.textures.count();
	if (count > 0) {
		GLShaderData** items = THIS.textures.items();
		for (int i = (int)(count-1); i >= 0; i--) {
			if (items[i]->isReference()) {
				GLTexture* texture;
				try {
					texture = &(context->textures->get(items[i]->getTarget()));
				} catch (...) {
					return false;
				}

				items[i]->setTextures(texture->ref().handles, true);
			}
			handlesCount += items[i]->getTextures().count();
		}
	}
	
	if (handlesCount != THIS.handles.count()) {
		THIS.handles.resize(handlesCount);
		THIS.handles1iv.resize(handlesCount);
		
		GLuint* handles = THIS.handles.items();
		GLShaderData** items = THIS.textures.items();
		uint32_t ofs = 0;
		for (int i = (int)(count-1); i >= 0; i--) {
			Vector<GLuint>* textures = &(items[i]->getTextures());
			uint32_t textureCount = textures->count();
			GLuint* textureItems = textures->items();
			for (int j = 0; j < textureCount; j++) {
				handles[ofs++] = textureItems[j];
			}
		}
	}
	
	return true;
}

GLuint RefGLShader::bind(GLint index, GLint uniform) {
	if (program != 0) {
		glUseProgram(program);
	}
	
	uint32_t count = handles.count();
	GLuint* items = handles.items();
	for (int i = 0; i < count; i++) {
		context->activeTexture(index);
		glBindTexture(GL_TEXTURE_2D, items[i]);
		handles1iv[i] = index;
		index++;
	}
	
	if (count == 1) {
		glUniform1i(uniform, handles1iv[0]);
	} else {
		glUniform1iv(uniform, count, handles1iv.items());
	}
	
	return index;
}

GLuint RefGLShader::bind(GLint index, Vector<GLint>& uniforms) {
	if (program != 0) {
		glUseProgram(program);
	}
	
	uint32_t count = handles.count();
	GLuint* items = handles.items();
	for (int i = 0; i < count; i++) {
		context->activeTexture(index);
		glBindTexture(GL_TEXTURE_2D, items[i]);
		handles1iv[i] = index;
		index++;
	}
	
	uint32_t unicount = uniforms.count();
	if (unicount > 0) {
		if (count == 1) {
			glUniform1i(uniforms[0], handles1iv[0]);
		} else if (unicount == 1) {
			glUniform1iv(uniforms[0], count, handles1iv.items());
		} else {
			if (count > unicount) count = unicount;
			for (int i = 0; i < count; i++) {
				glUniform1i(uniforms[i], handles1iv[i]);
			}
		}
	}
	
	return index;
}

GLShaders::GLShaders(GLRender* context) throw(const char*) {
	THIS.context = context;
	list = new HashMap<String, GLTexture>();
}

GLShaders::~GLShaders() {
	list = null;
	context = NULL;
}

GLShader& GLShaders::get(const wchar_t* key) throw(const char*) {
	return (GLShader&)list.get(key);
}

GLShader& GLShaders::createShader(const wchar_t* key, GLShaderData* vsh, GLShaderData* fsh, GLuint program, Vector<GLShaderData*>& textures) throw(const char*) {
	try {
		list.remove(key);
		GLShader* shader = &(list.put(key, new RefGLShader(context, vsh, fsh, program, textures)));
		if (wcscmp(key, L"null") == 0) {
		}
		return *shader;
	} catch (...) {
		throw;
	}
}

GLuint GLShaders::createVertexShader(const char* vertexShaderSource) throw(const char*) {
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
			GLchar* log = memAlloc(GLchar, log, logLen + 1);
			glGetShaderInfoLog(vertexShader, logLen, &logLen, log);
			String::format(L"OpenGL Program Log:\r\n%s", (char*)log).log();
			memFree(log);
		}
#endif
		glDeleteShader(vertexShader);
		throw eOpenGL;
	}
	
	return vertexShader;
}

GLuint GLShaders::createFragmentShader(const char* fragmentShaderSource) throw(const char*) {
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
			GLchar* log = memAlloc(GLchar, log, logLen + 1);
			glGetShaderInfoLog(fragmentShader, logLen, &logLen, log);
			String::format(L"OpenGL Program Log:\r\n%s", (char*)log).log();
			memFree(log);
		}
#endif
		glDeleteShader(fragmentShader);
		throw eOpenGL;
	}
	
	return fragmentShader;
}

void GLShaders::releaseShader(GLuint shader) {
	glDeleteShader(shader);
}

GLuint GLShaders::createProgram(GLuint vertexShader, GLuint fragmentShader) throw(const char*) {
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
			GLchar* log = memAlloc(GLchar, log, logLen + 1);
			glGetProgramInfoLog(program, logLen, &logLen, log);
			String::format(L"OpenGL Program Log:\r\n%s", (char*)log).log();
			memFree(log);
		}
#endif
		glDeleteProgram(program);
		throw eOpenGL;
	}
	
	return program;
}

void GLShaders::releaseProgram(GLuint program) {
	glDeleteProgram(program);
}
