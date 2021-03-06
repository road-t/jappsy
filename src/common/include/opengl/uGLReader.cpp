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

#include "uGLReader.h"
#include <opengl/uGLRender.h>
#include <opengl/uGLTextureSet.h>
#include <opengl/uGLShader.h>
#include <opengl/uGLObjectData.h>
#include <data/uVector.h>
#include <cipher/uCipher.h>
#include <core/uSystem.h>

struct CreateTextureThreadData {
	GLRender* context;
	const wchar_t* key;
	
	Vector<GLuint> handles;
	GLint width;
	GLint height;
	int style;
	
	void* data;
};

void* GLReader::CreateTextureHandleCallback(void* threadData) {
	struct CreateTextureThreadData* thread = (struct CreateTextureThreadData*)threadData;
	
	GLuint handle;
	try {
		handle = thread->context->textures->createTextureHandle(thread->width, thread->height, thread->style, thread->data);
	} catch (...) {
		memFree(thread->data);
		throw;
	}
	memFree(thread->data);
	try {
		thread->handles.push(handle);
	} catch (...) {
		thread->context->textures->releaseTextureHandle(handle);
		throw;
	}
	
	return NULL;
}

void* GLReader::CreateTextureCallback(void* threadData) {
	struct CreateTextureThreadData* thread = (struct CreateTextureThreadData*)threadData;
	
	if (thread->handles.count() > 0) {
		return thread->context->textures->createTexture(thread->key, thread->handles, thread->width, thread->height);
	} else {
		throw eIOReadLimit;
	}
}

void* GLReader::CreateTextureErrorCallback(void* threadData) {
	struct CreateTextureThreadData* thread = (struct CreateTextureThreadData*)threadData;
	
	int32_t count = thread->handles.count();
	GLuint* items = thread->handles.items();
	for (int i = 0; i < count; i++) {
		thread->context->textures->releaseTextureHandle(items[i]);
	}
	thread->handles.clear();
	
	return NULL;
}

GLTextureSet* GLReader::createTexture(GLRender* ctx, const CString& key, Stream* stream) throw(const char*) {
	stream->setPosition(0);
	uint32_t head = (uint32_t)stream->readU32();
	if ((head == GLReader::SDFFHEAD) || (head == GLReader::SDFIHEAD) || (head == GLReader::JIMGHEAD)) {
		struct CreateTextureThreadData thread;
		thread.context = ctx;
		thread.key = (wchar_t*)key;
		thread.width = stream->readU32();
		thread.height = stream->readU32();
		thread.style = GLTextureSet::NONE;
		int distance = 0; //unused
		
		if (head != GLReader::JIMGHEAD) {
			distance = stream->readU8();
		} else {
			thread.style = stream->readU32();
		}
		
		try {
			uint32_t len = (uint32_t)stream->readU32();
			uint32_t chunk = (uint32_t)stream->readU32();
			while (chunk != GLReader::JENDCHUNK) {
				if ((chunk == GLReader::IGZCHUNK) || (chunk == GLReader::IRWCHUNK)) {
					uint32_t dataSize;
					if (chunk == GLReader::IGZCHUNK) {
						dataSize = (uint32_t)(thread.width * thread.height * 4);
						thread.data = stream->readGZip(len, &dataSize);
					} else {
						thread.data = stream->readBytes(len);
						dataSize = len;
					}

					(void)OpenGLThreadSync(CreateTextureHandleCallback, &thread);
				} else {
					if (stream->skip(len) < 0)
						break;
				}
				
				len = (uint32_t)stream->readU32();
				chunk = (uint32_t)stream->readU32();
			}
			
			return (GLTextureSet*)(OpenGLThreadSync(CreateTextureCallback, &thread));
		} catch (...) {
			(void)OpenGLThreadSync(CreateTextureErrorCallback, &thread);
			throw;
		}
	}
	throw eIOInvalidFile;
}

struct CreateShaderThreadData {
	GLRender* context;
	const wchar_t* key;
	
	GLObjectData* vsh;
	GLObjectData* fsh;
	GLuint program;
	Vector<GLObjectData*> textures;
	
	uint32_t chunk;
	char* shaderSource;

	GLint width;
	GLint height;
	int style;
	
	void* data;
};

