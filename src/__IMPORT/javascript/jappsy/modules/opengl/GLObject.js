// Requires GLScene

Jappsy.GLObject = function(scene, key) {
	this.scene = scene;
	this.key = key;
	this.object = null;
	this.visible = true;
	this.onEvent = null;
	this.onEventHandler = null;
	this.time = null;
	
	this.modelMatrix = new Mat4().identity();
	this.objectMatrix = new Mat4().identity();
};

Jappsy.GLObject.prototype.release = function() {
	this.scene = null;
	this.key = null;
	this.object = null;
	this.onEvent = null;
	this.onEventHandler = null;
	this.time = null;
	this.modelMatrix = null;
	this.objectMatrix = null;
};

Jappsy.GLObject.prototype.setModel = function(key) {
	this.object = this.scene.context.models.get(key);
	return this;
};

Jappsy.GLObject.prototype.setFunc = function(callback) {
	this.object = new Jappsy.GLFunc(callback);
	return this;
};

Jappsy.GLObject.prototype.setParticleSystem = function(key) {
	this.object = this.scene.context.particles.get(key);
	return this;
};

Jappsy.GLObject.prototype.setDrawing = function(key, time) {
	this.object = this.scene.context.drawings.get(key);
	this.time = time;
	return this;
};

Jappsy.GLObject.prototype.eventHandler = function(event) {
	if (this.visible) {
		return this.onEvent(event, this.object);
	}
	return false;
};

Jappsy.GLObject.prototype.trackEvent = function(onevent) {
	if (this.object instanceof Jappsy.GLDrawing) {
		var pos = this.object.sprite.getPosition(this.object.position, this.object.paint);
		var x = pos[0];
		var y = pos[1];
		var w = this.object.sprite.width;
		var h = this.object.sprite.height;
		this.onEvent = onevent;
		this.onEventHandler = this.eventHandler.bind(this);
		this.scene.context.touchScreen.clickEvent(this.key, x, y, w, h, this.onEventHandler);
		this.scene.context.touchScreen.trackEvent(this.key, x, y, w, h, this.onEventHandler);
	}
};

Jappsy.GLObject.prototype.render = function() {
	if (this.visible && this.object) {
		if (this.time !== null)
			this.object.render(this, this.time);
		else
			this.object.render(this);
	}
};

///////////////////////////

Jappsy.GLObjects = function(scene) {
	this.scene = scene;
	this.list = {};
};

Jappsy.GLObjects.prototype.release = function() {
	this.scene = null;
	
	for (var key in this.list) {
		this.list[key].release();
	}
	this.list = null;
};

Jappsy.GLObjects.prototype.get = function(key) {
	return this.list[key] || null;
};

Jappsy.GLObjects.prototype.createObject = function(key) {
	if (this.list[key] === undefined)
		this.list[key] = new Jappsy.GLObject(this.scene, key);

	return this.list[key];
};

Jappsy.GLObjects.prototype.trackEvents = function(group, onevent) {
	for (var i = 0; i < group.length; i++) {
		this.list[group[i]].trackEvent(onevent);
	}
};

