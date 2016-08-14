Jappsy.TouchScreen = function(parent, callback, view) {
	this.parent = parent;
	this.onTouch = callback;
	this.oView = view;
	this.recordDistance = view.height / 20;	// 1/20 от высоты экрана
	this.minimalDistance = view.height / 6; // 1/6 от высоты экрана
	this.swipeDistance = view.height * 3 / 4; // 3/4 от высоты экрана
	this.clickList = [];
	this.trackList = [];
	
	this.trackLast = null;
	this.trackSpeed = null;
	this.trackingList = [];
	this.trackingLast = [];
	
	this.touchList = [];
	this.touchLast = null;
	this.touchCancel = false;
	this.touchStart = null;
	
	this.touchTime = null;
	this.touchDown = false;
	this.mouseTime = null;
	this.mouseDown = false;
	this.mouseRepeatTime = null;
	this.touchTimeout = null;

	this.defaultOnTouchStart = document.ontouchstart;
	this.defaultOnTouchEnd = document.ontouchend;
	this.defaultOnTouchMove = document.ontouchmove;
	this.defaultOnMouseDown = document.onmousedown;
	this.defaultOnMouseUp = document.onmouseup;
	this.defaultOnMouseOut = document.onmouseout;
	this.defaultOnMouseMove = document.onmousemove;

	document.ontouchstart = this.onTouchStart.bind(this);
	document.ontouchend = this.onTouchEnd.bind(this);
	document.ontouchmove = this.onTouchMove.bind(this);
	document.onmousedown = this.onMouseDown.bind(this);
	document.onmouseup = this.onMouseUp.bind(this);
	document.onmouseout = this.onMouseOut.bind(this);
	document.onmousemove = this.onMouseMove.bind(this);	
};

Jappsy.TouchScreen.prototype.release = function() {
	document.ontouchstart = this.defaultOnTouchStart;
	document.ontouchend = this.defaultOnTouchEnd;
	document.ontouchmove = this.defaultOnTouchMove;
	document.onmousedown = this.defaultOnMouseDown;
	document.onmouseup = this.defaultOnMouseUp;
	document.onmouseout = this.defaultOnMouseOut;
	document.onmousemove = this.defaultOnMouseMove;
	
	this.parent = null;
	this.onTouch = null;
	this.oView = null;
	this.clickList = [];
	this.trackList = [];
	
	this.trackLast = null;
	this.trackSpeed = null;
	this.trackingList = [];
	this.trackingLast = [];
	
	this.touchList = [];
	this.touchLast = null;
	this.touchStart = null;
	
	if (this.touchTimeout !== null) {
		cancelTimer(this.touchTimeout);
		this.touchTimeout = null;
	}
};

Jappsy.TouchScreen.prototype.trackEvent = function(name, x, y, w, h, callback) {
	this.trackList.push({ x: x, y: y, w: w, h: h, name: name, callback: callback });
};

Jappsy.TouchScreen.prototype.clickEvent = function(name, x, y, w, h, callback) {
	this.clickList.push({ x: x, y: y, w: w, h: h, name: name, callback: callback });
};

Jappsy.TouchScreen.prototype.checkBounds = function(x, y) {
	var rect = this.oView.getBoundingClientRect();
	if ((x < rect.left) || (x >= rect.right) ||
		(y < rect.top) || (y >= rect.bottom)) {
		return false;
	}
	return true;
};

Jappsy.TouchScreen.prototype.onTimeout = function() {
	this.touchTimeout = null;
	this.touchCancel = true;
};

Jappsy.TouchScreen.prototype.canStartTouch = function() {
	if (this.mouseTime == null) return true;
	var time = new Date().getTime();
	return (time >= (this.mouseTime + 1000));
};

Jappsy.TouchScreen.prototype.canStartMouse = function() {
	if (this.touchTime == null) {
		if (this.mouseRepeatTime == null) {
			return true;
		} else {
			var time = new Date().getTime();
			return (time >= this.mouseRepeatTime + 200);
		}
	} else {
		var time = new Date().getTime();
		if (this.mouseRepeatTime == null) {
			return (time >= (this.touchTime + 1000));
		} else {
			return ((time >= (this.touchTime + 1000)) &&
				(time >= this.mouseRepeatTime + 200));
		}
	}
};

