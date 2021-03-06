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
#include <core/uSystem.h>

GLShader::GLShader(GLRender* context, const CString& key, GLObjectData* vsh, GLObjectData* fsh, GLuint program, Vector<GLObjectData*>& textures) throw(const char*) {
	this->context = context;
	this->key = key;
	this->vsh = vsh;
	this->fsh = fsh;
	this->program = program;

	int32_t count = textures.count();
	if (count > 0) {
		this->textures.resize((uint32_t)count);
		memcpy(this->textures.items(), textures.items(), count * sizeof(GLObjectData*));
	}
}

GLShader::~GLShader() {
	int32_t count = this->textures.count();
	if (count > 0) {
		GLObjectData** textures = this->textures.items();
		for (int i = 0; i < count; i++) {
			memDelete(textures[i]);
		}
	}
	
	if (program != 0) {
		GLProgram::releaseProgram(program);
		program = 0;
	}
	
	if (vsh != NULL) {
		memDelete(vsh);
	}
	
	if (fsh != NULL) {
		memDelete(fsh);
	}
}

bool GLShader::checkReady() {
	if ((this->program == 0) && (this->vsh != NULL) && (this->fsh != NULL)) {
		if (this->vsh->isReference()) {
			GLObjectData* vsh = this->vsh;
			do {
				GLShader* shader;
				try {
					shader = context->shaders->get((wchar_t*)vsh->getTarget());
					vsh = shader->vsh;
				} catch (...) {
					vsh = NULL;
					break;
				}
			} while (vsh->isReference());
			if (vsh != NULL) {
				this->vsh->setShader(vsh->getShader(), true);
			} else {
				return false;
			}
		}

		if (this->fsh->isReference()) {
			GLObjectData* fsh = this->fsh;
			do {
				GLShader* shader;
				try {
					shader = context->shaders->get((wchar_t*)fsh->getTarget());
					fsh = shader->fsh;
				} catch (...) {
					fsh = NULL;
					break;
				}
			} while (fsh->isReference());
			if (fsh != NULL) {
				this->fsh->setShader(fsh->getShader(), true);
			} else {
				return false;
			}
		}
		
		GLuint vertexShader = this->vsh->getShader();
		GLuint frameShader = this->fsh->getShader();
		
		try {
			this->program = GLProgram::createProgram(vertexShader, frameShader);
		} catch (...) {
			return false;
		}
	}
	
	uint32_t handlesCount = 0;
	int32_t count = this->textures.count();
	if (count > 0) {
		GLObjectData** items = this->textures.items();
		for (int i = count-1; i >= 0; i--) {
			if (items[i]->isReference()) {
				GLTextureSet* texture;
				try {
					texture = context->textures->get((wchar_t*)(items[i]->getTarget()));
				} catch (...) {
					return false;
				}

				items[i]->setTextures(texture->handles, true);
			}
			handlesCount += items[i]->getTextures().count();
		}
	}
	
	if (handlesCount != this->handles.count()) {
		this->handles.resize(handlesCount);
		this->handles1iv.resize(handlesCount);
		
		GLuint* handles = this->handles.items();
		GLObjectData** items = this->textures.items();
		uint32_t ofs = 0;
		for (int i = 0; i < count; i++) {
			Vector<GLuint>* textures = &(items[i]->getTextures());
			int32_t textureCount = textures->count();
			GLuint* textureItems = textures->items();
			for (int j = 0; j < textureCount; j++) {
				handles[ofs++] = textureItems[j];
			}
		}
	}
	
	return true;
}

GLuint GLShader::bind(GLuint index, GLint uniform) {
	if (program != 0) {
		glUseProgram(program);
	}
	
	int32_t count = handles.count();
	GLuint* items = handles.items();
	for (int i = 0; i < count; i++) {
		context->activeTexture(index);
		glBindTexture(GL_TEXTURE_2D, items[i]);
		handles1iv[i] = index;
		index++;
	}
	
	if (uniform != -1) {
		if (count == 1) {
			glUniform1i(uniform, handles1iv[0]);
		} else {
			glUniform1iv(uniform, count, handles1iv.items());
		}
	}
	
	return index;
}

GLuint GLShader::bind(GLuint index, Vector<GLint>& uniforms) {
	if (program != 0) {
		glUseProgram(program);
	}
	
	int32_t count = handles.count();
	GLuint* items = handles.items();
	for (int i = 0; i < count; i++) {
		context->activeTexture(index);
		glBindTexture(GL_TEXTURE_2D, items[i]);
		handles1iv[i] = index;
		index++;
	}
	
	int32_t unicount = uniforms.count();
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
	this->context = context;
}

