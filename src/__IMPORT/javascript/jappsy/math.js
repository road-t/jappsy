Jappsy.Rotation = function(prev) {
	this.rotate = 0;
	this.speed = 0;
	this.accel = 0;
	this.time = 0;
	this.target = -1;
	this.prev = prev || null;
	this.stopped = true;
	
	this.updated = false;
	this.state = 0;
};

Jappsy.Rotation.Angle = function(angle) {
	return angle - (Math.floor(angle / 360)*360);
};

Jappsy.assign(Jappsy.Rotation, {
	speedup: {
		maxSpeed: 720,
		minSpeed: 720-180,
		maxAccel: 720,
		minAccel: 720-360,
	},
	slowdown: {
		maxAccel: -500,
		minAccel: -400,
		prevSpeed: 360,	// Если больше speedup.maxSpeed то торможение не зависит от пред.барабана
	},
});

Jappsy.Rotation.prototype.start = function(target) {
	if (target === undefined) {
		this.accel = Math.random()*(Jappsy.Rotation.speedup.maxAccel - Jappsy.Rotation.speedup.minAccel)+Jappsy.Rotation.speedup.minAccel;
	} else {
		this.target = Jappsy.Rotation.Angle(target);
	}
	this.state = 0;
	this.stopped = false;
};

Jappsy.Rotation.prototype.check = function() {
	if (this.target >= 0) {
		// Запустить кручение если барабан тормозит или не крутится
		if ((this.speed < Jappsy.Rotation.speedup.maxSpeed) && (this.accel <= 0)) {
			this.accel = Math.random()*(Jappsy.Rotation.speedup.maxAccel - Jappsy.Rotation.speedup.minAccel)+Jappsy.Rotation.speedup.minAccel;
		}
		if ((this.speed >= Jappsy.Rotation.speedup.minSpeed) && (this.accel >= 0)) {
			if ((this.prev == null) || ((this.prev.speed < Jappsy.Rotation.slowdown.prevSpeed) && (this.prev.accel <= 0))) {
				this.accel = Math.random()*(Jappsy.Rotation.slowdown.maxAccel - Jappsy.Rotation.slowdown.minAccel)+Jappsy.Rotation.slowdown.minAccel;
			
				// Вычисляем примерное время на торможение и расстояние
				var resultTime = -this.speed / this.accel;
				var resultRotate = this.speed * resultTime + this.accel * resultTime * resultTime / 2;
				var absRotate = (resultRotate + this.rotate) / 360;
				absRotate = (absRotate - Math.floor(absRotate)) * 360;
				// Вычисляем куда ближе цель (чуть медленнее или чуть быстрее)
				var forwardRotate = 0;
				var backwardRotate = 0;
				if (absRotate > this.target) {
					forwardRotate = 360 - (absRotate - this.target);
					backwardRotate = absRotate - this.target;
				} else {
					forwardRotate = this.target - absRotate;
					backwardRotate = 360 - (this.target - absRotate);
				}
				if ((forwardRotate > backwardRotate) && (resultRotate > backwardRotate)) {
					resultRotate -= backwardRotate;
				} else {
					resultRotate += forwardRotate;
				}
				// Вычисляем окончательное ускорение для торможения
				var resultAccel = -this.speed * this.speed / (resultRotate * 2);
			
				this.accel = resultAccel;
				this.target = -1;
			}
		}
	}
};

Jappsy.Rotation.prototype.userMove = function(delta) {
	this.rotate += delta;
	this.speed = 0;
	this.accel = 0;
	this.updated = true;
};

Jappsy.Rotation.prototype.userRotate = function(delta, speed) {
	this.rotate += delta;
	this.speed = Math.abs(speed);
	if (this.speed > Jappsy.Rotation.speedup.maxSpeed)
		this.speed = Jappsy.Rotation.speedup.maxSpeed;
	this.accel = Math.random()*(Jappsy.Rotation.slowdown.maxAccel - Jappsy.Rotation.slowdown.minAccel)+Jappsy.Rotation.slowdown.minAccel;;
	this.state = (speed > 0) ? -1 : 1;
	this.updated = true;
};