Jappsy.TouchScreen.prototype.onTouchStart = function(e) {
	var x = e.touches[0].clientX;
	var y = e.touches[0].clientY;
	if (this.checkBounds(x, y)) {
		if (this.canStartTouch()) {
			this.touchDown = true;
			this.touchList = [];
			this.touchLast = null;
			this.touchStart = new Date().getTime();
			this.record(x, y);
			if (!this.touchCancel) {
				this.touchTimeout = setTimeout(this.onTimeout.bind(this), 1000);
			}
		}
	} else if (this.defaultOnTouchStart) {
		this.defaultOnTouchStart(e);
	}
};

Jappsy.TouchScreen.prototype.onTouchEnd = function(e) {
	if (this.touchDown) {
		this.touchDown = false;
		this.touchTime = new Date().getTime();
		this.analyze(e.touches[0].clientX, e.touches[0].clientY);
	} else if (this.defaultOnTouchEnd) {
		this.defaultOnTouchEnd(e);
	}
};

Jappsy.TouchScreen.prototype.onTouchMove = function(e) {
	if (this.touchDown) {
		var x = e.touches[0].clientX;
		var y = e.touches[0].clientY;
		this.record(x, y);
		e.preventDefault();
	} else if (this.defaultOnTouchMove) {
		this.defaultOnTouchMove(e);
	}
};

Jappsy.TouchScreen.prototype.onMouseDown = function(e) {
	var x = e.clientX;
	var y = e.clientY;
	if (this.checkBounds(x, y)) {
		if (this.canStartMouse()) {
			this.mouseDown = true;
			this.touchList = [];
			this.touchLast = null;
			this.touchStart = new Date().getTime();
			this.record(x, y);
			if (!this.touchCancel) {
				this.touchTimeout = setTimeout(this.onTimeout.bind(this), 1000);
			}
		}
		e.target.focus();
		e.preventDefault();
	} else if (this.defaultOnMouseDown) {
		this.defaultOnMouseDown(e);
	}
};

Jappsy.TouchScreen.prototype.onMouseUp = function(e) {
	if (this.mouseDown) {
		this.mouseDown = false;
		this.mouseRepeatTime = this.mouseTime = new Date().getTime();
		this.analyze(e.clientX, e.clientY);
	} else if (this.defaultOnMouseUp) {
		this.defaultOnMouseUp(e);
	}
};

Jappsy.TouchScreen.prototype.onMouseOut = function(e) {	
	if (this.mouseDown) {
		this.mouseDown = false;
		this.mouseTime = new Date().getTime();
		this.analyze(e.clientX, e.clientY);
	} else if (this.defaultOnMouseOut) {
		this.defaultOnMouseOut(e);
	}
};

Jappsy.TouchScreen.prototype.onMouseMove = function(e) {
	if (this.mouseDown) {
		var x = e.clientX;
		var y = e.clientY;
		this.record(x, y);
	} else if (this.defaultOnMouseMove) {
		this.defaultOnMouseMove(e);
	}
};

