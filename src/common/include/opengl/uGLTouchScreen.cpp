/*
 * Copyright (C) 2016 The Jappsy Open Source Project (http://jappsy.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "uGLTouchScreen.h"
#include <core/uMemory.h>
#include <core/uSystem.h>
#include <opengl/uGLRender.h>
#include <math.h>

RefGLTouchScreen::RefGLTouchScreen(GLRender* context, onTouchCallback callback) {
	this->context = context;
	onTouch = callback;
	update();
	clickList = NULL;
	trackList = NULL;
	
	trackLast = NULL;
	trackSpeed = NAN;
	trackingList = NULL;
	trackingLast = NULL;
	
	touchList = NULL;
	touchLast = NULL;
	touchCancel = false;
	touchStart = 0;
	
	touchTime = 0;
	touchDown = false;
	mouseTime = 0;
	mouseDown = false;
	mouseRepeatTime = 0;
	touchTimeout = NULL;
}

void RefGLTouchScreen::release() {
	clearTimeout();
	handler.release();
}

RefGLTouchScreen::~RefGLTouchScreen() {
	release();
}

void RefGLTouchScreen::update() {
	GLfloat dimension = context->frame->height < context->frame->width ? context->frame->height : context->frame->width;
	recordDistance = dimension / 20.0f;
	minimalDistance = dimension / 6.0f;
	swipeDistance = dimension * 3.0f / 4.0f;
}

bool RefGLTouchScreen::checkBounds(float x, float y) {
	if ((x < 0) || (x >= context->frame->width) ||
		(y < 0) || (y >= context->frame->height))
		return false;
	return true;
}

void RefGLTouchScreen::trackEvent(const wchar_t* name, float x, float y, float w, float h, GLTouchEvent::Callback callback) {
	// TODO: this.trackList.push({ x: x, y: y, w: w, h: h, name: name, callback: callback });
}

void RefGLTouchScreen::clickEvent(const wchar_t* name, float x, float y, float w, float h, GLTouchEvent::Callback callback) {
	// TODO: this.clickList.push({ x: x, y: y, w: w, h: h, name: name, callback: callback });
}

void onTouchTimeout(const Object& userData) {
	(*((GLTouchScreen*)&(userData)))->onTimeout();
}

void RefGLTouchScreen::setTimeout(int delay) {
	if (touchTimeout == NULL) {
		touchTimeout = handler->postDelayed(onTouchTimeout, delay, this);
	}
}

void RefGLTouchScreen::clearTimeout() {
	if (touchTimeout != NULL) {
		handler->remove(touchTimeout);
		touchTimeout = NULL;
	}
}

void RefGLTouchScreen::onTimeout() {
	touchTimeout = NULL;
	touchCancel = true;
}

bool RefGLTouchScreen::canStartTouch() {
	if (mouseTime == 0) return true;
	uint64_t time = currentTimeMillis();
	return (time >= (mouseTime + 1000));
}

bool RefGLTouchScreen::canStartMouse() {
	if (touchTime == 0) {
		if (mouseRepeatTime == 0)
			return true;
		else {
			uint64_t time = currentTimeMillis();
			return (time >= mouseRepeatTime + 200);
		}
	} else {
		uint64_t time = currentTimeMillis();
		if (mouseRepeatTime == 0)
			return (time >= (touchTime + 1000));
		else
			return ((time >= (touchTime + 1000)) &&
					(time >= mouseRepeatTime + 200));
	}
}

void RefGLTouchScreen::onTouchStart(MotionEvent* event) {
/*
	MotionPointer* pointer = event->getPointer(0);
	float x = pointer->x;
	float y = pointer->y;
	if (checkBounds(x, y)) {
		if (canStartTouch()) {
			touchDown = true;
			touchList = [];
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
 */
}

void RefGLTouchScreen::onTouchEnd(MotionEvent* event) {
	if (touchDown) {
		touchDown = false;
		touchTime = currentTimeMillis();
		MotionPointer* pointer = event->getPointer(0);
		analyze(pointer->x, pointer->y);
	}
}

void RefGLTouchScreen::onTouchMove(MotionEvent* event) {
	if (touchDown) {
		MotionPointer* pointer = event->getPointer(0);
		record(pointer->x, pointer->y);
	}
}

void RefGLTouchScreen::onMouseDown(MotionEvent* event) {
	/*
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
	 */
}

void RefGLTouchScreen::onMouseUp(MotionEvent* event) {
	if (mouseDown) {
		mouseDown = false;
		mouseRepeatTime = mouseTime = currentTimeMillis();
		MotionPointer* pointer = event->getPointer(0);
		analyze(pointer->x, pointer->y);
	}
}

void RefGLTouchScreen::onMouseOut(MotionEvent* event) {
	if (mouseDown) {
		mouseDown = false;
		mouseTime = currentTimeMillis();
		MotionPointer* pointer = event->getPointer(0);
		analyze(pointer->x, pointer->y);
	}
}

void RefGLTouchScreen::onMouseMove(MotionEvent* event) {
	if (mouseDown) {
		MotionPointer* pointer = event->getPointer(0);
		record(pointer->x, pointer->y);
	}
}

void RefGLTouchScreen::analyze(float x, float y) {
	x = roundf(x * context->width / context->frame->width);
	y = roundf(y * context->height / context->frame->height);
	
	recordTrack(x, y, true);
	
	if (touchTimeout != NULL) {
		handler->removeCallbacks(onTouchTimeout);
		clearTimeout();
	}
	if (touchCancel) {
		touchCancel = false;
		return;
	}
	
	/*
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
	 */
}

void RefGLTouchScreen::recordTrack(float x, float y, bool stop) {
	/*
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
	 */
}

void RefGLTouchScreen::record(float x, float y) {
	/*
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
	 */
}