Jappsy.Rotation.prototype.update = function(time) {
	var currentTime = time || (new Date).getTime();
	if (this.time == 0) {
		this.time = currentTime;
	} else {
		var deltaTime = (currentTime - this.time) / 1000.0;
		if (deltaTime > 0) {
			this.check();

			// Определяем текущую скорость, время на ускорение и время с постоянной скоростью
			var accelTime = deltaTime;
			var leftTime = 0;
			var resultSpeed = this.speed + this.accel * deltaTime;
			var resultAccel = this.accel;
			if (resultSpeed > Jappsy.Rotation.speedup.maxSpeed) { // Предельная скорость
				resultSpeed = Jappsy.Rotation.speedup.maxSpeed;
				// Вычисляем время до максимальной скорости
				if (this.accel != 0)
					accelTime = (resultSpeed - this.speed) / this.accel;
				else
					accelTime = 0;
				leftTime = deltaTime - accelTime;
				resultAccel = 0; // отрубаем ускорение
			} else if (resultSpeed < 0) { // Предельное торможение
				resultSpeed = 0;
				// Вычисляем время до полного торможения
				if (this.accel != 0)
					accelTime = (resultSpeed - this.speed) / this.accel;
				else
					accelTime = 0;
				leftTime = deltaTime - accelTime;
				resultAccel = 0; // отрубаем ускорение
			}
			var resultRotate = this.speed * accelTime + this.accel * accelTime * accelTime / 2 + resultSpeed * leftTime;
			if (this.state >= 0) {
				this.rotate = Jappsy.Rotation.Angle(this.rotate + resultRotate);
			} else {
				this.rotate = Jappsy.Rotation.Angle(this.rotate - resultRotate);
			}
			this.speed = resultSpeed;
			this.accel = resultAccel;
			this.time = currentTime;
			if (this.speed != 0) {
				this.stopped = false;
			}
			if (this.updated) {
				this.updated = false;
				return true;
			}
			return resultRotate != 0;
		}
	}
	if (this.updated) {
		this.updated = false;
		return true;
	}
	return false;
};

Jappsy.Rotation.prototype.offset = function() {
	var ofs = Math.round(Jappsy.Rotation.Angle(this.rotate) / 45);
	ofs -= Math.floor(ofs / 8) * 8;
	return ofs;
};

Jappsy.Rotation.stoppedBars = function(bars) {
	if ((bars[0].speed == 0) && (bars[1].speed == 0) && (bars[2].speed == 0)) {
		if ((!bars[0].stopped) && (!bars[1].stopped) && (!bars[2].stopped)) {
			bars[0].stopped = bars[1].stopped = bars[2].stopped = true;
			if ((bars[0].state == 0) && (bars[1].state == 0) && (bars[2].state == 0)) {
				return true;
			}
		}
	}
	return false;
};

Jappsy.Rotation.updateBars = function(bars, time) {
	var res1 = bars[0].update(time);
	var res2 = bars[1].update(time);
	var res3 = bars[2].update(time);
	return res1 || res2 || res3;
};