Jappsy.TouchScreen.prototype.analyze = function(x, y) {
	var rect = this.oView.getBoundingClientRect();
	x = Math.round((x - rect.left) * this.oView.width / (rect.right - rect.left));
	y = Math.round((y - rect.top) * this.oView.height / (rect.bottom - rect.top));

	this.recordTrack(x, y, true);
	
	if (this.touchTimeout != null) {
		clearTimeout(this.touchTimeout);
		this.touchTimeout = null;
	}
	if (this.touchCancel) {
		this.touchCancel = false;
		return;
	}
	
	var list = this.touchList;
	if (list.length > 0) {
		var type = null;
		
		var x1 = list[0].x;
		var y1 = list[0].y;
		var x2 = list[list.length-1].x;
		var y2 = list[list.length-1].y;
	
		if (list.length == 1) {
			if (this.clickList.length > 0) {
				for (var i = this.clickList.length-1; i >= 0; i--) {
					var ww = this.clickList[i].w;
					var hh = this.clickList[i].h;
					var xx = this.clickList[i].x;
					var yy = this.clickList[i].y;
					if ((x1 >= xx) && (x1 < (xx + ww)) &&
						(y1 >= yy) && (y1 < (yy + hh))
					) {
						type = "click " + this.clickList[i].name;
						if (this.clickList[i].callback(type)) {
							break;
						}
					}
				}
			}
			if (type == null)
				type = "none";
		} else {
			// Определение кругового движения по часовой стрелке или против
			var cx, cy;
			{
				// Вычисляем центр окружности описывающей точки
				var xmin, xmax, ymin, ymax;
				xmin = xmax = x1;
				ymin = ymax = y1;
				for (i in list) {
					var xx = list[i].x;
					var yy = list[i].y;
					if (xx < xmin) xmin = xx;
					if (xx > xmax) xmax = xx;
					if (yy < ymin) ymin = yy;
					if (yy > ymax) ymax = yy;
				}
				cx = (xmin + xmax) / 2;
				cy = (ymin + ymax) / 2;
				
				// Вычисляем средний радиус и определяем число смещений по кругу
				var mr = 0; // Средний радиус
				var cw = 0; // Число смещений по часовой стрелке
				var bw = 0; // Число смещений против часовой стрелки
				var ca = 0; // Угол смещения по часовой стрелке
				var ba = 0;	// Угол смещения против часовой стрелки
				var lx = x2 - cx;
				var ly = y2 - cy;
				var la = Math.atan2(ly, lx); // Угол последней точки
				for (var i = 0; i < list.length; i++) {
					// Координаты относительно центра
					var xx = list[i].x - cx;
					var yy = list[i].y - cy;
					// Растояние до точки
					var r = Math.floor(Math.sqrt(xx * xx + yy * yy));
					// Направление движения по часовой стрелке или против
					var s = lx * yy - ly * xx;
					var na = Math.atan2(yy, xx);
					var a = (na - la) * 180.0 / Math.PI;
					while (a < -180.0) a += 360.0;
					while (a > 180.0) a -= 360.0;
					if (i != 0) {
						if (s > 0) { cw++; ca += a; }
						else if (s < 0) { bw++; ba -= a; }
					}
					// Кешируем вычисления
					list[i].r = r;
					mr += r;
					la = na;
					lx = xx;
					ly = yy;
				}
				mr = Math.floor(mr / list.length);
				
				// Вычисляем процентное соотношение смещений и направление
				var md = 0;
				if ((cw != 0) || (bw != 0)) {
					if (cw > bw) {
						md = Math.floor((cw - bw) * 100 / cw);
					} else {
						md = -Math.floor((bw - cw) * 100 / bw);
					}
				}
				// Угол смещения
				var a = Math.abs(ba - ca);
				
				// Проверяем ровность круга (допустимое искажение радиуса 50% на каждую точку)
				if ((mr > this.minimalDistance) && (Math.abs(md) > 90)) {
					var circle = true;
					var drm = 0;
					for (i in list) {
						var dr = Math.floor(Math.abs((list[i].r / mr) - 1) * 100);
						if (dr > drm) drm = dr;
						if (dr > 50) {
							circle = false;
							break;
						}
					}
					if (circle) {
						var ac = Math.round(a / 90);
						if (ac > 2) {
							type = "circle";
						} else {
							type = "arc";
						}
						if (md > 0) type += " right";
						else type += " left";
						if (ac > 5) {
							type += " " + (ac * 90);
						}
					}
				}
			}
			
			// Определение свайпов и их направления
			if (type == null) {
				// Вычисляем расстояние
				var dx = x2 - x1;
				var dy = y2 - y1;
				var d = Math.floor(Math.sqrt(dx * dx + dy * dy));
				
				// Отбрасываем случайные или короткие свайпы
				var time = new Date().getTime();
				time -= this.touchStart;
				if ((d > this.minimalDistance) && ((list.length < 15) || (d >= this.swipeDistance))) {

					// Проверяем ровность линии (допустимое искажение 25% от длины прямой)
					var swipe = true;
					var c = x1 * y2 - x2 * y1;
					for (var i = list.length - 2; i > 0; i--) {
						// Расстояние до точки от отрезка (+ знак стороны)
						var p = (list[i].y * dx - list[i].x * dy + c) / d;
						var dp = Math.floor(Math.abs(p) * 100 / d);
						if (dp > 25) {
							swipe = false;
							break;
						}
					}

					if (swipe) {
						type = "swipe";

						var ax = Math.abs(dx);
						var ay = Math.abs(dy);
						var ad = 0;
						if (ax > ay) {
							if (d > this.swipeDistance) type += " long";
							ad = Math.floor((ax - ay) * 100 / ax);
							if (ad > 50) {
								if (dx > 0) type += " right";
								else type += " left";
							} else if (dx > 0) {
								if (dy < 0) type += " right top";
								else type += " right bottom";
							} else {
								if (dy < 0) type += " left top";
								else type += " left bottom";
							}
						} else {
							if (d > this.swipeDistance) type += " long";
							ad = Math.floor((ay - ax) * 100 / ay);
							if (ad > 50) {
								if (dy < 0) type += " top";
								else type += " bottom";
							} else if (dy < 0) {
								if (dx > 0) type += " right top";
								else type += " left top";
							} else {
								if (dx > 0) type += " right bottom";
								else type += " left bottom";
							}
						}
					}
				}
			}
		}
		
		if (type != null) {
			this.onTouch(type);
		}
	}
};

