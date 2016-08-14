// Requires GLCache

Jappsy.assign(Jappsy, {
	UVMAP_DEFAULT: 0,
	UVMAP_PLANAR: 1,
	UVMAP_BOX: 2,
	UVMAP_CYLINDER: 3,
	UVMAP_SPHERE: 4,
});

Jappsy.GLMaterialTexture = function( name, blend, handle ) {
	this.file = name || "";
	this.blend = blend || 0;
	this.texture1iv = handle || null;

	this.x = 0.0;
	this.y = 0.0;
	this.w = 1.0;
	this.h = 1.0;
	this.uvmap = Jappsy.UVMAP_DEFAULT;
};

Jappsy.GLMaterialTexture.prototype.release = function() {
	this.file = null;
	this.texture1iv = null;
};

Jappsy.GLMaterialTexture.prototype.texture = function(context, key) {
	if (key == null) {
		this.texture1iv = null;
	} else {
		var texture = context.textures.get(key);
		this.texture1iv = (texture ? texture.handles[0] : null);
	}
	return this;
};

Jappsy.GLMaterialTexture.prototype.map = function(x, y, w, h, uvmap) {
	this.x = x || 0.0;
	this.y = y || 0.0;
	this.w = w || 1.0;
	this.h = h || 1.0;
	this.uvmap = uvmap || Jappsy.UVMAP_DEFAULT;
	return this;
};

//===============================

Jappsy.GLMaterial = function(name, texture) {
	this.name = name || "";
		
	// Ambient color is the color of an object where it is in shadow.
	// This color is what the object reflects when illuminated by ambient light rather than direct light.
	// 3DSMax - Ambient
	this.ambient = { color: [0, 0, 0] };
		
	// Diffuse color is the most instinctive meaning of the color of an object.
	// It is that essential color that the object reveals under pure white light.
	// It is perceived as the color of the object itself rather than a reflection of the light.
	// 3DSMax - Diffuse
	this.diffuse = { color: [1, 1, 1], texture: texture || new Jappsy.GLMaterialTexture() };
		
	// Specular color is the color of the light of a specular reflection.
	// Specular reflection is the type of reflection that is characteristic of light reflected from a shiny surface.
	// 3DSMax - Specular
	this.specular = { color: [1, 1, 1], texture: new Jappsy.GLMaterialTexture() };
		
	// Emissive color is the self-illumination color an object has.
	// 3DSMax - Self-Illumination
	this.emissive = { color: [0, 0, 0], texture: new Jappsy.GLMaterialTexture() };
	
	// 3DSMax - (value) Glossiness, (percent) Specular Level, (texture alpha channel) Specular Level Blend
	this.shininess = { value: 50, percent: 1, texture: new Jappsy.GLMaterialTexture() };
	
	// 3DSMax - Opacity
	this.opacity = { value: 1, texture: new Jappsy.GLMaterialTexture() };
	
	// 3DSMax - Bump
	this.bump = { scale: 1, texture: new Jappsy.GLMaterialTexture() };
	
	// 3DSMax - Reflection
	this.reflection = { };
	
	this.colors4fv = new Float32Array(20);
	
	this.update = function() {
		this.colors4fv[0] = this.ambient.color[0];
		this.colors4fv[1] = this.ambient.color[1];
		this.colors4fv[2] = this.ambient.color[2];
		this.colors4fv[3] = this.opacity.value;
		
		this.colors4fv[4] = this.diffuse.color[0];
		this.colors4fv[5] = this.diffuse.color[1];
		this.colors4fv[6] = this.diffuse.color[2];
		this.colors4fv[7] = this.diffuse.texture.blend;
		
		this.colors4fv[8] = this.specular.color[0];
		this.colors4fv[9] = this.specular.color[1];
		this.colors4fv[10] = this.specular.color[2];
		this.colors4fv[11] = this.specular.texture.blend;
		
		this.colors4fv[12] = this.emissive.color[0];
		this.colors4fv[13] = this.emissive.color[1];
		this.colors4fv[14] = this.emissive.color[2];
		this.colors4fv[15] = this.emissive.texture.blend;

		this.colors4fv[16] = this.shininess.value;
		this.colors4fv[17] = this.shininess.percent;
		this.colors4fv[18] = 0;
		this.colors4fv[19] = this.opacity.texture.blend;
	};
};

