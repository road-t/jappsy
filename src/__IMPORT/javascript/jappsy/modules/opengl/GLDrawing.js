Jappsy.GLDrawing = function(context, key, info) {
	if (Jappsy.GLDrawing.paint === undefined) {
		Jappsy.GLDrawing.paint = new Jappsy.JPaint().setColor(0xFFFFFFFF);
	}
	
	this.context = context;
	this.key = key;
	this.sprite = context.sprites.get(info[0]);
	if (info[3]) {
		this.paint = new Jappsy.JPaint(info[3]);
	} else {
		this.paint = new Jappsy.JPaint()
			.setColor(0xFFFFFFFF)
			.setAlignX(Jappsy.JAlignX.LEFT)
			.setAlignY(Jappsy.JAlignY.TOP);
	}
	this.position = info[1];
	if ((info[2]) && (Array.isArray(info[2]))) {
		this.background = -1;
		this.normal = info[2][0];
		this.hover = info[2][1];
		this.click = info[2][2];
		this.foreground = info[2][3] || -1;
	} else {
		this.background = info[2] || 0;
		this.normal = this.hover = this.click = this.foreground = -1;
	}
	this.hovered = false;
	this.pressed = false;
	
	this.onrender = null;
	this.animation = null;
};

Jappsy.GLDrawing.prototype.release = function() {
	this.context = null;
	this.key = null;
	this.sprite = null;
	this.paint = null;
	this.position = null;
	this.normal = this.hover = this.click = 0;
	this.animation = null;
};

Jappsy.GLDrawing.prototype.render = function(object, time) {
	if (this.animation)
		this.animation.update();

	if (this.background >= 0)
		this.sprite.render(this.position, this.background, this.paint, time);
	if (this.pressed) {
		if (this.click >= 0)
			this.sprite.render(this.position, this.click, this.paint, time);
		else if (this.normal >= 0)
			this.sprite.render(this.position, this.normal, this.paint, time);
	} else if (this.hovered) {
		if (this.hover >= 0)
			this.sprite.render(this.position, this.hover, this.paint, time);
		else if (this.normal >= 0)
			this.sprite.render(this.position, this.normal, this.paint, time);
	} else if (this.normal >= 0) {
		this.sprite.render(this.position, this.normal, this.paint, time);
	}

	if (this.onrender)
		this.onrender(this);

	if (this.foreground >= 0)
		this.sprite.render(this.position, this.foreground, this.paint, time);
};

///////////////////////////

Jappsy.GLDrawings = function(context) {
	this.context = context;
	this.list = {};
};

Jappsy.GLDrawings.prototype.release = function() {
	this.context = null;
	
	for (var key in this.list) {
		this.list[key].release();
	}
	this.list = null;
};

Jappsy.GLDrawings.prototype.get = function(key) {
	return this.list[key] || null;
};

Jappsy.GLDrawings.prototype.createDrawing = function(key, info) {
	if (this.list[key] === undefined)
		this.list[key] = new Jappsy.GLDrawing(this.context, key, info);

	return this.list[key];
};

Jappsy.GLDrawings.prototype.renderDrawing = function(key, time) {
	this.list[key].render(null, time);
};

///////////////////////////

Jappsy.Animation = function(target, ofs, min, max, speed, update) {
	this.target = target;
	this.time = new Date().getTime() + ofs * speed;
	this.min = min;
	this.max = max;
	this.speed = speed;
	this.update = update.bind(this);
};

Jappsy.Animation.prototype.release = function() {
	if (this.target) {
		this.target.animation = null;
		this.target = null;
	}
	this.time = null;
	this.update = null;
};

Jappsy.Animation.updatePingPong = function() {
	var time = new Date().getTime();
	if (time < this.time) {
		var ofs = (this.time - time) / this.speed;
		this.target.paint.m_color4iv[0] = this.target.paint.m_color4iv[1] = this.target.paint.m_color4iv[2] = this.target.paint.m_color4iv[3]
			= this.min - (this.max - this.min) * ofs;
	} else {
		var ofs = (this.speed - Math.abs(Jappsy.mod(time - this.time, this.speed * 2) - this.speed)) / this.speed;
		this.target.paint.m_color4iv[3]
			= 1.0;
		this.target.paint.m_color4iv[0] = this.target.paint.m_color4iv[1] = this.target.paint.m_color4iv[2]
			= this.min + (this.max - this.min) * ofs;
	}
};

