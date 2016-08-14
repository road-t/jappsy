// Requires GLScene

Jappsy.GLLight = function(scene, key) {
	this.scene = scene;
	this.key = key;
	this.position = new Vec3([0,0,-1]);
	this.target = new Vec3([0,0,0]);
	this.color = new Vec3([1,1,1]);
	this.intensivity = 1.0;
	this.hotspot = 0;
	this.falloff = 0;
	this.fixed = false;
	this.style = "direct";
	this.active = true;		// Параметр на будущее, чтобы можно было отрубать свет
	this.invalid = true;
	
	this.position3fv = new Vec3();
	this.target3fv = new Vec3();
	this.light16fv = new Mat4();
	
	//this.update();
};

Jappsy.GLLight.prototype.release = function() {
	this.scene = null;
	this.key = null;
	this.position = null;
	this.target = null;
	this.color = null;
	this.position3fv = null;
	this.target3fv = null;
	this.light16fv = null;
};

Jappsy.GLLight.prototype.invalidate = function() {
	this.invalid = true;
	return this;
};

Jappsy.GLLight.prototype.omni = function(config) {
	this.position.set(config.position);
	this.target.set([0,0,0]);
	this.color.set(config.color);
	this.intensivity = config.intensivity || 1.0;
	this.hotspot = config.radius || 0.0;
	this.falloff = config.falloff || 0.0;
	this.fixed = config.fixed || false;
	this.style = "omni";
	this.invalid = true;
	
	//this.update();
	return this;
};

Jappsy.GLLight.prototype.spot = function(config) {
	this.position.set(config.position);
	this.target.set(config.target);
	this.color.set(config.color);
	this.intensivity = config.intensivity || 1.0;
	this.hotspot = config.angle || 0.0;
	this.falloff = config.falloff || 0.0;
	this.fixed = config.fixed || false;
	this.style = "spot";
	this.invalid = true;
	
	//this.update();
	return this;
};

Jappsy.GLLight.prototype.direct = function(config) {
	this.position.set(config.position);
	this.target.set(config.target);
	this.color.set(config.color);
	this.intensivity = config.intensivity || 1.0;
	this.hotspot = config.radius || 0.0;
	this.falloff = config.falloff || 0.0;
	this.fixed = config.fixed || false;
	this.style = "direct";
	this.invalid = true;
	
	//this.update();
	return this;
};

Jappsy.GLLight.prototype.update = function() {
	if (this.invalid) {
		if (this.fixed) {
			this.position3fv.set(this.position);
			this.target3fv.set(this.target);
		} else {
			this.position3fv.transform(this.position, this.scene.camera.view16fv);
			this.target3fv.transform(this.target, this.scene.camera.view16fv);
		}
		
		this.light16fv[0] = this.position3fv[0];
		this.light16fv[1] = this.position3fv[1];
		this.light16fv[2] = this.position3fv[2];
		this.light16fv[8] = this.color[0] * this.intensivity;
		this.light16fv[9] = this.color[1] * this.intensivity;
		this.light16fv[10] = this.color[2] * this.intensivity;
		
		if (this.style == "omni") {
			this.light16fv[3] = this.hotspot;
			this.light16fv[4] = this.light16fv[5] = this.light16fv[6] = 0;
			this.light16fv[7] = this.falloff;
			this.light16fv[11] = 0;
		} else {
			this.light16fv[4] = this.target3fv[0];
			this.light16fv[5] = this.target3fv[1];
			this.light16fv[6] = this.target3fv[2];
			
			if (this.style == "spot") {
				this.light16fv[3] = this.hotspot * Math.PI / 360.0;
				this.light16fv[7] = this.falloff * Math.PI / 360.0;
				this.light16fv[11] = 1;
			} else {
				this.light16fv[3] = this.hotspot;
				this.light16fv[7] = this.falloff;
				this.light16fv[11] = 2;
			}
		}
		this.light16fv[12] = this.light16fv[13] = this.light16fv[14] = this.light16fv[15] = 0;
		
		this.invalid = false;
		return true;
	}
	
	return false;
};

///////////////////////////

Jappsy.GLLights = function(scene) {
	this.scene = scene;
	this.list = {};
};

Jappsy.GLLights.prototype.release = function() {
	this.scene = null;
	
	for (var key in this.list) {
		this.list[key].release();
	}
	this.list = null;
};

Jappsy.GLLights.prototype.get = function(key) {
	return this.list[key] || null;
};

Jappsy.GLLights.prototype.createLight = function(key) {
	if (this.list[key] === undefined)
		this.list[key] = new Jappsy.GLLight(this.scene, key);

	return this.list[key];
};