Jappsy.GLMaterial.prototype.release = function() {
	this.name = null;
	this.diffuse.texture = null;
	this.specular.texture = null;
	this.emissive.texture = null;
	this.shininess.texture = null;
	this.opacity.texture = null;
	this.bump.texture = null;
	this.colors4fv = null;
};

//===============================

Jappsy.GLModelNode = function(name, transformation) {
	this.name = name || "";
	this.transformation = transformation || new Mat4().identity();
};

Jappsy.GLModelNode.prototype.release = function(gl) {
	this.name = null;
	this.transformation = null;

	if (this.nodes !== undefined) {
		for (var i in this.nodes) {
			this.nodes[i].release(gl);
		}
		delete this.nodes;
	} else if (this.meshes !== undefined) {
		for (var i in this.meshes) {
			this.meshes[i].release(gl);
		}
		delete this.meshes;
	}	
};

Jappsy.GLModelNode.prototype.insertNode = function(node) {
	if (this.meshes === undefined) {
		if (this.nodes === undefined) {
			this.nodes = [];
		}
		this.nodes.push(node);
		return node;
	}
	return null;
};

Jappsy.GLModelNode.prototype.insertMesh = function(mesh) {
	if (this.nodes === undefined) {
		if (this.meshes === undefined) {
			this.meshes = [];
		}
		this.meshes.push(mesh);
		return mesh;
	}
	return null;
};

Jappsy.GLModelNode.prototype.parseJson = function (gl, node, meshes, rootmat) {
	this.name = node["name"] || "";
	this.transformation.set(node.transformation).transpose();

	var matV = new Mat4().multiply(rootmat, this.transformation);
	
	if (node.children !== undefined) {
		for (var i in node.children) {
			this.insertNode( new Jappsy.GLModelNode().parseJson(gl, node.children[i], meshes, matV) );
		}
	} else if (node.meshes !== undefined) {
		var matN = new Mat4().inverse(matV).transpose();

		for (var j in node.meshes) {
			var idx = node.meshes[j];
			var mesh = meshes[idx];
			
			this.insertMesh( new Jappsy.GLModelMesh().parseJson(gl, mesh, matN, matV) );
		}
	}
	
	return this;
};

Jappsy.GLModelNode.prototype.render = function(context, model) {
	if (this.nodes !== undefined) {
		for (var i in this.nodes) {
			this.nodes[i].render(context, model);
		}
	} else if (this.meshes !== undefined) {
		for (var i in this.meshes) {
			this.meshes[i].render(context, model);
		}
	}
};

//===============================

Jappsy.GLModelMesh = function(name, matterialindex, vertexBuffer, normalsBuffer, textureBuffer, indexBuffer, indexCount) {
	this.name = name || "",
	this.material = matterialindex || 0;
	this.vertices = vertexBuffer || null;
	this.normals = normalsBuffer || null;
	this.texturecoords = textureBuffer || null;
	this.indexes = indexBuffer || null;
	this.indexCount = indexCount || 0;
};

Jappsy.GLModelMesh.prototype.release = function(gl) {
	this.name = null;
	this.material = 0;
	
	gl.bindBuffer(gl.ARRAY_BUFFER, this.vertices);
	gl.bufferData(gl.ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.vertices);
	this.vertices = null;
	
	gl.bindBuffer(gl.ARRAY_BUFFER, this.normals);
	gl.bufferData(gl.ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.normals);
	this.normals = null;

	gl.bindBuffer(gl.ARRAY_BUFFER, this.texturecoords);
	gl.bufferData(gl.ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.texturecoords);
	this.texturecoords = null;
	
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indexes);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.indexes);
	this.indexes = null;
};