Jappsy.Animation.updateLightOut = function() {
	var time = new Date().getTime();
	var elapsed = time - this.time;
	if (elapsed < this.speed) {
		var ofs = elapsed / this.speed;
		this.target.paint.m_color4iv[3]
			= 1.0;
		this.target.paint.m_color4iv[0] = this.target.paint.m_color4iv[1] = this.target.paint.m_color4iv[2]
			= this.max - (this.max - this.min) * ofs;
	} else {
		this.target.paint.m_color4iv[3]
			= 1.0;
		this.target.paint.m_color4iv[0] = this.target.paint.m_color4iv[1] = this.target.paint.m_color4iv[2]
			= this.min;
		this.target.animation.release();
	}
};

Jappsy.Animation.updateLightIn = function() {
	var time = new Date().getTime();
	var elapsed = time - this.time;
	if (elapsed < this.speed) {
		var ofs = elapsed / this.speed;
		this.target.paint.m_color4iv[3]
			= 1.0;
		this.target.paint.m_color4iv[0] = this.target.paint.m_color4iv[1] = this.target.paint.m_color4iv[2]
			= this.min + (this.max - this.min) * ofs;
	} else {
		this.target.paint.m_color4iv[3]
			= 1.0;
		this.target.paint.m_color4iv[0] = this.target.paint.m_color4iv[1] = this.target.paint.m_color4iv[2]
			= this.max;
		this.target.animation.release();
	}
};

Jappsy.Animation.updateFadeOut = function() {
	var time = new Date().getTime();
	var elapsed = time - this.time;
	if (elapsed < this.speed) {
		var ofs = elapsed / this.speed;
		this.target.paint.m_color4iv[0] = this.target.paint.m_color4iv[1] = this.target.paint.m_color4iv[2] = this.target.paint.m_color4iv[3]
			= this.max - (this.max - this.min) * ofs;
	} else {
		this.target.paint.m_color4iv[0] = this.target.paint.m_color4iv[1] = this.target.paint.m_color4iv[2] = this.target.paint.m_color4iv[3]
			= this.min;
		this.target.animation.release();
	}
};

Jappsy.Animation.updateFadeIn = function() {
	var time = new Date().getTime();
	var elapsed = time - this.time;
	if (elapsed < this.speed) {
		var ofs = elapsed / this.speed;
		this.target.paint.m_color4iv[0] = this.target.paint.m_color4iv[1] = this.target.paint.m_color4iv[2] = this.target.paint.m_color4iv[3]
			= this.min + (this.max - this.min) * ofs;
	} else {
		this.target.paint.m_color4iv[0] = this.target.paint.m_color4iv[1] = this.target.paint.m_color4iv[2] = this.target.paint.m_color4iv[3]
			= this.max;
		this.target.animation.release();
	}
};

Jappsy.Animation.createPingPong = function(target, min, max, speed) {
	var ofs = (target.paint.m_color4iv[0] - min) / (max - min);
	if (target.animation)
		target.animation.release();
	target.animation = new Jappsy.Animation(target, -ofs, min, max, speed, this.updatePingPong);
};

Jappsy.Animation.createBlink = function(target, min, max, speed) {
	if (target.animation)
		target.animation.release();
	target.animation = new Jappsy.Animation(target, 0, min, max, speed, this.updateLightOut);
};

Jappsy.Animation.createLightOut = function(target, min, max, speed) {
	var ofs = max - (target.paint.m_color4iv[0] - min) / (max - min);
	if (target.animation)
		target.animation.release();
	target.animation = new Jappsy.Animation(target, -ofs, min, max, speed, this.updateLightOut);
};

Jappsy.Animation.createLightIn = function(target, min, max, speed) {
	var ofs = (target.paint.m_color4iv[0] - min) / (max - min);
	if (target.animation)
		target.animation.release();
	target.animation = new Jappsy.Animation(target, -ofs, min, max, speed, this.updateLightIn);
};

Jappsy.Animation.createFadeOut = function(target, min, max, speed) {
	var ofs = max - (target.paint.m_color4iv[0] - min) / (max - min);
	if (target.animation)
		target.animation.release();
	target.animation = new Jappsy.Animation(target, -ofs, min, max, speed, this.updateFadeOut);
};

Jappsy.Animation.createFadeIn = function(target, min, max, speed) {
	var ofs = (target.paint.m_color4iv[0] - min) / (max - min);
	if (target.animation)
		target.animation.release();
	target.animation = new Jappsy.Animation(target, -ofs, min, max, speed, this.updateFadeIn);
};
