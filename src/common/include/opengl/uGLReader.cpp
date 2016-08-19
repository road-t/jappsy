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
#include <opengl/uGLTexture.h>
#include <opengl/uGLShader.h>
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
		return &(thread->context->textures->createTexture(thread->key, thread->handles, thread->width, thread->height));
	} else {
		throw eIOReadLimit;
	}
}

void* GLReader::CreateTextureErrorCallback(void* threadData) {
	struct CreateTextureThreadData* thread = (struct CreateTextureThreadData*)threadData;
	
	uint32_t count = thread->handles.size();
	GLuint* items = thread->handles.items();
	for (int i = 0; i < count; i++) {
		thread->context->textures->releaseTextureHandle(items[i]);
	}
	thread->handles.clear();
	
	return NULL;
}

GLTexture& GLReader::createTexture(GLRender* ctx, const wchar_t* key, Stream& stream) throw(const char*) {
	stream.setPosition(0);
	uint32_t head = (uint32_t)stream.readInt();
	if ((head == GLReader::SDFFHEAD) || (head == GLReader::SDFIHEAD) || (head == GLReader::JIMGHEAD)) {
		struct CreateTextureThreadData thread;
		thread.context = ctx;
		thread.key = key;
		thread.width = stream.readInt();
		thread.height = stream.readInt();
		thread.style = GLTexture::NONE;
		int distance = 0; //unused
		
		if (head != GLReader::JIMGHEAD) {
			distance = stream.readUnsignedByte();
		} else {
			thread.style = stream.readInt();
		}
		
		try {
			uint32_t len = (uint32_t)stream.readInt();
			uint32_t chunk = (uint32_t)stream.readInt();
			while (chunk != GLReader::JENDCHUNK) {
				if ((chunk == GLReader::IGZCHUNK) || (chunk == GLReader::IRWCHUNK)) {
					uint32_t dataSize;
					if (chunk == GLReader::IGZCHUNK) {
						dataSize = thread.width * thread.height * 4;
						thread.data = stream.readGZip(len, &dataSize);
					} else {
						thread.data = stream.readBytes(len);
						dataSize = len;
					}

					(void)MainThreadSync(CreateTextureHandleCallback, &thread);
				} else {
					if (stream.skip(len) < 0)
						break;
				}
				
				len = (uint32_t)stream.readInt();
				chunk = (uint32_t)stream.readInt();
			}
			
			return *(GLTexture*)(MainThreadSync(CreateTextureCallback, &thread));
		} catch (...) {
			(void)MainThreadSync(CreateTextureErrorCallback, &thread);
			throw;
		}
	}
	throw eIOInvalidFile;
}

struct CreateShaderThreadData {
	GLRender* context;
	const wchar_t* key;
	
	GLShaderData* vsh;
	GLShaderData* fsh;
	GLuint program;
	Vector<GLShaderData*> textures;
	
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
			sh = thread->context->shaders->createVertexShader(thread->shaderSource);
		} else {
			sh = thread->context->shaders->createFragmentShader(thread->shaderSource);
		}
	} catch (...) {
		throw;
	}
	GLShaderData* shd = NULL;
	try {
		shd = memNew(shd, GLShaderData(thread->context));
		if (shd == NULL)
			throw eOutOfMemory;
		shd->setShader(sh, false);
	} catch (...) {
		thread->context->shaders->releaseShader(sh);
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
	
	GLShaderData* shd = NULL;
	try {
		shd = memNew(shd, GLShaderData(thread->context));
		if (shd == NULL)
			throw eOutOfMemory;
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
	
	thread->program = thread->context->shaders->createProgram(thread->vsh->getShader(), thread->fsh->getShader());
	
	return NULL;
}

void* GLReader::CreateShaderCallback(void* threadData) {
	struct CreateShaderThreadData* thread = (struct CreateShaderThreadData*)threadData;
	
	return &(thread->context->shaders->createShader(thread->key, thread->vsh, thread->fsh, thread->program, thread->textures));
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
		thread->context->shaders->releaseProgram(thread->program);
		thread->program = 0;
	}
	
	uint32_t count = thread->textures.count();
	if (count > 0) {
		GLShaderData** items = thread->textures.items();
		for (int i = 0; i < count; i++) {
			memDelete(items[i]);
		}
		thread->textures.clear();
	}
	
	return NULL;
}

GLShader& GLReader::createShader(GLRender* ctx, const wchar_t* key, Stream& stream) throw(const char*) {
	stream.setPosition(0);
	uint32_t head = (uint32_t)stream.readInt();
	if (head == GLReader::JSHDHEAD) {
		struct CreateShaderThreadData thread;
		thread.context = ctx;
		thread.key = key;
		thread.vsh = NULL;
		thread.fsh = NULL;
		thread.program = 0;

		uint32_t len = (uint32_t)stream.readInt();
		uint32_t chunk = (uint32_t)stream.readInt();

		try {
			while (chunk != GLReader::JENDCHUNK) {
				if ((chunk == GLReader::VGZCHUNK) ||
					(chunk == GLReader::VSHCHUNK) ||
					(chunk == GLReader::FGZCHUNK) ||
					(chunk == GLReader::FSHCHUNK)
				) {
					if ((chunk == GLReader::VGZCHUNK) || (chunk == GLReader::FGZCHUNK)) {
						thread.shaderSource = stream.readGZipString(len);
					} else {
						thread.shaderSource = stream.readString(len);
					}
					
					thread.chunk = chunk;
					try {
						(void)MainThreadSync(CreateShaderSourceCallback, &thread);
					} catch (...) {
						memFree(thread.shaderSource);
						thread.shaderSource = NULL;
						throw;
					}
					memFree(thread.shaderSource);
					thread.shaderSource = NULL;
				} else if ((chunk == GLReader::VRFCHUNK) || (chunk == GLReader::FRFCHUNK) || (chunk == GLReader::IRFCHUNK)) {
					char* target = stream.readString(len);
					GLShaderData* shd = NULL;
					try {
						shd = memNew(shd, GLShaderData(ctx));
						if (shd == NULL)
							throw eOutOfMemory;
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
					thread.width = stream.readInt();
					thread.height = stream.readInt();
					thread.style = stream.readInt();
					
					len = stream.readInt();
					chunk = (uint32_t)stream.readInt();
					if ((chunk == GLReader::IGZCHUNK) || (chunk == GLReader::IRWCHUNK)) {
						uint32_t dataSize;
						if (chunk == GLReader::IGZCHUNK) {
							dataSize = thread.width * thread.height * 4;
							thread.data = stream.readGZip(len, &dataSize);
						} else {
							thread.data = stream.readBytes(len);
							dataSize = len;
						}
						
						(void)MainThreadSync(CreateShaderTextureCallback, &thread);
					} else {
						throw eIOInvalidFile;
					}
				} else {
					if (stream.skip(len) < 0)
						break;
				}
				
				len = (uint32_t)stream.readInt();
				chunk = (uint32_t)stream.readInt();
			}
			
			if ((thread.vsh == NULL) && (thread.fsh == NULL)) {
				throw eIOInvalidFile;
			} else if ((thread.vsh != NULL) && (thread.fsh != NULL) && (!thread.vsh->isReference()) && (!thread.fsh->isReference())) {
				(void)MainThreadSync(CreateShaderProgramCallback, &thread);
			}
			
			return *(GLShader*)(MainThreadSync(CreateShaderCallback, &thread));
		} catch (...) {
			(void)MainThreadSync(CreateShaderErrorCallback, &thread);
			throw;
		}
	}
	throw eIOInvalidFile;
}
