Jappsy.GLParticle = function () {
	this.position = new Vec3([0,0,0]);
	this.velocity = new Vec3([0,0,0]);
	this.acceleration = new Vec3([0,0,0]);
	this.size = 1;
	this.startTime = 0;
	this.lifeTime = 0;
	this.type = "";
};

Jappsy.GLParticle.prototype.release = function() {
	this.position = null;
	this.velocity = null;
	this.acceleration = null;
	this.type = null;
};

///////////////////////////

Jappsy.GLParticleSystem = function(context, key) {
	this.context = context;
	this.key = key;
	this.list = [];
	this.startTime = 0;
	this.maxTime = 0;
	this.repeat = -1;
	this.startTimeRange = 0.5;
	this.color = new Vec3([1,1,1]);
	this.colorRange = [-0.5,-0.5,-0.5];
	this.color.random(this.colorRange);
	
	this.vertexBuffer = null;
	this.textureBuffer = null;
	this.velocityBuffer = null;
	this.accelBuffer = null;
	this.timeBuffer = null;
	this.indexBuffer = null;
	this.indexCount = 0;
};

Jappsy.GLParticleSystem.prototype.release = function() {
	var gl = this.context.gl;
	
	this.context = null;
	this.key = null;

	for (var i = 0; i < this.list.length; i++)
		this.list[i].release();
	this.list = [];

	this.color = null;
	this.colorRange = null;

	gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.vertexBuffer);
	this.vertexBuffer = null;

	gl.bindBuffer(gl.ARRAY_BUFFER, this.textureBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.textureBuffer);
	this.textureBuffer = null;

	gl.bindBuffer(gl.ARRAY_BUFFER, this.velocityBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.velocityBuffer);
	this.velocityBuffer = null;

	gl.bindBuffer(gl.ARRAY_BUFFER, this.accelBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.accelBuffer);
	this.accelBuffer = null;

	gl.bindBuffer(gl.ARRAY_BUFFER, this.timeBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.timeBuffer);
	this.timeBuffer = null;

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, 1, gl.STATIC_DRAW);
	gl.deleteBuffer(this.indexBuffer);
	this.indexBuffer = null;
};

Jappsy.GLParticleSystem.prototype.createRocket = function(startTime, position) {
	var p = new Jappsy.GLParticle();
	p.position.set(position);
	p.velocity.random([0.5, 0, 0.5]).add([0,0.5,0]).multiply(Math.random()*2 + 4);
	p.acceleration.set([0, -0.01, 0]);
	p.size = 2;
	p.startTime = startTime || 0;
	p.lifeTime = Math.floor(Math.random()*50) + 50;
	p.type = "rocket";
	this.list.push(p);
	
	var time = p.startTime + p.lifeTime;
	
	for (var i = 0; i < p.lifeTime; i++) {
		if (Math.random() > 0.25) {
			var vV = new Vec3().multiply(p.velocity, i);
			var vA = new Vec3().multiply(p.acceleration, i * (i + 1) / 2);
			var vP = new Vec3().add(p.position, vV).add(vA);
			vV.multiply(p.acceleration, i).add(p.velocity);
		
			var subtime = this.createSubFlare(p.startTime + i, vP, vV);
			if (subtime > time) time = subtime;
		}
	}

	var vV = new Vec3().multiply(p.velocity, p.lifeTime);
	var vA = new Vec3().multiply(p.acceleration, p.lifeTime * (p.lifeTime + 1) / 2);
	var vP = new Vec3().add(p.position, vV).add(vA);
	var c = Math.floor(Math.random()*50)+50;
	for (var i = 0; i < c; i++) {
		var subtime = this.createFlare(p.startTime + p.lifeTime, vP);
		if (subtime > time) time = subtime;
	}
	
	if (this.maxTime < time) this.maxTime = time;
	
	return p;
};