Jappsy.GLModelMesh.prototype.parseJson = function(gl, mesh, matN, matV) {
	var faces = mesh.faces;
	
	var vsrc = mesh.vertices;
	var vdst = [];
	for (var i = 0; i < vsrc.length; i += 3) {
		var v = new Vec3().transform(vsrc.slice(i, i+3), matV);
		v.multiply(2.54); // convert inches into mm
		Array.prototype.push.apply(vdst, v);
	}
	
	var vertexBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vdst), gl.STATIC_DRAW);
	
	var vsrc = mesh.normals;
	var vdst = [];
	for (var i = 0; i < vsrc.length; i += 3) {
		var v = new Vec3().transformNormal(vsrc.slice(i, i+3), matN);
		Array.prototype.push.apply(vdst, v);
	}
	
	var normalsBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, normalsBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vdst), gl.STATIC_DRAW);
	
	var textureBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, textureBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(mesh.texturecoords[0]), gl.STATIC_DRAW);
	
	var indexes = [];
	for (var k in faces) {
		indexes.push(faces[k][0], faces[k][1], faces[k][2]);
	}
	var indexBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(indexes), gl.STATIC_DRAW);
	var indexCount = indexes.length;
	delete indexes;
	
	this.name = mesh.name;
	this.material = mesh.materialindex;
	this.vertices = vertexBuffer;
	this.normals = normalsBuffer;
	this.texturecoords = textureBuffer;
	this.indexes = indexBuffer;
	this.indexCount = indexCount;
	
	return this;
};

Jappsy.GLModelMesh.prototype.render = function(context, model) {
	var gl = context.gl;
	var shader = context.shaderModel;

	var mat = model.materials[this.material];
	gl.uniform4fv(shader.uColors, mat.colors4fv);
	
	gl.bindBuffer(gl.ARRAY_BUFFER, this.vertices);
	gl.vertexAttribPointer(shader.aVertexPosition, 3, gl.FLOAT, false, 0, 0);
	
	gl.bindBuffer(gl.ARRAY_BUFFER, this.texturecoords);
	gl.vertexAttribPointer(shader.aTextureCoord, 2, gl.FLOAT, false, 0, 0);

	gl.bindBuffer(gl.ARRAY_BUFFER, this.normals);
	gl.vertexAttribPointer(shader.aVertexNormal, 3, gl.FLOAT, false, 0, 0);

	gl.activeTexture(gl.TEXTURE0);
	if (mat.diffuse.texture.texture1iv != null) {
		gl.bindTexture(gl.TEXTURE_2D, mat.diffuse.texture.texture1iv);
	} else {
		gl.bindTexture(gl.TEXTURE_2D, context.textures.defaultTexture);
	}
	gl.activeTexture(gl.TEXTURE1);
	if (mat.specular.texture.texture1iv != null) {
		gl.bindTexture(gl.TEXTURE_2D, mat.specular.texture.texture1iv);
	} else {
		gl.bindTexture(gl.TEXTURE_2D, context.textures.defaultTexture);
	}
	gl.activeTexture(gl.TEXTURE2);
	if (mat.emissive.texture.texture1iv != null) {
		gl.bindTexture(gl.TEXTURE_2D, mat.emissive.texture.texture1iv);
	} else {
		gl.bindTexture(gl.TEXTURE_2D, context.textures.defaultTexture);
	}
	gl.uniform1iv(shader.uTexture, new Int32Array([0,1,2]));

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indexes);
	gl.drawElements(gl.TRIANGLES, this.indexCount, gl.UNSIGNED_SHORT, 0);
};

//===============================

Jappsy.GLModel = function (context) {
	this.context = context;
	this.materials = [];
	this.rootnode = null;
};

Jappsy.GLModel.prototype.release = function() {
	if (this.context) {
		for (var i = 0; i < this.materials.length; i++) {
			this.materials[i].release();
		}
		this.materials = null;
		
		if (this.rootnode) {
			this.rootnode.release(this.context.gl);
		}
		
		this.context = null;
	}
};