GLShaders::~GLShaders() {
	int32_t count = list.count();
	GLShader** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLShader* GLShaders::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLShader* GLShaders::createShader(const CString& key, GLObjectData* vsh, GLObjectData* fsh, GLuint program, Vector<GLObjectData*>& textures) throw(const char*) {
	try {
		list.removedelete(key);
		GLShader* shader = new GLShader(context, key, vsh, fsh, program, textures);
		try {
			list.put(key, shader);
		} catch (...) {
			delete shader;
			throw;
		}
		return shader;
	} catch (...) {
		throw;
	}
}

// THREAD SAFE

struct CreateShaderDataThreadData {
	GLRender* context;
	const CString* key;
	Vector<GLObjectData*> textures;
	
	const char* shaderSource;
};

void* GLShaders::CreateVertexShaderCallback(void* threadData) {
	struct CreateShaderDataThreadData* thread = (struct CreateShaderDataThreadData*)threadData;
	
	GLuint sh = 0;
	try {
		sh = GLProgram::createVertexShader(thread->shaderSource);
	} catch (...) {
		throw;
	}
	GLObjectData* shd = NULL;
	try {
		shd = memNew(shd, GLObjectData(thread->context));
		shd->setShader(sh, false);
	} catch (...) {
		GLProgram::releaseShader(sh);
		if (shd != NULL) {
			memDelete(shd);
			shd = NULL;
		}
		throw;
	}
	
	try {
		return thread->context->shaders->createShader(*(thread->key), shd, NULL, 0, thread->textures);
	} catch (...) {
		memDelete(shd);
		throw;
	}
}

void* GLShaders::CreateFragmentShaderCallback(void* threadData) {
	struct CreateShaderDataThreadData* thread = (struct CreateShaderDataThreadData*)threadData;
	
	GLuint sh = 0;
	try {
		sh = GLProgram::createFragmentShader(thread->shaderSource);
	} catch (...) {
		throw;
	}
	GLObjectData* shd = NULL;
	try {
		shd = memNew(shd, GLObjectData(thread->context));
		shd->setShader(sh, false);
	} catch (...) {
		GLProgram::releaseShader(sh);
		if (shd != NULL) {
			memDelete(shd);
			shd = NULL;
		}
		throw;
	}
	
	try {
		return thread->context->shaders->createShader(*(thread->key), NULL, shd, 0, thread->textures);
	} catch (...) {
		memDelete(shd);
		throw;
	}
}

GLShader* GLShaders::createVertexShader(const CString& key, const char* vertexShaderSource) throw(const char*) {
	struct CreateShaderDataThreadData thread;
	thread.context = context;
	thread.key = &key;
	thread.shaderSource = vertexShaderSource;
	
	return (GLShader*)OpenGLThreadSync(CreateVertexShaderCallback, &thread);
}

GLShader* GLShaders::createFragmentShader(const CString& key, const char* fragmentShaderSource) throw(const char*) {
	struct CreateShaderDataThreadData thread;
	thread.context = context;
	thread.key = &key;
	thread.shaderSource = fragmentShaderSource;
	
	return (GLShader*)OpenGLThreadSync(CreateFragmentShaderCallback, &thread);
}

GLShader* GLShaders::createShader(const CString& key, const wchar_t* vshReference, const wchar_t* fshReference, void* library) throw(const char*) {
	GLObjectData* vsh = NULL;
	GLObjectData* fsh = NULL;
	Vector<GLObjectData*> textures;

#if defined(__IOS__)
	if (vshReference[0] == L'@') {
		CString vshRef = vshReference + 1;

		NSBundle* bundle = [NSBundle mainBundle];
		if (library != NULL) {
			bundle = (__bridge NSBundle*)library;
		}
		
		NSString *vertexShaderSource = [NSString stringWithContentsOfFile:[bundle pathForResource:(NSString*)vshRef ofType:nil] encoding:NSUTF8StringEncoding error:nil];
		const char *vertexShaderSourceCString = [vertexShaderSource cStringUsingEncoding:NSUTF8StringEncoding];
		createVertexShader(vshReference, vertexShaderSourceCString);
	}
	
	if (fshReference[0] == L'@') {
		CString fshRef = fshReference + 1;

		NSBundle* bundle = [NSBundle mainBundle];
		if (library != NULL) {
			bundle = (__bridge NSBundle*)library;
		}
		
		NSString *fragmentShaderSource = [NSString stringWithContentsOfFile:[bundle pathForResource:(NSString*)fshRef ofType:nil] encoding:NSUTF8StringEncoding error:nil];
		const char *fragmentShaderSourceCString = [fragmentShaderSource cStringUsingEncoding:NSUTF8StringEncoding];
		createFragmentShader(fshReference, fragmentShaderSourceCString);
	}
#endif
	
	try {
		vsh = memNew(vsh, GLObjectData(context));
		vsh->setTarget(vshReference);
		
		fsh = memNew(fsh, GLObjectData(context));
		fsh->setTarget(fshReference);
	} catch (...) {
		if (vsh != NULL) {
			memDelete(vsh);
			vsh = NULL;
		}
		if (fsh != NULL) {
			memDelete(fsh);
			fsh = NULL;
		}
		throw;
	}
	
	try {
		try {
			GLShader* shader = list.get(key);
			int32_t count = shader->textures.count();
			GLObjectData** items = shader->textures.items();
			for (int i = 0; i < count; i++) {
				textures.push(items[i]);
			}
			shader->textures.clear();
		} catch (...) {
		}
		return createShader(key, vsh, fsh, 0, textures);
	} catch (...) {
		memDelete(vsh);
		memDelete(fsh);
		
		int32_t count = textures.count();
		if (count > 0) {
			GLObjectData** items = textures.items();
			for (int i = 0; i < count; i++) {
				memDelete(items[i]);
			}
			textures.clear();
		}
		
		throw;
	}
}

