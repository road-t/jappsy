Jappsy.GLContextFrame = function(engine, context, callback, canvas) {
	var gl = null;
	try {
		gl = canvas.getContext("experimental-webgl", {
			preserveDrawingBuffer: true,
			alpha: false
		});
	}
	catch(e) {
	}
	if (gl == null) {
		throw 'Experemental WebGL Error:Unsupported Browser!'
	}

	this.engine = engine;
	this.context = context;
	
	this.currentTime = this.lastFrame = new Date().getTime();
	this.frames = new Jappsy.SmoothValue(10);
	this.fps = null;
	
	this.onFrame = callback;
	this.oCanvas = canvas;
	this.gl = gl;
	this.requestId = null;
	
	if (this.engine.config.FPSLIMIT != 0) {
		this.requestAnimationFrame = this.onLimitFrame;
		this.cancelAnimationFrame = this.onCancelFrame;
	} else {
		this.requestAnimationFrame =
			window.requestAnimationFrame ||
			window.webkitRequestAnimationFrame ||
			window.mozRequestAnimationFrame ||
			window.oRequestAnimationFrame ||
			window.msRequestAnimationFrame;
		
		this.cancelAnimationFrame =
			window.cancelAnimationFrame ||
			window.webkitCancelAnimationFrame ||
			window.mozCancelAnimationFrame ||
			window.oCancelAnimationFrame ||
			window.msCancelAnimationFrame;

		if (!this.requestAnimationFrame) {
			this.requestAnimationFrame = this.onUnlimitFrame;
			this.cancelAnimationFrame = this.onCancelFrame;
		} else {
			this.requestAnimationFrame = this.requestAnimationFrame.bind(window);
			this.cancelAnimationFrame = this.cancelAnimationFrame.bind(window);
		}
	}
	
	this.onLoop = this.loop.bind(this);
	
	this.start();
};

Jappsy.GLContextFrame.defaultFps = 30;

Jappsy.GLContextFrame.prototype.release = function() {
	this.stop();
	
	{
		// Unassign all attributes
		var gl = this.context.gl;
		var tempBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, tempBuffer);
		var numAttributes = gl.getParameter(gl.MAX_VERTEX_ATTRIBS);
		for (var attrib = 0; attrib < numAttributes; ++attrib) {
			gl.vertexAttribPointer(attrib, 1, gl.FLOAT, false, 0, 0);
		}
		gl.deleteBuffer(tempBuffer);
		
		// Clean OpenGL Memory
		var w = gl.canvas.width;
		var h = gl.canvas.height;
		gl.canvas.width = 1;
		gl.canvas.height = 1;
		gl.canvas.width = w;
		gl.canvas.height = h;
	}
	
	this.engine = null;
	this.context = null;
	this.currentTime = 0;
	this.lastFrame = 0;
	this.frames = null;
	this.fps = null;
	this.onFrame = null;
	this.oCanvas = null;
	this.gl = null;
	this.requestId = null;
	
	this.requestAnimationFrame = null;
	this.cancelAnimationFrame = null;
	this.onLoop = null;
};

Jappsy.GLContextFrame.prototype.onLimitFrame = function(callback) {
	return setTimeout(callback, 1000 / this.engine.config.FPSLIMIT);
};

Jappsy.GLContextFrame.prototype.onUnlimitFrame = function(callback) {
	return setTimeout(callback, 1000 / Jappsy.GLContextFrame.defaultFps);
};

Jappsy.GLContextFrame.prototype.onCancelFrame = function(requestId) {
	return clearTimeout(requestId);
};

Jappsy.GLContextFrame.prototype.loop = function() {
	this.currentTime = new Date().getTime();
	var elapsed = this.currentTime - this.lastFrame;
	if (elapsed > 0)
		this.frames.put(1000.0 / elapsed);
	this.fps = Math.floor(this.frames.value());
	
	this.onFrame(this.context, this.oCanvas, this.gl);
	
	this.lastFrame = this.currentTime;
	this.requestId = this.requestAnimationFrame(this.onLoop);
};

Jappsy.GLContextFrame.prototype.start = function() {
	if (this.requestId === null) {
		this.requestId = this.requestAnimationFrame(this.onLoop);
	}
};

Jappsy.GLContextFrame.prototype.stop = function() {
	if (this.requestId !== null) {
		this.cancelAnimationFrame(this.requestId);
		this.requestId = null;
	}
};