// example: model.createMaterial("mramor.png", 0, 0, 1, 1, Jappsy.UVMAP_PLANAR);
Jappsy.GLModel.prototype.createMaterial = function(key, x, y, w, h, uvmap) {
	var texture = new Jappsy.GLMaterialTexture(key, 1).texture(this.context, key).map(x, y, w, h, uvmap);
	var mat = new Jappsy.GLMaterial(key, texture);
	mat.update();
	this.materials.push(mat);
	return this.materials.length - 1;
};

Jappsy.GLModel.prototype.insertMaterial = function(mat) {
	mat.update();
	this.materials.push(mat);
	return this.materials.length - 1;
};

Jappsy.GLModel.prototype.createCoin = function (radius, height, vertexCount, materials) {
	var gl = this.context.gl;
	
	var mid1 = 0;
	var mid2 = 0;
	var tex1 = null;
	var tex2 = null;
	
	if (Array.isArray(materials)) {
		mid1 = materials[0];
		var mat1 = this.materials[mid1];
		tex1 = mat1.diffuse.texture;
		
		mid2 = materials[1];
		var mat2 = this.materials[mid2];
		tex2 = mat2.diffuse.texture;
	} else {
		mid1 = mid2 = materials;
		var mat = this.materials[materials];
		tex1 = tex2 = mat.diffuse.texture;
	}

	if (this.rootnode == null) {
		this.rootnode = new Jappsy.GLModelNode();
	}
	
	var node = this.rootnode.insertNode(new Jappsy.GLModelNode("cylinder"));
	
	var vcount1 = (vertexCount + 1) * 3 + 1;
	var fcount1 = vertexCount * 3;
	var vcount2 = (vertexCount + 1) + 1;
	var fcount2 = vertexCount;
	
	var vertexes1 = new Jappsy.Float32Stream(vcount1*3);
	var normals1 = new Jappsy.Float32Stream(vcount1*3);
	var texcoords1 = new Jappsy.Float32Stream(vcount1*2);
	var indexes1 = new Jappsy.Uint16Stream(fcount1*3);
	
	var vertexes2 = new Jappsy.Float32Stream(vcount2*3);
	var normals2 = new Jappsy.Float32Stream(vcount2*3);
	var texcoords2 = new Jappsy.Float32Stream(vcount2*2);
	var indexes2 = new Jappsy.Uint16Stream(fcount2*3);
	
	var trw1 = tex1.w / 2.0;
	var trh1 = tex1.h / 2.0;
	var tcx1 = tex1.x + trw1;
	var tcy1 = tex1.y + trh1;

	var trw2 = tex2.w / 2.0;
	var trh2 = tex2.h / 2.0;
	var tcx2 = tex2.x + trw2;
	var tcy2 = tex2.y + trh2;
	
	var h = height / 2.0;

	// Центральная точка верхней грани
	normals1.write(0, 0, 1);
	vertexes1.write(0, 0, h);
	texcoords1.write(tcx1, tcy1);
	
	// Центральная точка нижней грани
	normals2.write(0, 0, -1);
	vertexes2.write(0, 0, -h);
	texcoords2.write(tcx2, tcy2);

	var pid1 = 0;
	for (var i = 0; i <= vertexCount; i++) {
		var cid1 = vertexes1.position / 3;
		var cid2 = vertexes2.position / 3;
	
		var a = i*Math.PI*2/vertexCount;
		var nx = Math.cos(a);
		var ny = Math.sin(a);
		
		// Точка верхней грани
		normals1.write(0, 0, 1);
		vertexes1.write(nx * radius, ny * radius, h);
		texcoords1.write(nx * trw1 + tcx1, -ny * trh1 + tcy1);
		
		// Верхняя точка боковой грани
		normals1.write(nx, ny, 0);
		vertexes1.write(nx * radius, ny * radius, h);
		//if (tex1.uvmap == Jappsy.UVMAP_CYLINDER) {
		//	texcoords1.write(a * tex1.w + tex1.x, tex1.y);
		//} else {
			texcoords1.write(nx * trw1 + tcx1, -ny * trh1 + tcy1);
		//} 

		// Нижняя точка боковой грани
		normals1.write(nx, ny, 0);
		vertexes1.write(nx * radius, ny * radius, -h);
		//if (tex1.uvmap == Jappsy.UVMAP_CYLINDER) {
		//	texcoords1.write(a * tex1.w + tex1.x, tex1.y + tex1.h);
		//} else {
			texcoords1.write(nx * trw1 + tcx1, -ny * trh1 + tcy1);
		//}

		// Точка нижней грани
		normals2.write(0, 0, -1);
		vertexes2.write(nx * radius, ny * radius, -h);
		//if (tex2.uvmap == Jappsy.UVMAP_CYLINDER) {
			texcoords2.write(-nx * trw2 + tcx2, -ny * trh2 + tcy2);
		//} else {
		//	texcoords2.write(nx * trw2 + tcx2, -ny * trh2 + tcy2);
		//}
		
		// Треугольники
		if (i != 0) {
			indexes1.write(0, pid1, cid1);
			indexes1.write(pid1+1, pid1+2, cid1+1);
			indexes1.write(cid1+1, pid1+2, cid1+2);
			
			indexes2.write(0, cid2, pid2);
		}
		
		pid1 = cid1;
		pid2 = cid2;
	}
	
	{
		var vertexBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
		gl.bufferData(gl.ARRAY_BUFFER, vertexes1.data, gl.STATIC_DRAW);

		var normalsBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, normalsBuffer);
		gl.bufferData(gl.ARRAY_BUFFER, normals1.data, gl.STATIC_DRAW);
		
		var textureBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, textureBuffer);
		gl.bufferData(gl.ARRAY_BUFFER, texcoords1.data, gl.STATIC_DRAW);
		
		var indexBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
		gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indexes1.data, gl.STATIC_DRAW);
		var indexCount = indexes1.position;
		
		var mesh = node.insertMesh( new Jappsy.GLModelMesh("part1", mid1, vertexBuffer, normalsBuffer, textureBuffer, indexBuffer, indexCount) );
	}

	{
		var vertexBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
		gl.bufferData(gl.ARRAY_BUFFER, vertexes2.data, gl.STATIC_DRAW);

		var normalsBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, normalsBuffer);
		gl.bufferData(gl.ARRAY_BUFFER, normals2.data, gl.STATIC_DRAW);
		
		var textureBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, textureBuffer);
		gl.bufferData(gl.ARRAY_BUFFER, texcoords2.data, gl.STATIC_DRAW);
		
		var indexBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
		gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indexes2.data, gl.STATIC_DRAW);
		var indexCount = indexes2.position;
		
		var mesh = node.insertMesh( new Jappsy.GLModelMesh("part2", mid2, vertexBuffer, normalsBuffer, textureBuffer, indexBuffer, indexCount) );
	}
};

