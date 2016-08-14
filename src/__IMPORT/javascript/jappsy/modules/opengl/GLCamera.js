// Requires

Jappsy.GLCamera = function(context, key) {
	this.context = context;
	this.key = key;
	this.position = new Vec3([0,0,-1]);
	this.target = new Vec3([0,0,0]);
	this.head = new Vec3([0,1,0]);
	this.fov = 45;
	this.width = 1920;
	this.height = 1080;
	this.min = 0.1;
	this.max = 1000;
	this.style = "perspective";
	this.invalid = true;
	this.scale = 0.01;

	this.projection16fv = new Mat4();
	this.view16fv = new Mat4();

	//this.update();
};

Jappsy.GLCamera.prototype.release = function() {
	this.context = null;
	this.key = null;
	this.position = null;
	this.target = null;
	this.head = null;
	this.projection16fv = null;
	this.view16fv = null;
};

Jappsy.GLCamera.prototype.invalidate = function() {
	this.invalid = true;
	return this;
};

Jappsy.GLCamera.prototype.size = function(width, height) {
	this.width = width;
	this.height = height;
	this.invalid = true;
	
	//this.update();
	return this;
};

Jappsy.GLCamera.prototype.perspective = function(fov, min, max) {
	this.fov = fov;
	this.min = min;
	this.max = max;
	this.style = "perspective";
	this.invalid = true;
	
	//this.update();
	return this;
};

Jappsy.GLCamera.prototype.ortho = function(min, max) {
	this.min = min;
	this.max = max;
	this.style = "orthographic";
	this.invalid = true;
	
	//this.update();
	return this;
};

Jappsy.GLCamera.prototype.layer = function(offsetX, offsetY) {
	this.target[0] = offsetX;
	this.target[1] = offsetY;
	this.style = "layer";
	this.invalid = true;
	
	//this.update();
	return this;
};

Jappsy.GLCamera.prototype.lookAt = function(position, target, head) {
	this.position.set(position);
	this.target.set(target);
	this.head.set(head);
	this.invalid = true;
	
	//this.update();
	return this;
};

// Rotate Camera around Target
Jappsy.GLCamera.prototype.rotate = function(vec, angle) {
	var rotate = new Mat4().rotate(new Vec3(vec).normalize(), angle);
	this.position.subtract(this.target).transform(rotate).add(this.target);
	this.invalid = true;
	
	//this.update();
	return this;
};

Jappsy.GLCamera.prototype.update = function() {
	if (this.invalid) {
		if (this.style == "layer") {
			for (var i = 0; i < 16; i++)
				this.projection16fv[i] = 0;

			this.projection16fv[0] = 2 / this.width;
			this.projection16fv[5] = -2 / this.height;
			this.projection16fv[12] = -1 + this.target[0] * this.projection16fv[0];
			this.projection16fv[13] = 1 + this.target[1] * this.projection16fv[5];
			this.projection16fv[10] = this.projection16fv[15] = 1;

			/*
			{  2/w,     0,     0,    0}		rX = sX * 2 / w - 1 <=> (0..w) -> (-1..+1)
			{    0,  -2/h,     0,    0}		rY = sY * -2 / h + 1 <=> (0..w) -> (+1..-1)	// flip vertical
			{    0,     0,     1,    0}
			{   -1,     1,     0,    1}
			*/
			
			this.view16fv.identity();
		} else {
			if (this.style == "orthographic") {
				var w2 = this.width / 2;
				var h2 = this.height / 2;
				this.projection16fv.ortho(-w2, w2, -h2, h2, this.min, this.max);
			} else if (this.style == "perspective") {
				this.projection16fv.perspective(this.fov, this.width/this.height, this.min, this.max)
					.multiply(new Mat4().scale([this.scale, this.scale, this.scale]));
			}

			this.view16fv.lookAt(this.position, this.target, this.head);
		}
		
		this.invalid = false;
		return true;
	}
	
	return false;
};

///////////////////////////

Jappsy.GLCameras = function(context) {
	this.context = context;
	this.list = {};
	this.gui = null;
};

Jappsy.GLCameras.prototype.release = function() {
	this.context = null;
	
	for (var key in this.list) {
		this.list[key].release();
	}
	this.list = null;
	this.gui = null;
};

Jappsy.GLCameras.prototype.get = function(key) {
	return this.list[key] || null;
};

Jappsy.GLCameras.prototype.createCamera = function(key) {
	if (this.list[key] === undefined) {
		this.list[key] = new Jappsy.GLCamera(this.context, key);
		
		if (key == "gui") {
			this.gui = this.list[key];
		}
	}

	return this.list[key];
};

