Jappsy.assign(Jappsy, {
	DataStyleNone: 0,
	DataStyleObject: 1,
	DataStyleRefString: 2,
	DataStyleRefObject: 3,
});

Jappsy.Data = function(data, style) {
	this.data = data;
	this.style = style || Jappsy.DataStyleNone;
};

Jappsy.Data.prototype.update = function(data, style) {
	this.data = data;
	this.style = style;
	return this;
};

JappsyReader = {
	SDFFHEAD: 0x46464453,	// Signed Distance Field Font Head
	SDFIHEAD: 0x49464453,	// Signed Distance Field Image Head
	JIMGHEAD: 0x474D494A,	// Jappsy Image Head
	JSHDHEAD: 0x4448534A,	// Jappsy Shader Head
	
	IGZCHUNK: 0x5A472E49,	// GZip Pixel Data Chunk
	IRWCHUNK: 0x57522E49,	// Raw Pixel Data Chunk
	
	VGZCHUNK: 0x475A2E56,	// GZip Vertex Shader Data Chunk
	VSHCHUNK: 0x48532E56,	// Raw Vertex Shader Data Chunk
	VRFCHUNK: 0x46522E56,	// String Reference Chunk to Vertex Shader
	FGZCHUNK: 0x475A2E46,	// GZip Font Data Chunk | GZip Frame Shader Data Chunk
	FSHCHUNK: 0x48532E46,	// Raw Frame Shader Data Chunk
	FRFCHUNK: 0x46522E46,	// String Reference Chunk to Frame Shader 
	IHDCHUNK: 0x44482E49,	// Jappsy Image Head Chunk
	IRFCHUNK: 0x46522E49,	// String Reference Chunk to Image
	
	JENDCHUNK: 0x444E454A,	// Jappsy End Chunk

	getSize: function(stream) {
		stream.position = 0;
		var head = stream.readInt();
		if ((head == JappsyReader.SDFFHEAD) || (head == JappsyReader.SDFIHEAD) || (head == JappsyReader.JIMGHEAD)) {
			var result = [];
			result.push(stream.readInt());
			result.push(stream.readInt());
			if (head != JappsyReader.JIMGHEAD) {
				result.push(0);
				result.push(stream.readUnsignedByte());
			} else {
				result.push(stream.readInt());
				result.push(0);
			}
			return result;
		}
		throw new Error('Unsupported format of file');
		return null;
	},
	
	createTexture: function(ctx, key, data) {
		var stream = new Jappsy.Stream(data);
		stream.position = 0;
		var head = stream.readInt();
		if ((head == JappsyReader.SDFFHEAD) || (head == JappsyReader.SDFIHEAD) || (head == JappsyReader.JIMGHEAD)) {
			var width = stream.readInt();
			var height = stream.readInt();
			var style = 0;
			var distance = 0;
			var handles = [];
			
			if (head != JappsyReader.JIMGHEAD) {
				distance = stream.readUnsignedByte();
			} else {
				style = stream.readInt();
			}

			var len = stream.readInt();
			var chunk = stream.readInt();
			while (chunk != JappsyReader.JENDCHUNK) {
				if (chunk == JappsyReader.IGZCHUNK) {
					var data = Jappsy.inflate( stream.readBytes(len) );
					handles.push( ctx.textures.createTextureHandle(width, height, style, data) );
				} else if (chunk == JappsyReader.IRWCHUNK) {
					var data = stream.readBytes(len);
					handles.push( ctx.textures.createTextureHandle(width, height, style, data) );
				} else {
					if (stream.skip(len) < 0)
						break;
				}
				
				len = stream.readInt();
				chunk = stream.readInt();
			}
			
			if (handles.length > 0) {
				return ctx.textures.createTexture(key, handles, width, height);
			} else
				throw new Error('Unexpected end of file');
		}
		throw new Error('Unsupported format of file');
		return null;
	},
	
	createShader: function(ctx, key, data) {
		var stream = new Jappsy.Stream(data);
		stream.position = 0;
		var head = stream.readInt();
		if (head == JappsyReader.JSHDHEAD) {
			var vsh = null;
			var fsh = null;
			var program = null;
			var textures = [];
			var error = null;
			
			var len = stream.readInt();
			var chunk = stream.readInt();
			while (chunk != JappsyReader.JENDCHUNK) {
				if (chunk == JappsyReader.VGZCHUNK) {
					var sh = ctx.shaders.createVertexShader(Jappsy.buf2binstring(Jappsy.inflate(stream.readBytes(len))));
					if (sh !== null)
						vsh = new Jappsy.Data( sh, Jappsy.DataStyleObject );
					else {
						error = 'Unexpected content of file';
						break;
					}
				} else if (chunk == JappsyReader.VSHCHUNK) {
					var sh = ctx.shaders.createVertexShader(Jappsy.buf2binstring(stream.readBytes(len)));
					if (sh !== null)
						vsh = new Jappsy.Data( sh, Jappsy.DataStyleObject );
					else {
						error = 'Unexpected content of file';
						break;
					}
				} else if (chunk == JappsyReader.VRFCHUNK) {
					vsh = new Jappsy.Data( Jappsy.buf2binstring(stream.readBytes(len)), Jappsy.DataStyleRefString );
				} else
				
				if (chunk == JappsyReader.FGZCHUNK) {
					var sh = ctx.shaders.createFragmentShader(Jappsy.buf2binstring(Jappsy.inflate(stream.readBytes(len))));
					if (sh !== null)
						fsh = new Jappsy.Data( sh, Jappsy.DataStyleObject );
					else {
						error = 'Unexpected content of file';
						break;
					}
				} else if (chunk == JappsyReader.FSHCHUNK) {
					var sh = ctx.shaders.createFragmentShader(Jappsy.buf2binstring(stream.readBytes(len)));
					if (sh !== null)
						fsh = new Jappsy.Data( sh, Jappsy.DataStyleObject );
					else {
						error = 'Unexpected content of file';
						break;
					}
				} else if (chunk == JappsyReader.FRFCHUNK) {
					fsh = new Jappsy.Data( Jappsy.buf2binstring(stream.readBytes(len)), Jappsy.DataStyleRefString );
				} else

				if (chunk == JappsyReader.IHDCHUNK) {
					var width = stream.readInt();
					var height = stream.readInt();
					var style = stream.readInt();
					
					len = stream.readInt();
					chunk = stream.readInt();
					if (chunk == JappsyReader.IGZCHUNK) {
						var data = Jappsy.inflate( stream.readBytes(len) );
						textures.push( new Jappsy.Data( ctx.textures.createTextureHandle(width, height, style, data), Jappsy.DataStyleObject) );
					} else if (chunk == JappsyReader.IRWCHUNK) {
						var data = stream.readBytes(len);
						textures.push( new Jappsy.Data( ctx.textures.createTextureHandle(width, height, style, data), Jappsy.DataStyleObject) );
					} else {
						error = 'Unsupported format of file';
						break;
					}
				} else if (chunk == JappsyReader.IRFCHUNK) {
					textures.push( new Jappsy.Data( Jappsy.buf2binstring(stream.readBytes(len)), Jappsy.DataStyleRefString ) );
				} else {
					if (stream.skip(len) < 0)
						break;
				}
				
				len = stream.readInt();
				chunk = stream.readInt();
			}

			if ((vsh == null) && (fsh == null)) {
				error = 'Unexpected end of file';
			} else if ((vsh != null) && (fsh != null) && (vsh.style == Jappsy.DataStyleObject) && (fsh.style == Jappsy.DataStyleObject)) {
				program = ctx.shaders.createProgram(vsh.data, fsh.data);
				if (program == null) {
					error = 'Unexpected content of file';
				}
			}
			
			if (error !== null) {
				if ((vsh !== null) && (vsh.style == Jappsy.DataStyleObject)) ctx.shaders.releaseShader(vsh);
				if ((fsh !== null) && (fsh.style == Jappsy.DataStyleObject)) ctx.shaders.releaseShader(fsh);
				if (program != null) ctx.shaders.releaseProgram(program);
				if (textures.length > 0) {
					for (var i = 0; i < textures.length; i++) {
						if (textures[i].style == Jappsy.DataStyleObject)
							ctx.textures.releaseTextureHandle( textures[i].data );
					}
				}
				throw new Error(error);
			}
			
			return ctx.shaders.createShader(key, vsh, fsh, program, textures);
		}
		throw new Error('Unsupported format of file');
		return null;
	},
};