// Create Model from JSON 3D Model
Jappsy.GLModel.prototype.parseJson = function (json) {
	var gl = this.context.gl;
	
	if ((json.rootnode === undefined) || (json.meshes === undefined))
		return false;

	var rootmat = new Mat4().identity();
	this.rootnode = new Jappsy.GLModelNode().parseJson(gl, json.rootnode, json.meshes, rootmat);
	
	var materials = this.materials;
	for (var i in json.materials) {
		var material = json.materials[i].properties;
		
		var mat = new Jappsy.GLMaterial();
		
		for (var j in material) {
			var key = material[j].key;
			var value = material[j].value;
			var semantic = material[j].semantic;
			
			if (semantic == 0) {
				if (/name$/i.test(key)) {
					mat.name = value;
				} else if (/ambient$/i.test(key)) {
					mat.ambient.color = value;
				} else if (/diffuse$/i.test(key)) {
					mat.diffuse.color = value;
				} else if (/specular$/i.test(key)) {
					mat.specular.color = value;
				} else if (/emissive$/i.test(key)) {
					mat.emissive.color = value;
				} else if (/shininess$/i.test(key)) {
					mat.shininess.value = value;
				} else if (/shinpercent$/i.test(key)) {
					mat.shininess.percent = value;
				} else if (/opacity$/i.test(key)) {
					mat.opacity.value = value;
				} else if (/bumpscaling$/i.test(key)) {
					mat.bump.scale = value;
				}
			} else {
				var obj = null;
				switch (semantic) {
					case 1: obj = mat.diffuse; break;
					case 2: obj = mat.specular; break;
					case 4: obj = mat.emissive; break;
					case 5: obj = mat.bump; break;
					case 7: obj = mat.shininess; break;
					case 8: obj = mat.opacity; break;
					case 11: obj = mat.reflection; break;
				}
				if (obj !== null) {
					if (/file$/i.test(key)) {
						obj.texture = obj.texture || new Jappsy.GLMaterialTexture();
						obj.texture.file = value;
						obj.texture.texture( this.context, value.toLowerCase() );
					}
					if (/blend$/i.test(key)) {
						obj.texture = obj.texture || new Jappsy.GLMaterialTexture();
						obj.texture.blend = parseFloat(value);
					}
					if (/mapmodeu$/i.test(key)) {
						// Default: 0
					}
					if (/mapmodev$/i.test(key)) {
						// Default: 0
					}
					if (/uvtrafo$/i.test(key)) {
						// Default: [0,0,1,1,0]
					}
				}
			}
		}
		
		this.insertMaterial(mat);
	}
	
	return true;
};

