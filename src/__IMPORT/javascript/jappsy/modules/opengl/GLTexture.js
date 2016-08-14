Jappsy.assign(Jappsy, {
	GLImageStyleAuto: -1,
	GLImageStyleNormal: 0,
    GLImageStyleSmooth: 1,			// .s.
    GLImageStyleFlip: 2,			// .f.
    GLImageStyleRepeat: 3,			// .r.
    GLImageStylePatch9: 4,			// .9.
    GLImageStyleDistanceField: 5,
});

//===============================

Jappsy.GLTexture = function(context, handles, width, height) {
	this.context = context;
	this.handles = handles || [];
	this.width = width || 0;
	this.height = height || 0;
	this.handles1iv = null;
};

Jappsy.GLTexture.prototype.release = function() {
	if (this.handles.length != 0) {
		for (var i = 0; i < this.handles.length; i++) {
			this.context.textures.releaseTextureHandle(this.handles[i]);
		}
		this.width = 0;
		this.height = 0;
		this.handles = [];
	}
	this.handles1iv = null;
	this.context = null;
};

Jappsy.GLTexture.prototype.bind = function(index, uniform) {
	var gl = this.context.gl;

	var count = this.handles.length;
	if ((count > 0) && ((this.handles1iv === null) || (count != this.handles1iv.length))) {
		this.handles1iv = new Int32Array(count);
	}

	for (var i = 0; i < count; i++) {
		var handle = this.handles[i];
		this.context.activeTexture(index);
		gl.bindTexture(gl.TEXTURE_2D, handle);
		this.handles1iv[i] = index;
		index++;
	}
	
	if (uniform !== undefined) {
		if (count == 1) {
			gl.uniform1i(uniform, this.handles1iv[0]);
		} else if (count > 1) {
			gl.uniform1iv(uniform, this.handles1iv);
		}
	}
	
	return index;
};

//===============================

Jappsy.GLTextures = function(context) {
	this.context = context;
	this.list = {};
	this.defaultTexture = this.createSolidTexture("null", [0,0,0,0]);
};

Jappsy.GLTextures.prototype.release = function() {
	for (var key in this.list)
		this.list[key].release();
	this.defaultTexture = null;
	this.list = {};
	this.context = null;
};

Jappsy.GLTextures.prototype.get = function (key) {
	return this.list[key] || null;
};

//===============================

Jappsy.GLTextures.prototype.createTextureFromImg = function (key, image) {
	var gl = this.context.gl;
	
	var handle = gl.createTexture();
	
	if (handle == null)
		return null;

	gl.bindTexture(gl.TEXTURE_2D, handle);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);

    if (gl.getError() == gl.OUT_OF_MEMORY) {
		gl.deleteTexture(handle);
		return null;
    }

	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);//LINEAR_MIPMAP_NEAREST); FIXED: Параметр вызывал на стыках текстур артефакты
	//gl.generateMipmap(gl.TEXTURE_2D);

    if (gl.getError() == gl.OUT_OF_MEMORY) {
		gl.deleteTexture(handle);
		return null;
    }

	gl.bindTexture(gl.TEXTURE_2D, null);
	
	var texture = new Jappsy.GLTexture(this.context, [handle], image.width, image.height);
	
	if (this.list[key] !== undefined)
		this.list[key].release();
	this.list[key] = texture;
	
	if (key == "null")
		this.defaultTexture = handle;
	
	return texture;
};

Jappsy.GLTextures.prototype.createSolidTexture = function (key, rgba4fv) {
	var gl = this.context.gl;

    var handle = gl.createTexture();
    
	if (handle == null)
		return null;

    var data = new Uint8Array([rgba4fv[0] * 255, rgba4fv[1] * 255, rgba4fv[2] * 255, rgba4fv[3] * 255]);
    
    gl.bindTexture(gl.TEXTURE_2D, handle);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, data);

    if (gl.getError() == gl.OUT_OF_MEMORY) {
		gl.deleteTexture(handle);
		return null;
    }

	var texture = new Jappsy.GLTexture(this.context, [handle], 1, 1);

    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    
	if (this.list[key] !== undefined)
		this.list[key].release();
	this.list[key] = texture;
	
	if (key == "null")
		this.defaultTexture = handle;
	
	return texture;
}

Jappsy.GLTextures.prototype.createTexture = function (key, handles, width, height) {
	var texture = new Jappsy.GLTexture(this.context, handles, width, height);
	
	if (key !== null) {
		if (this.list[key] !== undefined)
			this.list[key].release();
		this.list[key] = texture;

		if (key == "null")
			this.defaultTexture = handles[0];
	}
	
	return texture;
};

//===============================

Jappsy.GLTextures.prototype.createTextureHandle = function (width, height, style, data) {
	var gl = this.context.gl;
	
	var handle = gl.createTexture();
	
	if (handle == null)
		return null;
	
	gl.bindTexture(gl.TEXTURE_2D, handle);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, data);
    
    if (gl.getError() == gl.OUT_OF_MEMORY) {
		gl.deleteTexture(handle);
		return null;
    }
    
    if (style & 0x01) { // Smooth
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	} else {
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
	}
	if (style & 0x02) { // Repeat
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
	} else {
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
	}
	
	return handle;
};

Jappsy.GLTextures.prototype.releaseTextureHandle = function (handle) {
	var gl = this.context.gl;
	
	gl.deleteTexture(this.handle);
};
