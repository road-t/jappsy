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

#ifndef JAPPSY_UOPENGL_H
#define JAPPSY_UOPENGL_H

#include <platform.h>

#if defined(__IOS__)
	#import <OpenGLES/ES2/gl.h>
	#import <OpenGLES/ES2/glext.h>
#elif defined(__OSX__)
	#import <OpenGL/OpenGL.h>

	#if CGL_VERSION_1_3
		#import <OpenGL/gl3.h>
	#else
		#import <OpenGL/gl.h>
	#endif
#elif defined(__JNI__)
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
#elif defined(__WINNT__)
	#include <opengl/opengl.h>
#endif

#define clamp(v, min, max)	((v < min) ? min : ((v > max) ? max : v))

#if defined(__IOS__)
	#define glBindVertexArray glBindVertexArrayOES
	#define glGenVertexArrays glGenVertexArraysOES
	#define glDeleteVertexArrays glDeleteVertexArraysOES
#elif defined(__OSX__)
	#if CGL_VERSION_1_3
		#define glBindVertexArray glBindVertexArray
		#define glGenVertexArrays glGenVertexArrays
		#define glGenerateMipmap glGenerateMipmap
		#define glDeleteVertexArrays glDeleteVertexArrays
	#else
		#define glBindVertexArray glBindVertexArrayAPPLE
		#define glGenVertexArrays glGenVertexArraysAPPLE
		#define glGenerateMipmap glGenerateMipmapEXT
		#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
	#endif
#endif

static inline const char* GetGLErrorString(GLenum error) {
	const char *str;
	switch(error) {
		case GL_NO_ERROR:
			str = "GL_NO_ERROR";
			break;
		case GL_INVALID_ENUM:
			str = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			str = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			str = "GL_INVALID_OPERATION";
			break;
		#if defined (GL_OUT_OF_MEMORY)
			case GL_OUT_OF_MEMORY:
				str = "GL_OUT_OF_MEMORY";
				break;
		#endif
		#if defined(GL_INVALID_FRAMEBUFFER_OPERATION)
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				str = "GL_INVALID_FRAMEBUFFER_OPERATION";
				break;
		#endif
		#if defined(GL_STACK_OVERFLOW)
			case GL_STACK_OVERFLOW:
				str = "GL_STACK_OVERFLOW";
				break;
		#endif
		#if defined(GL_STACK_UNDERFLOW)
			case GL_STACK_UNDERFLOW:
				str = "GL_STACK_UNDERFLOW";
				break;
		#endif
		#if defined(GL_TABLE_TOO_LARGE)
			case GL_TABLE_TOO_LARGE:
				str = "GL_TABLE_TOO_LARGE";
				break;
		#endif
		default:
			str = "(ERROR: Unknown Error Enum)";
			break;
	}
	return str;
}

#define GetGLError()									\
{														\
    GLenum err = glGetError();							\
    while (err != GL_NO_ERROR) {						\
        LOG("GLError %s set in File:%s Line:%d",		\
        GetGLErrorString(err), __FILE__, __LINE__);	    \
        err = glGetError();								\
    }													\
}

#define CheckGLError()									\
{														\
	GLenum err = glGetError();							\
	GLint count = 0;									\
	bool oom = false;									\
	while (err != GL_NO_ERROR) {						\
		if (err == GL_OUT_OF_MEMORY) oom = true;		\
		LOG("GLError %s set in File:%s Line:%d",		\
		GetGLErrorString(err), __FILE__, __LINE__);	    \
		err = glGetError();								\
	}													\
	if (count > 0) {									\
		if (oom) throw eOutOfMemory;					\
		throw eOpenGL;									\
	}													\
}

static inline const char* GetGLFramebufferStatus(GLenum status) {
	const char *str;
	switch (status) {
		case GL_FRAMEBUFFER_COMPLETE:
			str = "GL_FRAMEBUFFER_COMPLETE";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			str = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
			str = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			str = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			str = "GL_FRAMEBUFFER_UNSUPPORTED";
			break;
		default:
			str = "(ERROR: Unknown Framebuffer Status)";
			break;
	}
	return str;
}

#define GetGLFramebufferStatus()									\
{																	\
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);		\
	if (status != GL_FRAMEBUFFER_COMPLETE) {						\
		NSLog(@"GLFramebufferStatus %s set in File:%s Line:%d\n",   \
			GetGLFramebufferStatus(status), __FILE__, __LINE__);	\
	}																\
}

#endif