Jappsy.GLModel.prototype.render = function(object) {
	var scene = object.scene;
	var context = scene.context;
	var gl = context.gl;
	var shader = context.shaderModel;
	
	gl.enable(gl.DEPTH_TEST);
	
	var index = shader.bind(0);

	gl.uniform3fv(shader.uAmbientLightColor, scene.ambient);
	gl.uniform1i(shader.uLightsCount, scene.lights1i);
	gl.uniformMatrix4fv(shader.uLights, false, scene.lights16fvv);

	gl.enableVertexAttribArray(shader.aVertexPosition);
	gl.enableVertexAttribArray(shader.aTextureCoord);
	gl.enableVertexAttribArray(shader.aVertexNormal);

	//gl.enable(gl.TEXTURE_2D);
	gl.enable(gl.BLEND);

	scene.modelView16fv.multiply(scene.camera.view16fv, object.objectMatrix).multiply(object.modelMatrix);
	scene.modelViewProjection16fv.multiply(scene.camera.projection16fv, scene.modelView16fv);
	scene.normal16fv.inverse(scene.modelView16fv).transpose();
	gl.uniformMatrix4fv(shader.uModelViewProjectionMatrix, false, scene.modelViewProjection16fv);
	gl.uniformMatrix4fv(shader.uModelViewMatrix, false, scene.modelView16fv);
	gl.uniformMatrix4fv(shader.uNormalMatrix, false, scene.normal16fv);
	
	this.rootnode.render(context, this);

	gl.disableVertexAttribArray(shader.aVertexPosition);
	gl.disableVertexAttribArray(shader.aTextureCoord);
	gl.disableVertexAttribArray(shader.aVertexNormal);

	context.cleanup(3);

	gl.disable(gl.BLEND);
	//gl.disable(gl.TEXTURE_2D);
	gl.disable(gl.DEPTH_TEST);
};

///////////////////////////

Jappsy.GLModels = function(context) {
	this.context = context;
	this.list = {};
};

Jappsy.GLModels.prototype.release = function() {
	this.context = null;
	for (var key in this.list) {
		this.list[key].release();
	}
	this.list = null;
};

Jappsy.GLModels.prototype.get = function(key) {
	return this.list[key] || null;
};

Jappsy.GLModels.prototype.createModel = function(key, jsonModel) {
	if (this.list[key] !== undefined)
		this.list[key].release();
	
	this.list[key] = new Jappsy.GLModel(this.context);
	if (jsonModel !== undefined) {
		this.list[key].parseJson(jsonModel);
	}
	return this.list[key];
};
