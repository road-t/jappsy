// Requires GLContext, GLCache

Jappsy.GLScene = function(context) {
	this.context = context;
	this.camera = null;
	this.ambient = new Vec3([0.25,0.25,0.25]);			// Ambient Color
	this.lights = new Jappsy.GLLights(this);
	this.objects = new Jappsy.GLObjects(this);
	this.layers = [];
	
	this.lights1i = 0;
	this.lights16fvv = new Float32Array(this.context.lightsMaxCount * 16);
	this.modelViewProjection16fv = new Mat4().identity();
	this.modelView16fv = new Mat4().identity();
	this.normal16fv = new Mat4().identity();
};

Jappsy.GLScene.prototype.release = function() {
	this.context = null;
	this.camera = null;
	this.ambient = null;
	
	this.lights.release();	
	this.objects.release();

	this.lights = null;
	this.objects = null;

	this.layers = null;
	this.lights16fvv = null;
	this.modelViewProjection16fv = null;
	this.modelView16fv = null;
	this.normal16fv = null;
};

Jappsy.GLScene.prototype.setCamera = function(camera) {
	this.camera = camera;
};

Jappsy.GLScene.prototype.rotateGroup = function(group, vec, angle, permanent) {
	if (permanent && permanent !== false) {
		var rotate = new Mat4().rotate(vec, angle);
		for (var i = 0; i < group.length; i++) {
			var obj = this.objects.get(group[i]);
			if (obj != null) {
				obj.objectMatrix.multiply(rotate);
			}
		}
	} else {
		for (var i = 0; i < group.length; i++) {
			var obj = this.objects.get(group[i]);
			if (obj != null) {
				obj.modelMatrix.rotate(vec, angle);
			}
		}
	}
};

Jappsy.GLScene.prototype.visibleGroup = function(group, visible) {
	for (var i = 0; i < group.length; i++) {
		var obj = this.objects.get(group[i]);
		if (obj != null) {
			obj.visible = visible;
		}
	}
};

Jappsy.GLScene.prototype.startParticlesGroup = function(group, repeat) {
	var currentTime = (new Date).getTime() * 60 / 1000;
	
	for (var i = 0; i < group.length; i++) {
		var obj = this.objects.get(group[i]);
		if (obj != null) {
			if (obj.object.startTime == 0) {
				obj.object.startTime = currentTime + Math.random()*obj.object.startTimeRange;
			}
			obj.object.repeat = repeat;
			obj.object.color.random(obj.object.colorRange);
		}
	}
};

Jappsy.GLScene.prototype.createLayer = function() {
	this.layers.push([]);
	return this.layers.length-1;
};

Jappsy.GLScene.prototype.createLayerObject = function(index, key) {
	var object = this.objects.createObject(key);
	this.layers[index].push(object);
	return object;
};

Jappsy.GLScene.prototype.createLayerDrawing = function(index, key, time) {
	if (time === undefined)
		time = null;
	if (Array.isArray(key)) {
		for (var i = 0; i < key.length; i++) {
			var object = this.objects.createObject(key[i]);
			this.layers[index].push(object);
			object.setDrawing(key[i], time);
		}
		return null;
	} else {
		var object = this.objects.createObject(key);
		this.layers[index].push(object);
		object.setDrawing(key, time);
		return object;
	}
};

Jappsy.GLScene.prototype.update = function() {
	var invalidateLights = this.camera.update();
	
	this.lights1i = 0;
	for (var key in this.lights.list) {
		var light = this.lights.list[key];
		if (light.active) {
			if (invalidateLights) light.invalidate();
			light.update();
			for (var i = 0; i < 16; i++)
				this.lights16fvv[this.lights1i*16 + i] = light.light16fv[i];
			this.lights1i++;
			if (this.lights1i >= this.lightsMaxCount) break;
		}
	}
};

Jappsy.GLScene.prototype.render = function() {
	this.update();

	var gl = this.context.gl;
	gl.enable(gl.DEPTH_TEST);
	gl.clear(gl.DEPTH_BUFFER_BIT);
	gl.disable(gl.DEPTH_TEST);
	
	for (var layerIndex = 0; layerIndex < this.layers.length; layerIndex++) {
		var layer = this.layers[layerIndex];
		for (var objectIndex = 0; objectIndex < layer.length; objectIndex++) {
			var object = layer[objectIndex];
			object.render();
		}
	}

	/*
	for (var key in this.objects.list) {
		var object = this.objects.list[key];
		object.render();
	}
	*/
	
	//this.particles.render();
};

///////////////////////////

Jappsy.GLScenes = function(context) {
	this.context = context;
	this.list = {};
};

Jappsy.GLScenes.prototype.release = function() {
	this.context = null;
	
	for (var key in this.list) {
		this.list[key].release();
	}
	this.list = null;
};

Jappsy.GLScenes.prototype.get = function(key) {
	return this.list[key] || null;
};

Jappsy.GLScenes.prototype.createScene = function(key) {
	if (this.list[key] !== undefined)
		this.list[key].release();
	
	this.list[key] = new Jappsy.GLScene(this.context);
	return this.list[key];
};
