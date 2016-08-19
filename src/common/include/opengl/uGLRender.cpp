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

#include "uGLRender.h"

#include <opengl/uGLCamera.h>
#include <opengl/uGLEngine.h>

const char* GLRender::extensions = NULL;

bool GLRender::isExtensionSupported(const char *extension) {
	if (extensions == NULL)
		return false;
	
	const char *start;
	const char *where, *terminator;
	
	where = strchr(extension, ' ');
	if ( where || *extension == '\0' )
		return false;
	
	for ( start = extensions; ; ) {
		where = strstr( start, extension );
		
		if ( !where )
			break;
		
		terminator = where + strlen( extension );
		
		if ( where == start || *(where - 1) == ' ' )
			if ( *terminator == ' ' || *terminator == '\0' )
				return true;
		
		start = terminator;
	}
	
	return false;
}

GLRender::GLRender(GLEngine engine, uint32_t width, uint32_t height, GLFrame::onFrameCallback onframe, RefGLTouchScreen::onTouchCallback ontouch) {
	THIS.engine = engine;
	THIS.width = width;
	THIS.height = height;
	
	frame = memNew(frame, GLFrame(engine, this, onframe, engine));
	touchScreen = new GLTouchScreen(this, ontouch, engine);
	loader = new Loader(this, engine);
	
	textures = memNew(textures, GLTextures(this));
	shaders = memNew(shaders, GLShaders(this));
	sprites = memNew(sprites, GLSprites(this));
	scenes = memNew(scenes, GLScenes(this));
	cameras = memNew(cameras, GLCameras(this));
	models = memNew(models, GLModels(this));
	particles = memNew(particles, GLParticles(this));
	drawings = memNew(drawings, GLDrawings(this));
	
	cameras->createCamera(L"gui").ref().size(width, height).layer(0, 0);
	lightsMaxCount = 6;
	
	shaderSprite = NULL;
	shaderParticle = NULL;
	shaderModel = NULL;
	shaderSquareFill = NULL;
	shaderSquareStroke = NULL;
	shaderSquareTexture = NULL;
	
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	
	extensions = (const char*)glGetString(GL_EXTENSIONS);
	isNPOTSupported = isExtensionSupported("GL_OES_texture_npot");
	
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearDepthf(1.0);
	glDepthFunc(GL_LEQUAL);
	resetBlend();
	
	glGenBuffers(1, &m_squareBuffer);
	glGenBuffers(1, &m_textureBuffer);
	glGenBuffers(1, &m_normalBuffer);
	glGenBuffers(1, &m_indexBuffer);
	
	light.set(1.0);
}

GLRender::~GLRender() {
	loader.release();
	touchScreen.release();
	touchScreen = null;
	if (frame != NULL) {
		memDelete(frame);
		frame = NULL;
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_squareBuffer);
	glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &m_squareBuffer);
	m_squareBuffer = 0;

	glBindBuffer(GL_ARRAY_BUFFER, m_textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &m_textureBuffer);
	m_textureBuffer = 0;

	glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &m_normalBuffer);
	m_normalBuffer = 0;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
	glDeleteBuffers(1, &m_indexBuffer);
	m_indexBuffer = 0;
	
	shaderSprite = NULL;
	shaderParticle = NULL;
	shaderModel = NULL;
	shaderSquareFill = NULL;
	shaderSquareStroke = NULL;
	shaderSquareTexture = NULL;
	
	memDelete(drawings);
	memDelete(particles);
	memDelete(models);
	memDelete(cameras);
	memDelete(scenes);
	memDelete(sprites);
	memDelete(shaders);
	memDelete(textures);
	
	drawings = NULL;
	particles = NULL;
	models = NULL;
	cameras = NULL;
	scenes = NULL;
	sprites = NULL;
	textures = NULL;
	shaders = NULL;
	
	engine = null;
}

void GLRender::resetBlend() {
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
}

void GLRender::activeTexture(GLint index) {
	switch (index) {
		case 1: glActiveTexture(GL_TEXTURE1); break;
		case 2: glActiveTexture(GL_TEXTURE2); break;
		case 3: glActiveTexture(GL_TEXTURE3); break;
		default: glActiveTexture(GL_TEXTURE0); break;
	}
}