Jappsy.GLParticleSystem.prototype.createFlare = function(startTime, position) {
	var p = new Jappsy.GLParticle();
	p.position.set(position);
	p.velocity.random([0.5, 0.5, 0.5]).multiply(Math.random()*2 + 1);
	p.acceleration.set([0, -0.01, 0]);
	p.size = 5;
	p.startTime = startTime || 0;
	p.lifeTime = Math.floor(Math.random()*100) + 10;
	p.type = "flare";
	this.list.push(p);

	var time = p.startTime + p.lifeTime;

	for (var i = 0; i < p.lifeTime; i++) {
		if (Math.random() > 0.75) {
			var vV = new Vec3().multiply(p.velocity, i);
			var vA = new Vec3().multiply(p.acceleration, i * (i + 1) / 2);
			var vP = new Vec3().add(p.position, vV).add(vA);
			vV.multiply(p.acceleration, i).add(p.velocity);
		
			var subtime = this.createSubFlare(p.startTime + i, vP, vV);
			if (subtime > time) time = subtime;
		}
	}

	if (this.maxTime < time) this.maxTime = time;

	return p;
};

Jappsy.GLParticleSystem.prototype.createSubFlare = function(startTime, position, velocity) {
	var p = new Jappsy.GLParticle();
	p.position.set(position);
	p.velocity.random([0.5, 0.5, 0.5]).multiply(Math.random() + 1).add(velocity).multiply(0.25);
	p.acceleration.set([0, -0.01, 0]);
	p.size = 2;
	p.startTime = startTime || 0;
	p.lifeTime = Math.floor(Math.random()*50) + 10;
	p.type = "subflare";
	this.list.push(p);
	
	var time = p.startTime + p.lifeTime;
	
	if (this.maxTime < time) this.maxTime = time;

	return p;
};

Jappsy.GLParticleSystem.prototype.generate = function() {
	var gl = this.context.gl;
	
	var vertex = [];
	var texture = [];
	var velocity = [];
	var accel = [];
	var time = [];
	var index = [];
	
	var __UNUSED__ = 0;
	
	for (var i = 0; i < this.list.length; i++) {
		var p = this.list[i];
		for (var j = 0; j < 4; j++) {
			vertex.push(p.position[0], p.position[1], p.position[2], j);
			velocity.push(p.velocity[0], p.velocity[1], p.velocity[2], p.size);
			accel.push(p.acceleration[0], p.acceleration[1], p.acceleration[2], __UNUSED__);
			time.push(p.startTime, p.lifeTime, __UNUSED__, __UNUSED__);
		}
		texture.push(0,0,0,1,1,1,1,0);
		var k = i*4;
		index.push(k,k+1,k+2,k,k+2,k+3);
	}

	this.vertexBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertex), gl.STATIC_DRAW);

	this.textureBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, this.textureBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(texture), gl.STATIC_DRAW);

	this.velocityBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, this.velocityBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(velocity), gl.STATIC_DRAW);

	this.accelBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, this.accelBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(accel), gl.STATIC_DRAW);

	this.timeBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, this.timeBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(time), gl.STATIC_DRAW);
	
	this.indexBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(index), gl.STATIC_DRAW);
	this.indexCount = index.length;
};