Jappsy.TouchScreen.prototype.recordTrack = function(x, y, stop) {
	var time = new Date().getTime();

	var cur = { x: x, y: y, time: time };
	
	if (this.trackLast == null) {
		this.trackSpeed = { x: 0, y: 0, time: 0 };
		this.trackLast = cur;
	} else {
		var elapsed = (time - this.trackLast.time);
		if (elapsed > 50) {
			if (elapsed > 250) {
				this.trackSpeed = { x: 0, y: 0, time: 0 };
				this.trackLast = cur;
			} else {
				var xs = ((x - this.trackLast.x) + this.trackSpeed.x * this.trackSpeed.time) / (elapsed + this.trackSpeed.time);
				var ys = ((y - this.trackLast.y) + this.trackSpeed.y * this.trackSpeed.time) / (elapsed + this.trackSpeed.time);
				this.trackSpeed = { x: xs, y: ys, time: elapsed };
				this.trackLast = cur;
			}
		}
	}
	
	var processed = false;
	for (var i = this.trackList.length-1; i >= 0; i--) {
		var track = this.trackList[i];
		var idx = this.trackingList.indexOf(track);
		var name = (track.name ? " " + track.name : "");

		if ((!stop) &&
			(x >= track.x) && (x < (track.x + track.w)) &&
			(y >= track.y) && (y < (track.y + track.h))
		) {
			if (idx < 0) {
				if (!processed) {
					// Track In (Enter)
					this.trackingList.push(track);
					this.trackingLast.push(cur);
					processed |= track.callback("enter" + name, cur, null, this.trackSpeed);
				}
			} else {
				if (!processed) {
					// Track Move
					var last = this.trackingLast[idx];
					var delta = { x: (x - last.x), y: (y - last.y), time: (time - last.time) };
					processed |= track.callback("move" + name, cur, delta, this.trackSpeed);
					this.trackingLast[idx] = cur;
				}
			}
		} else if (idx >= 0) {
			// Track Out (Leave)
			var last = this.trackingLast[idx];
			var delta = { x: (x - last.x), y: (y - last.y), time: (time - last.time) };
			track.callback("leave" + name, cur, delta, this.trackSpeed);
			this.trackingList.splice(idx, 1);
			this.trackingLast.splice(idx, 1);
		}
	}
	
	if (stop) {
		this.trackSpeed = null;
		this.trackLast = null;
	}
};

Jappsy.TouchScreen.prototype.record = function(x, y) {
	var rect = this.oView.getBoundingClientRect();
	x = Math.round((x - rect.left) * this.oView.width / (rect.right - rect.left));
	y = Math.round((y - rect.top) * this.oView.height / (rect.bottom - rect.top));

	this.recordTrack(x, y, false);

	var dist = null;
	if (this.touchLast != null) {
		var dx = this.touchLast.x - x;
		var dy = this.touchLast.y - y;
		dist = Math.floor(Math.sqrt(dx * dx + dy * dy));
	}

	if ((dist == null) || (dist > this.recordDistance)) {
		this.touchLast = { x: x, y: y };
		this.touchList.push({ x: x, y: y });
	}
};