void GLRender::cleanup(GLint index) {
	for (GLint i = 0; i < index; i++) {
		activeTexture(i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	glUseProgram(0);
}

GLfloat* GLRender::makeRect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
	m_square[0] = m_square[4] = x1;
	m_square[1] = m_square[3] = y1;
	m_square[2] = m_square[6] = x2;
	m_square[5] = m_square[7] = y2;
	return m_square;
}

GLfloat* GLRender::makeLine(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
	m_line[0] = x1;
	m_line[1] = y1;
	m_line[2] = x2;
	m_line[3] = y2;
	return m_line;
}

GLfloat* GLRender::makeColor(uint32_t* color, uint32_t count) {
	if (count > 5) count = 5;
	for (uint32_t i = 0; i < count; i++) {
		m_color[i * 4 + 0] = (GLfloat)((color[i] >> 16) & 0xFF) / 255.0;	// R
		m_color[i * 4 + 1] = (GLfloat)((color[i] >> 8) & 0xFF) / 255.0;	// G
		m_color[i * 4 + 2] = (GLfloat)(color[i] & 0xFF) / 255.0;			// B
		m_color[i * 4 + 3] = (GLfloat)((color[i] >> 24) & 0xFF) / 255.0;	// A
	}
	return m_color;
}

void GLRender::fill(uint32_t color) {
	GLfloat* c = makeColor(&color, 1);
	glClearColor(c[0], c[1], c[2], c[3]);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GLRender::fillAlpha(uint8_t alpha) {
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	glClearColor(0.0, 0.0, 0.0, (GLfloat)alpha / 255.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void GLRender::fillDepth() {
	glClear(GL_DEPTH_BUFFER_BIT);
}

void GLRender::drawRect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const GLPaint& paint) {
	// TODO:
	/*
	GLCamera cam = THIS.cameras->gui;
	cam.update();
	
	if ((paint.m_color & 0xFF000000) != 0) {
		var shader = this.shaderSquareFill;
		
		if ((paint.m_color & 0xFF000000) != 0xFF000000)
			gl.enable(gl.BLEND);
		else
			gl.disable(gl.BLEND);
		
		gl.useProgram(shader.program);
		
		gl.uniformMatrix4fv(shader.uLayerProjectionMatrix, false, cam.projection16fv);
		
		var c = this.makeColor(paint.m_color);
		gl.uniform4fv(shader.uColor, c);
		
		gl.bindBuffer(gl.ARRAY_BUFFER, this.m_squareBuffer);
		gl.bufferData(gl.ARRAY_BUFFER, this.makeRect(x1, y1, x2, y2), gl.DYNAMIC_DRAW);
		gl.vertexAttribPointer(shader.aVertexPosition, 2, gl.FLOAT, false, 0, 0);
		gl.enableVertexAttribArray(shader.aVertexPosition);
		
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
		
		gl.disableVertexAttribArray(shader.aVertexPosition);
		gl.disable(gl.BLEND);
		gl.useProgram(null);
	}
	
	if ((paint.m_strokeColor & 0xFF000000) != 0) {
		var shader = this.shaderSquareStroke;
		
		gl.enable(gl.BLEND);
		gl.useProgram(shader.program);
		
		gl.uniformMatrix4fv(shader.uLayerProjectionMatrix, false, cam.projection16fv);
		
		var half = Math.floor(paint.m_strokeWidth / 2);
		gl.uniform2fv(shader.uCorners, new Float32Array([ x1 - half, y1 - half, x2 + half, y2 + half ]));
		gl.uniform1f(shader.uBorder, paint.m_strokeWidth);
		
		var c = this.makeColor(paint.m_strokeColor);
		gl.uniform4fv(shader.uColor, c);
		
		gl.bindBuffer(gl.ARRAY_BUFFER, this.m_squareBuffer);
		gl.bufferData(gl.ARRAY_BUFFER, this.makeRect(x1-half, y1-half, x2+half, y2+half), gl.DYNAMIC_DRAW);
		gl.vertexAttribPointer(shader.aVertexPosition, 2, gl.FLOAT, false, 0, 0);
		gl.enableVertexAttribArray(shader.aVertexPosition);
		
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
		
		gl.disableVertexAttribArray(shader.aVertexPosition);
		gl.disable(gl.BLEND);
		gl.useProgram(null);
	}
*/
}

void GLRender::drawTexture(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const wchar_t* key) {
	// TODO:
/*
	var cam = this.cameras.get("gui");
	cam.update();
	
	var shader = this.shaderSquareTexture;
	
	gl.enable(gl.BLEND);
	gl.useProgram(shader.program);
	
	gl.uniformMatrix4fv(shader.uLayerProjectionMatrix, false, cam.projection16fv);
	
	gl.bindBuffer(gl.ARRAY_BUFFER, this.m_squareBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, this.makeRect(x1, y1, x2, y2), gl.DYNAMIC_DRAW);
	gl.vertexAttribPointer(shader.aVertexPosition, 2, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(shader.aVertexPosition);
	
	gl.bindBuffer(gl.ARRAY_BUFFER, this.m_textureBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, this.makeRect(0, 0, 1, 1), gl.DYNAMIC_DRAW);
	gl.vertexAttribPointer(shader.aTextureCoord, 2, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(shader.aTextureCoord);
	
	//gl.enable(gl.TEXTURE_2D);
	var texture = this.textures.get(key);
	var index = texture.bind(0, shader.uTexture);
	
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
	
	gl.disableVertexAttribArray(shader.aVertexPosition);
	gl.disableVertexAttribArray(shader.aTextureCoord);
	
	this.cleanup(index);
	//gl.disable(gl.TEXTURE_2D);
 */
}

void GLRender::drawEffect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const wchar_t* key, GLfloat localTime, GLfloat worldTime) {
	// TODO:
/*
	var cam = this.cameras.get("gui");
	cam.update();
	
	var shader = this.shaders.get(key);
	var program = shader.program;
	var uLayerProjectionMatrix = gl.getUniformLocation(program, "uLayerProjectionMatrix");
	//var uTexture = gl.getUniformLocation(program, "uTexture");
	var aVertexPosition = gl.getAttribLocation(program, "aVertexPosition");
	var aTextureCoord = gl.getAttribLocation(program, "aTextureCoord");
	var uTime = gl.getUniformLocation(program, "uTime");
	var uWorldTime = gl.getUniformLocation(program, "uWorldTime");
	var uTexture = gl.getUniformLocation(program, "uTexture");
	
	//gl.enable(gl.TEXTURE_2D);
	gl.enable(gl.BLEND);
	var index = shader.bind(0, uTexture);
	
	gl.uniform1f(uTime, Jappsy.mod(localTime, 1));
	gl.uniform1f(uWorldTime, Jappsy.mod(worldTime, 1));
	
	gl.uniformMatrix4fv(uLayerProjectionMatrix, false, cam.projection16fv);
	
	gl.bindBuffer(gl.ARRAY_BUFFER, this.m_squareBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, this.makeRect(x1, y1, x2, y2), gl.DYNAMIC_DRAW);
	gl.vertexAttribPointer(aVertexPosition, 2, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(aVertexPosition);
	
	gl.bindBuffer(gl.ARRAY_BUFFER, this.m_textureBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, this.makeRect(0, 0, 1, 1), gl.DYNAMIC_DRAW);
	gl.vertexAttribPointer(aTextureCoord, 2, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(aTextureCoord);
	
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
	
	gl.disableVertexAttribArray(aVertexPosition);
	gl.disableVertexAttribArray(aTextureCoord);
	
	this.cleanup(index);
	//gl.disable(gl.TEXTURE_2D);
	gl.disable(gl.BLEND);
 */
}

void GLRender::drawEffectMobile(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const wchar_t* key, GLfloat localTime, GLfloat worldTime) {
	// TODO:
/*
	var cam = this.cameras.get("gui");
	cam.update();
 
	var shader = this.shaders.get(key);
	var program = shader.program;
	
	var uLayerProjectionMatrix = gl.getUniformLocation(program, "uLayerProjectionMatrix");
	var aVertexPosition = gl.getAttribLocation(program, "aVertexPosition");
	var aTextureCoord = gl.getAttribLocation(program, "aTextureCoord");
	var uTime = gl.getUniformLocation(program, "uTime");
	var uTexture0 = gl.getUniformLocation(program, "uTexture0");
	var uTexture1 = gl.getUniformLocation(program, "uTexture1");
 
	//gl.enable(gl.TEXTURE_2D);
	gl.enable(gl.BLEND);
	var index = shader.bind(0, [uTexture0, uTexture1]);
	
	gl.uniform2f(uTime, Jappsy.mod(localTime, 1), Jappsy.mod(worldTime, 1));
 
	gl.uniformMatrix4fv(uLayerProjectionMatrix, false, cam.projection16fv);
 
	gl.bindBuffer(gl.ARRAY_BUFFER, this.m_squareBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, this.makeRect(x1, y1, x2, y2), gl.DYNAMIC_DRAW);
	gl.vertexAttribPointer(aVertexPosition, 2, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(aVertexPosition);
	
	gl.bindBuffer(gl.ARRAY_BUFFER, this.m_textureBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, this.makeRect(0, 0, 1, 1), gl.DYNAMIC_DRAW);
	gl.vertexAttribPointer(aTextureCoord, 2, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(aTextureCoord);
	
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
 
	gl.disableVertexAttribArray(aVertexPosition);
	gl.disableVertexAttribArray(aTextureCoord);
 
	this.cleanup(index);
	//gl.disable(gl.TEXTURE_2D);
	gl.disable(gl.BLEND);
 */
}

bool GLRender::createShaders(JSONObject shaders) {
	Iterator<String> keys;
 
	keys = shaders.keys();
	while (keys.hasNext()) {
		String key = keys.next();
		JSONArray data = shaders.getJSONArray(key);
		String vsh = data.optString(0);
		String fsh = data.optString(1);
		THIS.shaders->createShader(key, vsh, fsh);
	}

	keys = THIS.shaders->keys();
	while (keys.hasNext()) {
		String key = keys.next();
		if (!THIS.shaders->get(key).checkReady()) {
			return false;
		}
	}
	keys.reset();
	while (keys.hasNext()) {
		String key = keys.next();
		GLShader shader = THIS.shaders->get(key);
	}
/*
	for (var key in this.shaders.list) {
		var shader = this.shaders.list[key];
		var program = shader.program;
		
		if (key == "sprite") {
			this.shaderSprite = shader;
			Jappsy.assign(shader, {
			uLayerProjectionMatrix: gl.getUniformLocation(program, "uLayerProjectionMatrix"),
			uPosition: gl.getUniformLocation(program, "uPosition"),
			uTexture: gl.getUniformLocation(program, "uTexture"),
			aVertexPosition: gl.getAttribLocation(program, "aVertexPosition"),
			aTextureCoord: gl.getAttribLocation(program, "aTextureCoord"),
				
			uLight: gl.getUniformLocation(program, "uLight"),
			uTime: gl.getUniformLocation(program, "uTime"),
			});
		} else if (key == "particle") {
			this.shaderParticle = shader;
			Jappsy.assign(shader, {
			uModelViewProjectionMatrix: gl.getUniformLocation(program, "uModelViewProjectionMatrix"),
				
			uPixelX: gl.getUniformLocation(program, "uPixelX"),
			uPixelY: gl.getUniformLocation(program, "uPixelY"),
			uTime: gl.getUniformLocation(program, "uTime"),
			uTexture: gl.getUniformLocation(program, "uTexture"),
			uColor: gl.getUniformLocation(program, "uColor"),
				
			aVertexPosition: gl.getAttribLocation(program, "aVertexPosition"),
			aTextureCoord: gl.getAttribLocation(program, "aTextureCoord"),
			aVelocity: gl.getAttribLocation(program, "aVelocity"),
			aAcceleration: gl.getAttribLocation(program, "aAcceleration"),
			aTime: gl.getAttribLocation(program, "aTime"),
			});
		} else if (key == "model") {
			this.shaderModel = shader;
			Jappsy.assign(shader, {
			uModelViewProjectionMatrix: gl.getUniformLocation(program, "uModelViewProjectionMatrix"),
			uModelViewMatrix: gl.getUniformLocation(program, "uModelViewMatrix"),
			uNormalMatrix: gl.getUniformLocation(program, "uNormalMatrix"),
				
			uAmbientLightColor: gl.getUniformLocation(program, "uAmbientLightColor"),
				
			uLightsCount: gl.getUniformLocation(program, "uLightsCount"),
			uLights: gl.getUniformLocation(program, "uLights"),
				
			uColors: gl.getUniformLocation(program, "uColors"),
			uTexture: gl.getUniformLocation(program, "uTexture"),
				
			aVertexPosition: gl.getAttribLocation(program, "aVertexPosition"),
			aTextureCoord: gl.getAttribLocation(program, "aTextureCoord"),
			aVertexNormal: gl.getAttribLocation(program, "aVertexNormal"),
			});
		} else if (key == "square_fill") {
			this.shaderSquareFill = shader;
			Jappsy.assign(shader, {
			uLayerProjectionMatrix: gl.getUniformLocation(program, "uLayerProjectionMatrix"),
			uColor: gl.getUniformLocation(program, "uColor"),
			aVertexPosition: gl.getAttribLocation(program, "aVertexPosition"),
			});
		} else if (key == "square_stroke") {
			this.shaderSquareStroke = shader;
			Jappsy.assign(shader, {
			uLayerProjectionMatrix: gl.getUniformLocation(program, "uLayerProjectionMatrix"),
			uCorners: gl.getUniformLocation(program, "uCorners"),
			uBorder: gl.getUniformLocation(program, "uBorder"),
			uColor: gl.getUniformLocation(program, "uColor"),
			aVertexPosition: gl.getAttribLocation(program, "aVertexPosition"),
			});
		} else if (key == "square_texture") {
			this.shaderSquareTexture = shader;
			Jappsy.assign(shader, {
			uLayerProjectionMatrix: gl.getUniformLocation(program, "uLayerProjectionMatrix"),
			uTexture: gl.getUniformLocation(program, "uTexture"),
			aVertexPosition: gl.getAttribLocation(program, "aVertexPosition"),
			aTextureCoord: gl.getAttribLocation(program, "aTextureCoord"),
			});
		}
	}
	 */
	
	return true;
}

void GLRender::createModels(JSONObject models) {
	
}

void GLRender::createSprites(JSONObject sprites) {
	
}

void GLRender::createDrawings(JSONObject drawings) {
	
}
