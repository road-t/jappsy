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

GLTexture& GLReader::createTexture(GLRender* ctx, const wchar_t* key, Stream& stream) throw(const char*) {
	stream.setPosition(0);
	uint32_t head = (uint32_t)stream.readInt();
	if ((head == GLReader::SDFFHEAD) || (head == GLReader::SDFIHEAD) || (head == GLReader::JIMGHEAD)) {
		GLint width = stream.readInt();
		GLint height = stream.readInt();
		int style = GLTexture::NONE;
		int distance = 0;
		Vector<GLuint> handles;
		
		if (head != GLReader::JIMGHEAD) {
			distance = stream.readUnsignedByte();
		} else {
			style = stream.readInt();
		}
		
		try {
			uint32_t len = (uint32_t)stream.readInt();
			uint32_t chunk = (uint32_t)stream.readInt();
			while (chunk != GLReader::JENDCHUNK) {
				if ((chunk == GLReader::IGZCHUNK) || (chunk == GLReader::IRWCHUNK)) {
					void* data = NULL;
					uint32_t dataSize;
					if (chunk == GLReader::IGZCHUNK) {
						dataSize = width * height * 4;
						data = stream.readGZip(len, &dataSize);
					} else {
						data = stream.readBytes(len);
						dataSize = len;
					}

					GLuint handle;
					try {
						handle = ctx->textures->createTextureHandle(width, height, style, data);
					} catch (...) {
						memFree(data);
						throw;
					}
					memFree(data);
					try {
						handles.push(handle);
					} catch (...) {
						ctx->textures->releaseTextureHandle(handle);
						throw;
					}
				} else {
					if (stream.skip(len) < 0)
						break;
				}
				
				len = (uint32_t)stream.readInt();
				chunk = (uint32_t)stream.readInt();
			}
			
			if (handles.count() > 0) {
				return ctx->textures->createTexture(key, handles, width, height);
			} else {
				throw eIOReadLimit;
			}
		} catch (...) {
			uint32_t count = handles.size();
			GLuint* items = handles.items();
			for (int i = 0; i < count; i++) {
				ctx->textures->releaseTextureHandle(items[i]);
			}
			handles.clear();
			throw;
		}
	}
	throw eIOInvalidFile;
}

GLShader& GLReader::createShader(GLRender* ctx, const wchar_t* key, Stream& stream) throw(const char*) {
	stream.setPosition(0);
	uint32_t head = (uint32_t)stream.readInt();
	if (head == GLReader::JSHDHEAD) {
		GLShaderData* vsh = NULL;
		GLShaderData* fsh = NULL;
		GLuint program = 0;
		Vector<GLShaderData*> textures;
		
		uint32_t len = (uint32_t)stream.readInt();
		uint32_t chunk = (uint32_t)stream.readInt();
		
		try {
			while (chunk != GLReader::JENDCHUNK) {
				if ((chunk == GLReader::VGZCHUNK) ||
					(chunk == GLReader::VSHCHUNK) ||
					(chunk == GLReader::FGZCHUNK) ||
					(chunk == GLReader::FSHCHUNK)
				) {
					char* shaderSource;
					if ((chunk == GLReader::VGZCHUNK) || (chunk == GLReader::FGZCHUNK)) {
						shaderSource = stream.readGZipString(len);
					} else {
						shaderSource = stream.readString(len);
					}
					GLuint sh = 0;
					try {
						if ((chunk == GLReader::VGZCHUNK) || (chunk == GLReader::VSHCHUNK)) {
							sh = ctx->shaders->createVertexShader(shaderSource);
						} else {
							sh = ctx->shaders->createFragmentShader(shaderSource);
						}
					} catch (...) {
						memFree(shaderSource);
						throw;
					}
					memFree(shaderSource);
					GLShaderData* shd = NULL;
					try {
						shd = memNew(shd, GLShaderData(ctx));
						if (shd == NULL)
							throw eOutOfMemory;
						shd->setShader(sh, false);
					} catch (...) {
						ctx->shaders->releaseShader(sh);
						if (shd != NULL) {
							memDelete(shd);
							shd = NULL;
						}
						throw;
					}
					if ((chunk == GLReader::VGZCHUNK) || (chunk == GLReader::VSHCHUNK)) {
						vsh = shd;
					} else {
						fsh = shd;
					}
				} else if ((chunk == GLReader::VRFCHUNK) || (chunk == GLReader::FRFCHUNK) || (chunk == GLReader::IRFCHUNK)) {
					char* target = stream.readString(len);
					GLShaderData* shd = NULL;
					try {
						shd = memNew(shd, GLShaderData(ctx));
						if (shd == NULL)
							throw eOutOfMemory;
						shd->setTarget(target, false);
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
						vsh = shd;
					} else if (chunk == GLReader::FRFCHUNK) {
						fsh = shd;
					} else {
						try {
							textures.push(shd);
						} catch (...) {
							memDelete(shd);
							shd = NULL;
							throw;
						}
					}
				} else if (chunk == GLReader::IHDCHUNK) {
					GLint width = stream.readInt();
					GLint height = stream.readInt();
					int style = stream.readInt();
					
					len = stream.readInt();
					chunk = (uint32_t)stream.readInt();
					if ((chunk == GLReader::IGZCHUNK) || (chunk == GLReader::IRWCHUNK)) {
						void* data = NULL;
						uint32_t dataSize;
						if (chunk == GLReader::IGZCHUNK) {
							dataSize = width * height * 4;
							data = stream.readGZip(len, &dataSize);
						} else {
							data = stream.readBytes(len);
							dataSize = len;
						}
						
						GLuint handle;
						try {
							handle = ctx->textures->createTextureHandle(width, height, style, data);
						} catch (...) {
							memFree(data);
							throw;
						}
						memFree(data);
						
						GLShaderData* shd = NULL;
						try {
							shd = memNew(shd, GLShaderData(ctx));
							if (shd == NULL)
								throw eOutOfMemory;
							Vector<GLuint> handles;
							handles.push(handle);
							shd->setTextures(handles, false);
						} catch (...) {
							ctx->textures->releaseTextureHandle(handle);
							if (shd != NULL) {
								memDelete(shd);
								shd = NULL;
							}
							throw;
						}

						try {
							textures.push(shd);
						} catch (...) {
							memDelete(shd);
							shd = NULL;
							throw;
						}
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
			
			if ((vsh == NULL) && (fsh == NULL)) {
				throw eIOInvalidFile;
			} else if ((vsh != NULL) && (fsh != NULL) && (!vsh->isReference()) && (!fsh->isReference())) {
				program = ctx->shaders->createProgram(vsh->getShader(), fsh->getShader());
			}
			
			return ctx->shaders->createShader(key, vsh, fsh, program, textures);
		} catch (...) {
			if (vsh != NULL) {
				memDelete(vsh);
				vsh = NULL;
			}
			
			if (fsh != NULL) {
				memDelete(fsh);
				fsh = NULL;
			}
			
			if (program != 0) {
				ctx->shaders->releaseProgram(program);
				program = 0;
			}
			
			uint32_t count = textures.count();
			if (count > 0) {
				GLShaderData** items = textures.items();
				for (int i = 0; i < count; i++) {
					memDelete(items[i]);
				}
				textures.clear();
			}
			
			throw;
		}
	}
	throw eIOInvalidFile;
}
