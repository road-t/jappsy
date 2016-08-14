// Requires GLContext

Jappsy.GLShader = function(context, vsh, fsh, program, textures) {
	this.context = context;
	this.vsh = vsh;
	this.fsh = fsh;
	this.program = program;
	this.textures = textures;
	this.handles1iv = null;
};

Jappsy.GLShader.prototype.release = function() {
	if (this.textures.length != 0) {
		for (var i = 0; i < this.textures.length; i++) {
			var texture = this.textures[i];
			if (texture.style == Jappsy.DataStyleObject)
				this.context.textures.releaseTextureHandle(texture.data);
		}
		this.textures = [];
	}
	if (this.program != null) {
		this.context.shaders.releaseProgram(this.program);
		this.program = null;
	}
	if (this.vsh != null) {
		if (this.vsh.style == Jappsy.DataStyleObject)
			this.context.shaders.releaseShader(this.vsh.data);
		this.vsh = null;
	}
	if (this.fsh != null) {
		if (this.fsh.style == Jappsy.DataStyleObject)
			this.context.shaders.releaseShader(this.fsh.data);
		this.fsh = null;
	}
	this.context = null;
};

Jappsy.GLShader.prototype.checkReady = function() {
	if ((this.program == null) && (this.vsh != null) && (this.fsh != null)) {
		if (this.vsh.style == Jappsy.DataStyleRefString) {
			var vsh = this.vsh;
			do {
				var shader = this.context.shaders.get(this.vsh.data);
				if (shader != null) {
					vsh = shader.vsh;
				} else {
					vsh = null;
					break;
				}
			} while (vsh.style == Jappsy.DataStyleRefString);
			if (vsh != null) {
				this.vsh.update(vsh.data, Jappsy.DataStyleRefObject);
			} else
				return false;
		}
		
		if (this.fsh.style == Jappsy.DataStyleRefString) {
			var fsh = this.fsh;
			do {
				var shader = this.context.shaders.get(this.fsh.data);
				if (shader != null) {
					fsh = shader.fsh;
				} else {
					fsh = null;
					break;
				}
			} while (fsh.style == Jappsy.DataStyleRefString);
			if (fsh != null) {
				this.fsh.update(fsh.data, Jappsy.DataStyleRefObject);
			} else
				return false;
		}
		
		this.program = this.context.shaders.createProgram(this.vsh.data, this.fsh.data);
		if (this.program == null) {
			return false;
		}
	}
	
	if (this.textures.length != 0) {
		for (var i = this.textures.length-1; i >= 0; i--) {
			if (this.textures[i].style == Jappsy.DataStyleRefString) {
				var texture = this.context.textures.get(this.textures[i].data);
				if (texture != null) {
					// insert with replace
					var textures = [];
					for (var j = 0; j < texture.handles.length; j++) {
						textures.push( new Jappsy.Data(texture.handles[j], Jappsy.DataStyleRefObject) );
					}
					this.textures.splice.apply(this.textures, [i, 1].concat(textures));
				} else {
					return false;
				}
			}
		}
	}
	
	return true;
};

Jappsy.GLShader.prototype.bind = function(index, uniform) {
	var gl = this.context.gl;
	
	if (this.program != null) {
		gl.useProgram(this.program);
	}
	
	var count = this.textures.length;
	if ((count > 0) && ((this.handles1iv === null) || (count != this.handles1iv.length))) {
		this.handles1iv = new Int32Array(count);
	}

	for (var i = 0; i < count; i++) {
		var handle = this.textures[i].data;
		this.context.activeTexture(index);
		gl.bindTexture(gl.TEXTURE_2D, handle);
		this.handles1iv[i] = index;
		index++;
	}
	
	if (uniform !== undefined) {
		if (Array.isArray(uniform)) {
			if (count == 1) {
				gl.uniform1i(uniform[0], this.handles1iv[0]);
			} else if (count > 1) {
				if (count > uniform.length) count = uniform.length;
				for (var i = 0; i < count; i++) {
					gl.uniform1i(uniform[i], this.handles1iv[i]);
				}
			}
		} else {
			if (count == 1) {
				gl.uniform1i(uniform, this.handles1iv[0]);
			} else if (count > 1) {
				gl.uniform1iv(uniform, this.handles1iv);
			}
		}
	}
	
	return index;
};

//===============================

Jappsy.GLShaders = function(context) {
	this.context = context;
	this.list = {};
};

Jappsy.GLShaders.prototype.release = function() {
	for (var key in this.list)
		this.list[key].release();
	this.list = {};
	this.context = null;
};

Jappsy.GLShaders.prototype.get = function (key) {
	return this.list[key] || null;
};

Jappsy.GLShaders.prototype.createShader = function (key, vsh, fsh, program, textures) {
	var shader = new Jappsy.GLShader(this.context, vsh, fsh, program, textures);
	
	if (this.list[key] !== undefined)
		this.destroyShader(this.list[key]);
	this.list[key] = shader;
	
	return shader;
};

//===============================

Jappsy.GLShaders.prototype.createVertexShader = function (vertexShaderSource) {
	var gl = this.context.gl;
	
	var vertexShader = gl.createShader(gl.VERTEX_SHADER);
	gl.shaderSource(vertexShader, vertexShaderSource);
	gl.compileShader(vertexShader);
	
	if (!gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS)) {
		alert("OpenGL Vertex Shader: " + gl.getShaderInfoLog(vertexShader));
		gl.deleteShader(vertexShader);
		return null;
	}
	
	return vertexShader;
};

Jappsy.GLShaders.prototype.createFragmentShader = function (fragmentShaderSource) {
	var gl = this.context.gl;
	
	var fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
	gl.shaderSource(fragmentShader, fragmentShaderSource);
	gl.compileShader(fragmentShader);
	
	if (!gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS)) {
		alert("OpenGL Fragment Shader: " + gl.getShaderInfoLog(fragmentShader));
		gl.deleteShader(fragmentShader);
		return null;
	}
	
	return fragmentShader;
};

Jappsy.GLShaders.prototype.releaseShader = function (shader) {
	var gl = this.context.gl;
	
	gl.deleteShader(shader);
};

Jappsy.GLShaders.prototype.createProgram = function (vertexShader, fragmentShader) {
	var gl = this.context.gl;
	
	var program = gl.createProgram();
	gl.attachShader(program, vertexShader);
	gl.attachShader(program, fragmentShader);
	gl.linkProgram(program);
	
	if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
		alert("OpenGL Program: " + gl.getProgramInfoLog(program));
		gl.deleteProgram(program);
		return null;
	}
	
	return program;
};

Jappsy.GLShaders.prototype.releaseProgram = function (program) {
	var gl = this.context.gl;

	gl.deleteProgram(program);
};