Jappsy.Rotation.calculateBars = function(bars, mask) {
	var ofs1 = bars[0].offset();
	var ofs2 = bars[1].offset();
	var ofs3 = bars[2].offset();
	if (ofs1 == 1) { // S1 ? ?
		if (ofs2 == 1) { // S1 S1 ?
			if (ofs3 == 1) { // S1 S1 S1
				mask[0] = mask[1] = mask[2] = 1;
				return 888;
			} else { // S1 S1 *
				mask[0] = mask[1] = 1; mask[2] = 0;
				return 88;
			}
		} else { // S1 * *
			mask[0] = 1; mask[1] = mask[2] = 0;
			return 8;
		}
	} else if (ofs1 == 3) { // S2 ? ?
		if (ofs2 == 3) { // S2 S2 ?
			if (ofs3 == 3) { // S2 S2 S2
				mask[0] = mask[1] = mask[2] = 2;
				return 888;
			} else { // S2 S2 *
				mask[0] = mask[1] = 2; mask[2] = 0;
				return 88;
			}
		} else if (ofs2 == 1) { // S2 S1 ?
			if (ofs3 == 3) { // S2 S1 S2
				mask[0] = mask[2] = 2; mask[1] = 1;
				return -8;
			} else { // S2 * *
				mask[0] = 2; mask[1] = mask[2] = 0;
				return 8;
			}
		} else { // S2 * *
			mask[0] = 2; mask[1] = mask[2] = 0;
			return 8;
		}
	} else if (ofs1 == 5) { // S3 ? ?
		if (ofs2 == 5) { // S3 S3 ?
			if (ofs3 == 5) { // S3 S3 S3
				mask[0] = mask[1] = mask[2] = 3;
				return 888;
			} else { // S3 S3 *
				mask[0] = mask[1] = 3; mask[2] = 0;
				return 88;
			}
		} else if (ofs2 == 3) { // S3 S2 ?
			if (ofs3 == 5) { // S3 S2 S3
				mask[0] = mask[2] = 3; mask[1] = 2;
				return -8;
			} else { // S3 * *
				mask[0] = 3; mask[1] = mask[2] = 0;
				return 8;
			}
		} else { // S3 * *
			mask[0] = 3; mask[1] = mask[2] = 0;
			return 8;
		}
	} else if (ofs1 == 6) { // M1 ? ?
		if (ofs2 == 6) { // M1 M1 ?
			if (ofs3 == 6) { // M1 M1 M1
				mask[0] = mask[1] = mask[2] = 1;
				return 444;
			} else { // M1 M1 *
				mask[0] = mask[1] = 1; mask[2] = 0;
				return 44;
			}
		} else { // M1 * *
			mask[0] = 1; mask[1] = mask[2] = 0;
			return 4;
		}
	} else if (ofs1 == 0) { // M2 ? ?
		if (ofs2 == 0) { // M2 M2 ?
			if (ofs3 == 0) { // M2 M2 M2
				mask[0] = mask[1] = mask[2] = 2;
				return 444;
			} else {
				mask[0] = mask[1] = 2; mask[2] = 0;
				return 44;
			}
		} else if (ofs2 == 6) { // M2 M1 ?
			if (ofs3 == 0) { // M2 M1 M2
				mask[0] = mask[2] = 2; mask[1] = 1;
				return -4;
			} else { // M2 * *
				mask[0] = 2; mask[1] = mask[2] = 0;
				return 4;
			}
		} else { // M2 * *
			mask[0] = 2; mask[1] = mask[2] = 0;
			return 4;
		}
	} else if (ofs1 == 2) { // M3 ? ?
		if (ofs2 == 2) { // M3 M3 ?
			if (ofs3 == 2) { // M3 M3 M3
				mask[0] = mask[1] = mask[2] = 3;
				return 444;
			} else { // M3 M3 *
				mask[0] = mask[1] = 3; mask[2] = 0;
				return 44;
			}
		} else if (ofs2 == 0) { // M3 M2 ?
			if (ofs3 == 2) { // M3 M2 M3
				mask[0] = mask[2] = 3; mask[1] = 2;
				return -4;
			} else { // M3 * *
				mask[0] = 3; mask[1] = mask[2] = 0;
				return 4;
			}
		} else { // M3 * *
			mask[0] = 3; mask[1] = mask[2] = 0;
			return 4;
		}
	}
	mask[0] = mask[1] = mask[2] = 0;
	return 0;
};