///////////////////////////

Jappsy.GLContext = function(engine, onframe, ontouch, canvas) {
	this.engine = engine;

	this.frame = new Jappsy.GLContextFrame(engine, this, onframe, canvas);
	this.touchScreen = ontouch ? new Jappsy.TouchScreen(this, ontouch, canvas) : null;
	this.loader = new Jappsy.Loader(this);

	var gl = this.frame.gl;
	this.gl = gl;
	
	this.m_square = new Float32Array(8);
	this.m_line = new Float32Array(4);
	this.m_color = new Float32Array(20);
	
	this.textures = new Jappsy.GLTextures(this);
	this.shaders = new Jappsy.GLShaders(this);
	this.sprites = new Jappsy.GLSprites(this);
	this.scenes = new Jappsy.GLScenes(this);
	this.cameras = new Jappsy.GLCameras(this);
	this.models = new Jappsy.GLModels(this);
	this.particles = new Jappsy.GLParticles(this);
	this.drawings = new Jappsy.GLDrawings(this);

	this.cameras.createCamera("gui")
		.size(canvas.width, canvas.height)
		.layer(0, 0);
	
	this.lightsMaxCount = 6;
	
	this.shaderSprite = null;
	this.shaderParticle = null;
	this.shaderModel = null;
	this.shaderSquareFill = null;
	this.shaderSquareStroke = null;
	this.shaderSquareTexture = null;

	this.maxTextureSize = gl.getParameter(gl.MAX_TEXTURE_SIZE);
	this.isNPOTSupported = gl.getExtension("GL_OES_texture_npot");
	
	gl.clearColor(0.0, 0.0, 0.0, 1.0);
	gl.clear(gl.COLOR_BUFFER_BIT);
	gl.clearDepth(1.0);
	gl.depthFunc(gl.LEQUAL);
	this.resetBlend();

	this.m_squareBuffer = gl.createBuffer();
	this.m_textureBuffer = gl.createBuffer();
	this.m_normalBuffer = gl.createBuffer();
	this.m_indexBuffer = gl.createBuffer();
	
	this.light = new Vec4([1,1,1,1]);
};

Jappsy.GLContext.prototype.release = function() {
	this.loader.release();
	if (this.touchScreen !== null) {
		this.touchScreen.release();
		this.touchScreen = null;
	}
	this.frame.release();
	
	this.light = null;
	
	var gl = this.gl;
	
	gl.bindBuffer(gl.ARRAY_BUFFER, this.m_squareBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.m_squareBuffer);
	this.m_squareBuffer = null;
	
	gl.bindBuffer(gl.ARRAY_BUFFER, this.m_textureBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.m_textureBuffer);
	this.m_textureBuffer = null;

	gl.bindBuffer(gl.ARRAY_BUFFER, this.m_normalBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.m_normalBuffer);
	this.m_normalBuffer = null;

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.m_indexBuffer);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.m_indexBuffer);
	this.m_indexBuffer = null;
	
	this.shaderSprite = null;
	this.shaderParticle = null;
	this.shaderModel = null;
	this.shaderSquareFill = null;
	this.shaderSquareStroke = null;
	this.shaderSquareTexture = null;
	
	this.drawings.release();
	this.particles.release();
	this.models.release();
	this.cameras.release();
	this.scenes.release();
	this.sprites.release();
	this.shaders.release();
	this.textures.release();
	
	this.drawings = null;
	this.particles = null;
	this.models = null;
	this.cameras = null;
	this.scenes = null;
	this.sprites = null;
	this.textures = null;
	this.shaders = null;
	
	this.m_square = null;
	this.m_line = null;
	this.m_color = null;
	
	this.gl = null;
	this.engine = null;

	this.frame = null;
	this.loader = null;
};

Jappsy.GLContext.prototype.resetBlend = function() {
	var gl = this.gl;

	//gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
	gl.blendEquation(gl.FUNC_ADD);
};

Jappsy.GLContext.prototype.makeRect = function (x1, y1, x2, y2) {
	this.m_square[0] = this.m_square[4] = x1;
	this.m_square[1] = this.m_square[3] = y1;
	this.m_square[2] = this.m_square[6] = x2;
	this.m_square[5] = this.m_square[7] = y2;
	return this.m_square;
};

