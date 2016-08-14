Jappsy.GLSprite = function(context, key, info) {
	this.context = context;
	this.texture = null;
	this.key = key;
	this.width = 0;
	this.height = 0;
	this.frames = 0;
	
	this.vertexBuffer = null;
	this.textureBuffer = null;
	this.indexBuffer = null;
	
	var texture = this.context.textures.get(info[0]);
	this.texture = texture;
	this.width = info[1][0];
	this.height = info[1][1];
	this.frames = info[3] || 1;
	
	var vertices = [];
	var textures = [];
	var indexes = [];
	
	var sw = this.width;
	var sh = this.height;
	var ox, oy, w, h;
	if (info[2]) {
		var tw = texture.width;
		var th = texture.height;
		ox = info[2][0] / tw;
		oy = info[2][1] / th;
		w = sw / tw;
		h = sh / th;
	} else {
		ox = oy = 0;
		w = h = 1;
	}

	var tx = ox, ty = oy;
	for (var i = 0; i < this.frames; i++) {
		var k = i*4;
		vertices.push(0,0,0,sh,sw,sh,sw,0);
		textures.push(tx,ty,tx,ty+h,tx+w,ty+h,tx+w,ty);
		indexes.push(k,k+1,k+2,k,k+2,k+3);

		if (info[4] !== undefined) {
			tx += w*info[4][0];
			ty += h*info[4][1];
		} else {
			ty += h;
			if ((ty+h) > 1.0) {
				ty = oy;
				tx += w;
			}
		}
	}

	var gl = this.context.gl;

	this.vertexBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);

	this.textureBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, this.textureBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(textures), gl.STATIC_DRAW);
	
	this.indexBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(indexes), gl.STATIC_DRAW);
};

Jappsy.GLSprite.prototype.release = function() {
	var gl = this.context.gl;
	
	this.context = null;
	this.texture = null;
	
	gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.vertexBuffer);
	this.vertexBuffer = null;

	gl.bindBuffer(gl.ARRAY_BUFFER, this.textureBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.textureBuffer);
	this.textureBuffer = null;

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.indexBuffer);
	this.indexBuffer = null;
};

Jappsy.GLSprite.prototype.getPosition = function(position, paint) {
	if (paint !== undefined) {
		position = [ position[0], position[1] ];
		if (paint.m_alignX == 0) {
			position[0] -= this.width / 2;
		} else if (paint.m_alignX > 0) {
			position[0] -= this.width;
		}
		if (paint.m_alignY == 0) {
			position[1] -= this.height / 2;
		} else if (paint.m_alignY > 0) {
			position[1] -= this.height;
		}
	}

	return position;
};

Jappsy.GLSprite.prototype.render = function(position, frame, paint, time) {
	var gl = this.context.gl;
	var shader = this.context.shaderSprite;
	var cam = this.context.cameras.gui;
	cam.update();

	gl.enable(gl.BLEND);
	gl.useProgram(shader.program);
	
	if (paint !== undefined) {
		gl.uniform4fv(shader.uLight, paint.m_color4iv);
	} else {
		gl.uniform4fv(shader.uLight, this.context.light);
	}
	
	if (time !== undefined) {
		gl.uniform1f(shader.uTime, time);
	} else {
		var time = (new Date).getTime() / 2000.0;
		gl.uniform1f(shader.uTime, time - Math.floor(time));
	}
	
	position = this.getPosition(position, paint);
	
	gl.uniformMatrix4fv(shader.uLayerProjectionMatrix, false, cam.projection16fv);
	gl.uniform2fv(shader.uPosition, new Float32Array(position));

	gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);
	gl.vertexAttribPointer(shader.aVertexPosition, 2, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(shader.aVertexPosition);
	
	gl.bindBuffer(gl.ARRAY_BUFFER, this.textureBuffer);
	gl.vertexAttribPointer(shader.aTextureCoord, 2, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray(shader.aTextureCoord);

	//gl.enable(gl.TEXTURE_2D);
	this.texture.bind(0, shader.uTexture);
	
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);
	gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 6 * frame * 2/*size of UNSIGNED_SHORT*/);

	gl.disableVertexAttribArray(shader.aVertexPosition);
	gl.disableVertexAttribArray(shader.aTextureCoord);

	gl.bindTexture(gl.TEXTURE_2D, null);
	//gl.disable(gl.TEXTURE_2D);
};

///////////////////////////

Jappsy.GLSprites = function(context) {
	this.context = context;
	this.list = {};
};

Jappsy.GLSprites.prototype.release = function() {
	for (var key in this.list) {
		this.list[key].release();
	}
	this.list = null;
	this.context = null;
};

Jappsy.GLSprites.prototype.get = function(key) {
	return this.list[key] || null;
};

Jappsy.GLSprites.prototype.createSprite = function(key, info) {
	var gl = this.context.gl;
	
	if (this.list[key] !== undefined)
		this.list[key].release();
	
	var sprite = new Jappsy.GLSprite(this.context, key, info);
	
	this.list[key] = sprite;
	
	return sprite;
};

Jappsy.GLSprites.prototype.renderSprite = function(key, position, frame, paint, time) {
	this.list[key].render(position, frame, paint, time);
};

Jappsy.GLSprites.prototype.renderSpriteNumber = function(key, position, step, value) {
	var s = value.toString(10);
	var p = s.length-1;
	var pos = [position[0], position[1]];
	if (step > 0) {
		pos[0] += step*p;
	} else {
		pos[0] += step*2;
		step = -step;
	}
	var sprite = this.list[key];
	for (; p >= 0; p--) {
		var v = s.charCodeAt(p) - 48;
		sprite.render(pos, v);
		pos[0] -= step;
	}
};