// (Временная функция) Положение барабанов приходит с сервера
Jappsy.Rotation.generateBars = function(bars, grace) {
	var ofs1, ofs2, ofs3;

	if (grace == 888) {
		ofs1 = ofs2 = ofs3 = Jappsy.random(3)*2+1; // S1 S1 S1 | S2 S2 S2 | S3 S3 S3
	} else if (grace == 444) {
		ofs1 = ofs2 = ofs3 = Jappsy.mod(Jappsy.random(3)*2-2, 8); // M1 M1 M1 | M2 M2 M2 | M3 M3 M3
	} else if (grace == 88) {
		ofs1 = ofs2 = Jappsy.random(3)*2+1; // S1 S1 ? | S2 S2 ? | S3 S3 ?
		ofs3 = Jappsy.mod(Jappsy.random(7)+1+ofs1, 8); // S1 S1 !S1 | S2 S2 !S2 | S3 S3 !S3
	} else if (grace == 44) {
		ofs1 = ofs2 = Jappsy.mod(Jappsy.random(3)*2-2, 8); // M1 M1 ? | M2 M2 ? | M3 M3 ?
		ofs3 = Jappsy.mod(Jappsy.random(7)+1+ofs1, 8); // M1 M1 !M1 | M2 M2 !M2 | M3 M3 !M3
	} else if (grace == 8) {
		ofs1 = Jappsy.random(3)*2+1; // S1 ? ? | S2 ? ? | S3 ? ?
		ofs2 = Jappsy.mod(Jappsy.random(7)+1+ofs1, 8); // S1 !S1 ? | S2 !S2 ? | S3 !S3 ?
		ofs3 = Jappsy.mod(Jappsy.random(7)+1+ofs1, 8); // S1 !S1 !S1 | S2 !S2 !S2 | S3 !S3 !S3
	} else if (grace == 4) {
		ofs1 = Jappsy.mod(Jappsy.random(3)*2-2, 8); // M1 ? ? | M2 ? ? | M3 ? ?
		ofs2 = Jappsy.mod(Jappsy.random(7)+1+ofs1, 8); // M1 !M1 ? | M2 !M2 ? | M3 !M3 ?
		ofs3 = Jappsy.mod(Jappsy.random(7)+1+ofs1, 8); // M1 !M1 !M1 | M2 !M2 !M2 | M3 !M3 !M3
	} else if (grace == -8) {
		ofs1 = ofs3 = Jappsy.random(2)*2+3; // S2 ? S2 | S3 ? S3
		ofs2 = ofs1 - 2; // S2 S1 S2 | S3 S2 S3
	} else if (grace == -4) {
		ofs1 = ofs3 = Jappsy.random(2)*2; // M2 ? M2 | M3 ? M3
		ofs2 = Jappsy.mod(ofs1 - 2, 8); // M2 M1 M2 | M3 M2 M3
	} else {
		ofs1 = Jappsy.random(2)*3+4; // !S!M ? ?
		ofs2 = Jappsy.random(8);
		ofs3 = Jappsy.random(8);
	}

	bars[0].start(ofs1 * 45);
	bars[1].start(ofs2 * 45);
	bars[2].start(ofs3 * 45);
};

var tempseq = [0, -4, 0, 4, 0, -8, 0, 0, 0, 0, 0, 8, 0, 0, 44, 88, 444, 888];
var tempofs = 0;
Jappsy.Rotation.generateGrace = function(bars) {
	var graces = [0, 4, 8, -4, -8, 44, 88, 444, 888];
	var grace = 0;
	grace = tempseq[tempofs]; tempofs++; if (tempofs >= tempseq.length) tempofs = 0;
	/*
	if (Jappsy.random(4) == 0) {
		grace = graces[Jappsy.random(8)+1];
	}
	*/
	Jappsy.Rotation.generateBars(bars, grace)
	return grace;
};

var tempdblseq = [0, 1, 1, 0];
var tempdblofs = 0;
Jappsy.Rotation.generateDouble = function() {
	var index = 0;
	/*
	index = Jappsy.random(2);
	*/
	index = tempdblseq[tempdblofs]; tempdblofs++; if (tempdblofs >= tempdblseq.length) tempdblofs = 0;
	return index;
};