Jappsy.GLContext.prototype.makeLine = function (x1, y1, x2, y2) {
	this.m_line[0] = x1;
	this.m_line[1] = y1;
	this.m_line[2] = x2;
	this.m_line[3] = y2;
	return this.m_line;
};

Jappsy.GLContext.prototype.makeColor = function (color, count) {
	if (typeof color === 'number') {
		return new Float32Array([
			((color >> 16) & 0xFF) / 255.0,		// R
			((color >> 8) & 0xFF) / 255.0,		// G
			(color & 0xFF) / 255.0,				// B
			((color >> 24) & 0xFF) / 255.0		// A
		]);
	} else {
		var ofs = 0;
		for (var i in color) {
			this.m_color[ofs + 0] = ((color[i] >> 16) & 0xFF) / 255.0;		// R
			this.m_color[ofs + 1] = ((color[i] >> 8) & 0xFF) / 255.0;		// G
			this.m_color[ofs + 2] = (color[i] & 0xFF) / 255.0;				// B
			this.m_color[ofs + 3] = ((color[i] >> 24) & 0xFF) / 255.0;		// A
			ofs += 4;
		}
		return new Float32Array(this.m_color.slice(0, ofs));
	}
};

Jappsy.GLContext.prototype.fill = function (color) {
	var gl = this.gl;
	
	var c = this.makeColor(color);
	gl.clearColor(c[0], c[1], c[2], c[3]);
	gl.clear(gl.COLOR_BUFFER_BIT);
};

Jappsy.GLContext.prototype.fillAlpha = function (alpha) {
	var gl = this.gl;
	
	gl.colorMask(false, false, false, true);
	gl.clearColor(0.0, 0.0, 0.0, alpha / 255.0);
	gl.clear(gl.COLOR_BUFFER_BIT);
	gl.colorMask(true, true, true, true);
};

Jappsy.GLContext.prototype.fillDepth = function () {
	var gl = this.gl;
	
	gl.clear(gl.DEPTH_BUFFER_BIT);
};

Jappsy.GLContext.prototype.drawRect = function (x1, y1, x2, y2, paint) {
	var gl = this.gl;
	var cam = this.cameras.get("gui");
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
};

Jappsy.GLContext.prototype.activeTexture = function(index) {
	var gl = this.gl;
	
	switch (index) {
		case 0: gl.activeTexture(gl.TEXTURE0); break;
		case 1: gl.activeTexture(gl.TEXTURE1); break;
		case 2: gl.activeTexture(gl.TEXTURE2); break;
		default: gl.activeTexture(gl.TEXTURE3); break;
	}
};

Jappsy.GLContext.prototype.cleanup = function(index) {
	var gl = this.gl;
	
	for (var i = 0; i < index; i++) {
		this.activeTexture(i);
		gl.bindTexture(gl.TEXTURE_2D, null);
	}
	
	gl.useProgram(null);
};

Jappsy.GLContext.prototype.drawTexture = function(x1, y1, x2, y2, key) {
	var gl = this.gl;
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
};

Jappsy.GLContext.prototype.drawEffect = function(x1, y1, x2, y2, key, localTime, worldTime) {
	var gl = this.gl;
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
};

Jappsy.GLContext.prototype.drawEffectMobile = function(x1, y1, x2, y2, key, localTime, worldTime) {
	var gl = this.gl;
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
};












Jappsy.GLContext.prototype.createShaders = function (shaders) {
	var gl = this.gl;

	for (var key in shaders) {
		var shader = this.shaders.createShader(key,
			new Jappsy.Data(shaders[key][0], Jappsy.DataStyleRefString),
			new Jappsy.Data(shaders[key][1], Jappsy.DataStyleRefString),
			null,
			[]);
		if (shader == null)
			return false;
	}
	
	for (var key in this.shaders.list) {
		var shader = this.shaders.list[key];
		if (!shader.checkReady())
			return false;
	}
	
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
	
	return true;
};

Jappsy.GLContext.prototype.createModels = function (models) {
	for (var key in models) {
		this.models.createModel(key, models[key]);
	}
};

Jappsy.GLContext.prototype.createSprites = function (sprites) {
	for (var key in sprites) {
		var sprite = this.sprites.createSprite( key, sprites[key] );
	}
};

Jappsy.GLContext.prototype.createDrawings = function (drawings) {
	for (var key in drawings) {
		var drawing = this.drawings.createDrawing( key, drawings[key] );
	}
};
