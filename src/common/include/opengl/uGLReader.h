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

#ifndef JAPPSY_UGLREADER_H
#define JAPPSY_UGLREADER_H

#include <platform.h>
#include <opengl/uOpenGL.h>
#include <data/uString.h>
#include <data/uStream.h>

class GLRender;
class GLTexture;
class GLShader;

class GLReader {
public:
	constexpr static int SDFFHEAD = 0x46464453;	// Signed Distance Field Font Head
	constexpr static int SDFIHEAD = 0x49464453;	// Signed Distance Field Image Head
	constexpr static int JIMGHEAD = 0x474D494A;	// Jappsy Image Head
	constexpr static int JSHDHEAD = 0x4448534A;	// Jappsy Shader Head
	
	constexpr static int IGZCHUNK = 0x5A472E49;	// GZip Pixel Data Chunk
	constexpr static int IRWCHUNK = 0x57522E49;	// Raw Pixel Data Chunk
	
	constexpr static int VGZCHUNK = 0x475A2E56;	// GZip Vertex Shader Data Chunk
	constexpr static int VSHCHUNK = 0x48532E56;	// Raw Vertex Shader Data Chunk
	constexpr static int VRFCHUNK = 0x46522E56;	// String Reference Chunk to Vertex Shader
	constexpr static int FGZCHUNK = 0x475A2E46;	// GZip Font Data Chunk | GZip Frame Shader Data Chunk
	constexpr static int FSHCHUNK = 0x48532E46;	// Raw Frame Shader Data Chunk
	constexpr static int FRFCHUNK = 0x46522E46;	// String Reference Chunk to Frame Shader
	constexpr static int IHDCHUNK = 0x44482E49;	// Jappsy Image Head Chunk
	constexpr static int IRFCHUNK = 0x46522E49;	// String Reference Chunk to Image
	
	constexpr static int JENDCHUNK = 0x444E454A;	// Jappsy End Chunk
	
	static GLTexture& createTexture(GLRender* ctx, const String& key, Stream& stream) throw(const char*);
	static GLShader& createShader(GLRender* ctx, const String& key, Stream& stream) throw(const char*);
	
private:
	static void* CreateTextureHandleCallback(void* threadData);
	static void* CreateTextureCallback(void* threadData);
	static void* CreateTextureErrorCallback(void* threadData);
	
	static void* CreateShaderSourceCallback(void* threadData);
	static void* CreateShaderTextureCallback(void* threadData);
	static void* CreateShaderProgramCallback(void* threadData);
	static void* CreateShaderCallback(void* threadData);
	static void* CreateShaderErrorCallback(void* threadData);
};

#endif //JAPPSY_UGLREADER_H