Jappsy.GLParticleSystem.prototype.render = function(object) {
	var s = object.object;
	
	if (s.startTime == 0)
		return;
	
	var currentTime = (new Date).getTime() * 60 / 1000;
	if ((s.startTime + s.maxTime) < currentTime) {
		if (s.repeat == 0) {
			s.startTime = 0;
			return;
		}
		
		if (s.repeat > 0)
			s.repeat--;
		s.startTime = currentTime + Math.random()*s.startTimeRange;
		s.color.random(s.colorRange);
	}
	
	if (currentTime < s.startTime)
		return;

	var scene = object.scene;
	var context = scene.context;
	var gl = context.gl;
	var shader = context.shaderParticle;
	
	gl.enable(gl.BLEND);
	gl.blendFunc(gl.SRC_ALPHA, gl.ONE); // Искры
	//gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA); // Крапли краски
	//gl.blendFunc(gl.SRC_COLOR, gl.ONE_MINUS_SRC_COLOR); // Пузыри
	//gl.blendFunc(gl.ONE_MINUS_DST_COLOR, gl.ONE_MINUS_SRC_COLOR); // Хлопья / Конфети
	//gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA); gl.blendEquation(gl.FUNC_REVERSE_SUBTRACT);

	var index = shader.bind(0);

	var vCam = new Vec3().subtract(scene.camera.position, scene.camera.target).normalize();
	var vX = new Vec3().cross(vCam, scene.camera.head).normalize();
	var vY = new Vec3().cross(vX, vCam).normalize();

	//scene.modelView16fv.set(scene.camera.view16fv);
	//scene.modelViewProjection16fv.multiply(scene.camera.projection16fv, scene.modelView16fv);
	
	scene.modelView16fv.multiply(scene.camera.view16fv, object.objectMatrix).multiply(object.modelMatrix);
	scene.modelViewProjection16fv.multiply(scene.camera.projection16fv, scene.modelView16fv);
	
	gl.uniformMatrix4fv(shader.uModelViewProjectionMatrix, false, scene.modelViewProjection16fv);
	
	gl.enableVertexAttribArray(shader.aVertexPosition);
	gl.enableVertexAttribArray(shader.aTextureCoord);
	gl.enableVertexAttribArray(shader.aVelocity);
	gl.enableVertexAttribArray(shader.aAcceleration);
	gl.enableVertexAttribArray(shader.aTime);
	
	//gl.enable(gl.TEXTURE_2D);

	gl.uniform3fv(shader.uPixelX, vX);
	gl.uniform3fv(shader.uPixelY, vY);

	{
		gl.uniform3fv(shader.uTime, new Float32Array([currentTime - s.startTime, s.maxTime, 0]));
		gl.uniform3fv(shader.uColor, s.color);
		
		gl.bindBuffer(gl.ARRAY_BUFFER, s.vertexBuffer);
		gl.vertexAttribPointer(shader.aVertexPosition, 4, gl.FLOAT, false, 0, 0);
		
		gl.bindBuffer(gl.ARRAY_BUFFER, s.textureBuffer);
		gl.vertexAttribPointer(shader.aTextureCoord, 2, gl.FLOAT, false, 0, 0);

		gl.bindBuffer(gl.ARRAY_BUFFER, s.velocityBuffer);
		gl.vertexAttribPointer(shader.aVelocity, 4, gl.FLOAT, false, 0, 0);

		gl.bindBuffer(gl.ARRAY_BUFFER, s.accelBuffer);
		gl.vertexAttribPointer(shader.aAcceleration, 4, gl.FLOAT, false, 0, 0);

		gl.bindBuffer(gl.ARRAY_BUFFER, s.timeBuffer);
		gl.vertexAttribPointer(shader.aTime, 4, gl.FLOAT, false, 0, 0);

		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, s.indexBuffer);
		gl.drawElements(gl.TRIANGLES, s.indexCount, gl.UNSIGNED_SHORT, 0);
	}
	
	gl.disableVertexAttribArray(shader.aVertexPosition);
	gl.disableVertexAttribArray(shader.aTextureCoord);
	gl.disableVertexAttribArray(shader.aVelocity);
	gl.disableVertexAttribArray(shader.aAcceleration);
	gl.disableVertexAttribArray(shader.aTime);
	
	context.cleanup(index);

	//gl.disable(gl.TEXTURE_2D);
	
	context.resetBlend();
};

///////////////////////////

Jappsy.GLParticles = function(context) {
	this.context = context;
	this.list = {};
};

Jappsy.GLParticles.prototype.release = function() {
	for (var key in this.list) {
		this.list[key].release();
	}
	this.list = {};
	this.context = null;
};

Jappsy.GLParticles.prototype.get = function(key) {
	return this.list[key] || null;
};

Jappsy.GLParticles.prototype.createParticleSystem = function(key) {
	if (this.list[key] === undefined)
		this.list[key] = new Jappsy.GLParticleSystem(this.context, key);

	return this.list[key];
};

Jappsy.GLParticles.prototype.destroyParticleSystem = function(key) {
	if (this.list[key] === undefined)
		return;

	var s = this.list[key];
	delete this.list[key];
	
	s.release();
};