void* GLReader::CreateShaderSourceCallback(void* threadData) {
	struct CreateShaderThreadData* thread = (struct CreateShaderThreadData*)threadData;
	
	GLuint sh = 0;
	try {
		if ((thread->chunk == GLReader::VGZCHUNK) || (thread->chunk == GLReader::VSHCHUNK)) {
			sh = GLProgram::createVertexShader(thread->shaderSource);
		} else {
			sh = GLProgram::createFragmentShader(thread->shaderSource);
		}
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
	if ((thread->chunk == GLReader::VGZCHUNK) || (thread->chunk == GLReader::VSHCHUNK)) {
		thread->vsh = shd;
	} else {
		thread->fsh = shd;
	}

	return NULL;
}

void* GLReader::CreateShaderTextureCallback(void* threadData) {
	struct CreateShaderThreadData* thread = (struct CreateShaderThreadData*)threadData;
	
	GLuint handle;
	try {
		handle = thread->context->textures->createTextureHandle(thread->width, thread->height, thread->style, thread->data);
	} catch (...) {
		memFree(thread->data);
		throw;
	}
	memFree(thread->data);
	
	GLObjectData* shd = NULL;
	try {
		shd = memNew(shd, GLObjectData(thread->context));
		Vector<GLuint> handles;
		handles.push(handle);
		shd->setTextures(handles, false);
	} catch (...) {
		thread->context->textures->releaseTextureHandle(handle);
		if (shd != NULL) {
			memDelete(shd);
			shd = NULL;
		}
		throw;
	}
	
	try {
		thread->textures.push(shd);
	} catch (...) {
		memDelete(shd);
		shd = NULL;
		throw;
	}
	
	return NULL;
}

void* GLReader::CreateShaderProgramCallback(void* threadData) {
	struct CreateShaderThreadData* thread = (struct CreateShaderThreadData*)threadData;
	
	thread->program = GLProgram::createProgram(thread->vsh->getShader(), thread->fsh->getShader());
	
	return NULL;
}

void* GLReader::CreateShaderCallback(void* threadData) {
	struct CreateShaderThreadData* thread = (struct CreateShaderThreadData*)threadData;
	
	return thread->context->shaders->createShader(thread->key, thread->vsh, thread->fsh, thread->program, thread->textures);
}

void* GLReader::CreateShaderErrorCallback(void* threadData) {
	struct CreateShaderThreadData* thread = (struct CreateShaderThreadData*)threadData;
	
	if (thread->vsh != NULL) {
		memDelete(thread->vsh);
		thread->vsh = NULL;
	}
	
	if (thread->fsh != NULL) {
		memDelete(thread->fsh);
		thread->fsh = NULL;
	}
	
	if (thread->program != 0) {
		GLProgram::releaseProgram(thread->program);
		thread->program = 0;
	}
	
	int32_t count = thread->textures.count();
	if (count > 0) {
		GLObjectData** items = thread->textures.items();
		for (int i = 0; i < count; i++) {
			memDelete(items[i]);
		}
		thread->textures.clear();
	}
	
	return NULL;
}

GLShader* GLReader::createShader(GLRender* ctx, const CString& key, Stream* stream) throw(const char*) {
	stream->setPosition(0);
	uint32_t head = (uint32_t)stream->readU32();
	if (head == GLReader::JSHDHEAD) {
		struct CreateShaderThreadData thread;
		thread.context = ctx;
		thread.key = (wchar_t*)key;
		thread.vsh = NULL;
		thread.fsh = NULL;
		thread.program = 0;

		uint32_t len = (uint32_t)stream->readU32();
		uint32_t chunk = (uint32_t)stream->readU32();

		try {
			while (chunk != GLReader::JENDCHUNK) {
				if ((chunk == GLReader::VGZCHUNK) ||
					(chunk == GLReader::VSHCHUNK) ||
					(chunk == GLReader::FGZCHUNK) ||
					(chunk == GLReader::FSHCHUNK)
				) {
					if ((chunk == GLReader::VGZCHUNK) || (chunk == GLReader::FGZCHUNK)) {
						thread.shaderSource = stream->readGZipString(len);
					} else {
						thread.shaderSource = stream->readString(len);
					}
					
					thread.chunk = chunk;
					try {
						(void)OpenGLThreadSync(CreateShaderSourceCallback, &thread);
					} catch (...) {
						memFree(thread.shaderSource);
						thread.shaderSource = NULL;
						throw;
					}
					memFree(thread.shaderSource);
					thread.shaderSource = NULL;
				} else if ((chunk == GLReader::VRFCHUNK) || (chunk == GLReader::FRFCHUNK) || (chunk == GLReader::IRFCHUNK)) {
					char* target = stream->readString(len);
					GLObjectData* shd = NULL;
					try {
						shd = memNew(shd, GLObjectData(ctx));
						shd->setTarget(target);
					} catch (...) {
						memFree(target);
						if (shd != NULL) {
							memDelete(shd);
							shd = NULL;
						}
						throw;
					}
					memFree(target);
					if (chunk == GLReader::VRFCHUNK) {
						thread.vsh = shd;
					} else if (chunk == GLReader::FRFCHUNK) {
						thread.fsh = shd;
					} else {
						try {
							thread.textures.push(shd);
						} catch (...) {
							memDelete(shd);
							shd = NULL;
							throw;
						}
					}
				} else if (chunk == GLReader::IHDCHUNK) {
					thread.width = stream->readU32();
					thread.height = stream->readU32();
					thread.style = stream->readU32();
					
					len = stream->readU32();
					chunk = (uint32_t)stream->readU32();
					if ((chunk == GLReader::IGZCHUNK) || (chunk == GLReader::IRWCHUNK)) {
						uint32_t dataSize;
						if (chunk == GLReader::IGZCHUNK) {
							dataSize = (uint32_t)(thread.width * thread.height * 4);
							thread.data = stream->readGZip(len, &dataSize);
						} else {
							thread.data = stream->readBytes(len);
							dataSize = len;
						}
						
						(void)OpenGLThreadSync(CreateShaderTextureCallback, &thread);
					} else {
						throw eIOInvalidFile;
					}
				} else {
					if (stream->skip(len) < 0)
						break;
				}
				
				len = (uint32_t)stream->readU32();
				chunk = (uint32_t)stream->readU32();
			}
			
			if ((thread.vsh == NULL) && (thread.fsh == NULL)) {
				throw eIOInvalidFile;
			} else if ((thread.vsh != NULL) && (thread.fsh != NULL) && (!thread.vsh->isReference()) && (!thread.fsh->isReference())) {
				(void)OpenGLThreadSync(CreateShaderProgramCallback, &thread);
			}
			return (GLShader*)(OpenGLThreadSync(CreateShaderCallback, &thread));
		} catch (...) {
			(void)OpenGLThreadSync(CreateShaderErrorCallback, &thread);
			throw;
		}
	}
	throw eIOInvalidFile;
}
