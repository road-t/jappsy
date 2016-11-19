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

const static char saVertexPosition[] = "aVertexPosition";
const static char saTextureCoord[] = "aTextureCoord";
const static char suLayerProjectionMatrix[] = "uLayerProjectionMatrix";
const static char suSize[] = "uSize";
const static char suPosition[] = "uPosition";

GLProgram::GLProgram(GLContext& context, const char* vsh, const char* fsh, const GLProgramVariable *attributes, const GLProgramVariable *uniforms) throw(const char*) {
	this->context = &context;
}

GLProgram::~GLProgram() {
	
}

void GLProgram::destroy() {
	
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
	{ NULL, NULL }
}
{}

GLProgramTexture::GLProgramTexture(GLContext& context) : GLProgram(context, GLProgramTexture_VSH, GLProgramTexture_FSH, attributes, uniforms) {
	
}
