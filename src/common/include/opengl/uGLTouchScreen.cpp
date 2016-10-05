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

GLTouchScreen::GLTouchScreen(GLRender* context, onTouchCallback callback, void* userData) {
	this->context = context;
	onTouch = callback;
	this->userData = userData;
//	update();
}

GLTouchScreen::~GLTouchScreen() {
	{
		int32_t count = clickList.count();
		GLTouchObject** items = clickList.items();
		for (int i = 0; i < count; i++) {
			delete items[i];
		}
	}

	{
		int32_t count = trackList.count();
		GLTouchObject** items = trackList.items();
		for (int i = 0; i < count; i++) {
			delete items[i];
		}
	}

	/*
	{
		int32_t count = trackList.count();
		GLTouchObject** items = trackList.items();
		for (int i = 0; i < count; i++) {
			delete items[i];
		}
	}
	
	{
		int32_t count = trackList.count();
		GLTouchObject** items = trackList.items();
		for (int i = 0; i < count; i++) {
			delete items[i];
		}
	}
	 */
}

void GLTouchScreen::update(float width, float height) {
	GLfloat dimension = context->height < context->width ? context->height : context->width;
	recordDistance = dimension / 20.0f;
	minimalDistance = dimension / 6.0f;
	swipeDistance = dimension * 3.0f / 4.0f;
	
	this->width = width;
	this->height = height;
}

void GLTouchScreen::trackEvent(const CString& name, float x, float y, float w, float h, GLTouchObject::Callback callback, void* userData) {
	trackList.push( new GLTouchObject(name, x, y, w, h, callback, userData) );
}

void GLTouchScreen::clickEvent(const CString& name, float x, float y, float w, float h, GLTouchObject::Callback callback, void* userData) {
	clickList.push( new GLTouchObject(name, x, y, w, h, callback, userData) );
}

bool GLTouchScreen::checkBounds(float x, float y) {
	if ((x < 0) || (x >= context->frame->width) ||
		(y < 0) || (y >= context->frame->height))
		return false;
	return true;
}

bool GLTouchScreen::canStartTouch() {
	if (mouseTime == 0) return true;
	uint64_t time = currentTimeMillis();
	return (time >= (mouseTime + 1000));
}

bool GLTouchScreen::canStartMouse() {
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

void GLTouchScreen::checkTimeout() {
	uint64_t time = currentTimeMillis();
	if ((touchTimeout != 0) && (touchTimeout <= time)){
		touchTimeout = 0;
		touchCancel = true;
	}
}

void GLTouchScreen::onTouchStart(MotionEvent* event) {
	checkTimeout();
	
	MotionPointer* pointer = event->getPointer(0);
	float x = pointer->x;
	float y = pointer->y;
	if (checkBounds(x, y)) {
		if (canStartTouch()) {
			touchDown = true;
			touchList.clear();
			touchLast = NULL;
			touchStart = currentTimeMillis();
			record(x, y);
			if (!touchCancel) {
				touchTimeout = touchStart + 1000;
			}
		}
	}
}

void GLTouchScreen::onTouchEnd(MotionEvent* event) {
	checkTimeout();
	
	if (touchDown) {
		touchDown = false;
		touchTime = currentTimeMillis();
		MotionPointer* pointer = event->getPointer(0);
		analyze(pointer->x, pointer->y);
	}
}

void GLTouchScreen::onTouchMove(MotionEvent* event) {
	checkTimeout();
	
	if (touchDown) {
		MotionPointer* pointer = event->getPointer(0);
		record(pointer->x, pointer->y);
	}
}

void GLTouchScreen::onMouseDown(MotionEvent* event) {
	checkTimeout();
	
	MotionPointer* pointer = event->getPointer(0);
	float x = pointer->x;
	float y = pointer->y;
	if (checkBounds(x, y)) {
		if (canStartMouse()) {
			mouseDown = true;
			touchList.clear();
			touchLast = NULL;
			touchStart = currentTimeMillis();
			record(x, y);
			if (!touchCancel) {
				touchTimeout = touchStart + 1000;
			}
		}
	}
}

void GLTouchScreen::onMouseUp(MotionEvent* event) {
	if (mouseDown) {
		mouseDown = false;
		mouseRepeatTime = mouseTime = currentTimeMillis();
		MotionPointer* pointer = event->getPointer(0);
		analyze(pointer->x, pointer->y);
	}
}

void GLTouchScreen::onMouseOut(MotionEvent* event) {
	if (mouseDown) {
		mouseDown = false;
		mouseTime = currentTimeMillis();
		MotionPointer* pointer = event->getPointer(0);
		analyze(pointer->x, pointer->y);
	}
}

void GLTouchScreen::onMouseMove(MotionEvent* event) {
	if (mouseDown) {
		MotionPointer* pointer = event->getPointer(0);
		record(pointer->x, pointer->y);
	}
}

void GLTouchScreen::analyze(float x, float y) {
	x = roundf(x * width / context->frame->width);
	y = roundf(y * height / context->frame->height);
	
	if (context->sceneRatio > context->ratio) { // Экран уже чем треубется
		x = roundf(x * context->width / width);
		y = roundf((((y / height) - 0.5) * context->sceneRatio / context->ratio + 0.5) * context->height);
	} else {
		x = roundf((((x / width) - 0.5) * context->ratio / context->sceneRatio + 0.5) * context->width);
		y = roundf(y * context->height / height);
	}

	recordTrack(x, y, true);
	
	if (touchTimeout != 0) {
		touchTimeout = 0;
	}
	if (touchCancel) {
		touchCancel = false;
		return;
	}
	
	int32_t listlength = touchList.count();
	Vec3* list = touchList.items();
	if (listlength > 0) {
		CString type;
		
		GLfloat x1 = list[0].x;
		GLfloat y1 = list[0].y;
		GLfloat x2 = list[listlength - 1].x;
		GLfloat y2 = list[listlength - 1].y;
		
		if (listlength == 1) {
			int32_t count = clickList.count();
			if (count > 0) {
				GLTouchObject** items = clickList.items();
				for (int i = count - 1; i >= 0; i--) {
					GLfloat ww = items[i]->w;
					GLfloat hh = items[i]->h;
					GLfloat xx = items[i]->x;
					GLfloat yy = items[i]->y;
					if ((x1 >= xx) && (x1 < (xx + ww)) &&
						(y1 >= yy) && (y1 < (yy + hh))
					) {
						type = L"click ";
						type += items[i]->name;
						if (items[i]->onevent(type, NULL, NULL, NULL, items[i]->userData)) {
							break;
						}
					}
				}
			}
			if (type.m_length == 0)
				type = L"none";
		} else {
			// Определение кругового движения по часовой стрелке или против
			GLfloat cx, cy;
			{
				// Вычисляем центр окружности описывающей точки
				GLfloat xmin, xmax, ymin, ymax;
				xmin = xmax = x1;
				ymin = ymax = y1;
				for (int i = 0; i < listlength; i++) {
					GLfloat xx = list[i].x;
					GLfloat yy = list[i].y;
					if (xx < xmin) xmin = xx;
					if (xx > xmax) xmax = xx;
					if (yy < ymin) ymin = yy;
					if (yy > ymax) ymax = yy;
				}
				cx = (xmin + xmax) / 2.0;
				cy = (ymin + ymax) / 2.0;
				
				// Вычисляем средний радиус и определяем число смещений по кругу
				GLfloat mr = 0; // Средний радиус
				GLfloat cw = 0; // Число смещений по часовой стрелке
				GLfloat bw = 0; // Число смещений против часовой стрелки
				GLfloat ca = 0; // Угол смещения по часовой стрелке
				GLfloat ba = 0;	// Угол смещения против часовой стрелки
				GLfloat lx = x2 - cx;
				GLfloat ly = y2 - cy;
				GLfloat la = atan2f(ly, lx); // Угол последней точки
				for (int i = 0; i < listlength; i++) {
					// Координаты относительно центра
					GLfloat xx = list[i].x - cx;
					GLfloat yy = list[i].y - cy;
					// Растояние до точки
					GLfloat r = floorf(sqrtf(xx * xx + yy * yy));
					// Направление движения по часовой стрелке или против
					GLfloat s = lx * yy - ly * xx;
					GLfloat na = atan2f(yy, xx);
					GLfloat a = (na - la) * 180.0 / M_PI;
					while (a < -180.0) a += 360.0;
					while (a > 180.0) a -= 360.0;
					if (i != 0) {
						if (s > 0) { cw++; ca += a; }
						else if (s < 0) { bw++; ba -= a; }
					}
					// Кешируем вычисления
					list[i].z = r;
					mr += r;
					la = na;
					lx = xx;
					ly = yy;
				}
				mr = floorf(mr / (GLfloat)listlength);
				
				// Вычисляем процентное соотношение смещений и направление
				GLfloat md = 0;
				if ((cw != 0) || (bw != 0)) {
					if (cw > bw) {
						md = floorf((cw - bw) * 100.0 / cw);
					} else {
						md = -floorf((bw - cw) * 100.0 / bw);
					}
				}
				// Угол смещения
				GLfloat a = fabsf(ba - ca);
				
				// Проверяем ровность круга (допустимое искажение радиуса 50% на каждую точку)
				if ((mr > minimalDistance) && (fabsf(md) > 90.0)) {
					bool circle = true;
					GLfloat drm = 0;
					for (int i = 0; i < listlength; i++) {
						GLfloat dr = floorf(fabsf((list[i].z / mr) - 1.0) * 100.0);
						if (dr > drm) drm = dr;
						if (dr > 50.0) {
							circle = false;
							break;
						}
					}
					if (circle) {
						int ac = roundf(a / 90.0);
						if (ac > 2) {
							type = L"circle";
						} else {
							type = L"arc";
						}
						if (md > 0) type += L" right";
						else type += L" left";
						if (ac > 5) {
							type += L" ";
							type += (ac * 90);
						}
					}
				}
			}
			
			// Определение свайпов и их направления
			if (type.m_length == 0) {
				// Вычисляем расстояние
				GLfloat dx = x2 - x1;
				GLfloat dy = y2 - y1;
				GLfloat d = floorf(sqrtf(dx * dx + dy * dy));
				
				// Отбрасываем случайные или короткие свайпы
				uint64_t time = currentTimeMillis();
				time -= touchStart;
				if ((d > minimalDistance) && ((listlength < 15) || (d >= swipeDistance))) {
					
					// Проверяем ровность линии (допустимое искажение 25% от длины прямой)
					bool swipe = true;
					GLfloat c = x1 * y2 - x2 * y1;
					for (int i = listlength - 2; i > 0; i--) {
						// Расстояние до точки от отрезка (+ знак стороны)
						GLfloat p = (list[i].y * dx - list[i].x * dy + c) / d;
						GLfloat dp = floorf(fabsf(p) * 100.0 / d);
						if (dp > 25) {
							swipe = false;
							break;
						}
					}
					
					if (swipe) {
						type = L"swipe";
						
						GLfloat ax = fabsf(dx);
						GLfloat ay = fabsf(dy);
						GLfloat ad = 0;
						if (ax > ay) {
							if (d > swipeDistance) type += L" long";
							ad = floorf((ax - ay) * 100.0 / ax);
							if (ad > 50) {
								if (dx > 0) type += L" right";
								else type += L" left";
							} else if (dx > 0) {
								if (dy < 0) type += L" right top";
								else type += L" right bottom";
							} else {
								if (dy < 0) type += L" left top";
								else type += L" left bottom";
							}
						} else {
							if (d > swipeDistance) type += L" long";
							ad = floorf((ay - ax) * 100.0 / ay);
							if (ad > 50) {
								if (dy < 0) type += L" top";
								else type += L" bottom";
							} else if (dy < 0) {
								if (dx > 0) type += L" right top";
								else type += L" left top";
							} else {
								if (dx > 0) type += L" right bottom";
								else type += L" left bottom";
							}
						}
					}
				}
			}
		}
		
		if (type.m_length != 0) {
			onTouch(type, userData);
		}
	}
}

void GLTouchScreen::recordTrack(float x, float y, bool stop) {
	uint64_t time = currentTimeMillis();

	GLTouchPoint cur(x, y, time);
	
	if (trackLast == NULL) {
		trackSpeed.update(0, 0, 0);
		_trackLast = cur;
		trackLast = &_trackLast;
	} else {
		uint64_t elapsed = (time - trackLast->time);
		if (elapsed > 50) {
			if (elapsed > 250) {
				trackSpeed.update(0, 0, 0);
			} else {
				GLfloat xs = ((x - trackLast->x) + trackSpeed.x * trackSpeed.time) / (elapsed + trackSpeed.time);
				GLfloat ys = ((y - trackLast->y) + trackSpeed.y * trackSpeed.time) / (elapsed + trackSpeed.time);
				trackSpeed.update(xs, ys, elapsed);
			}
			_trackLast = cur;
			trackLast = &_trackLast;
		}
	}
	
	bool processed = false;
	int32_t count = trackList.count();
	GLTouchObject** items = trackList.items();
	for (int i = 0; i < count; i++) {
		GLTouchObject* track = items[i];
		int32_t idx = trackingList.search(track);
		CString name;
		if (track->name.m_length > 0) {
			name = L" ";
			name += track->name;
		}
		
		if ((!stop) &&
			(x >= track->x) && (x < (track->x + track->w)) &&
			(y >= track->y) && (y < (track->y + track->h))
			) {
			if (idx < 0) {
				if (!processed) {
					// Track In (Enter)
					trackingList.push(track);
					trackingLast.push(cur);
					processed |= track->onevent(CString::format(L"enter%ls", (wchar_t*)name), &cur, NULL, &trackSpeed, track->userData);
				}
			} else {
				if (!processed) {
					// Track Move
					GLTouchPoint last = trackingLast[idx];
					GLTouchPoint delta = GLTouchPoint( (x - last.x), (y - last.y), (time - last.time) );
					processed |= track->onevent(CString::format(L"move%ls", (wchar_t*)name), &cur, &delta, &trackSpeed, track->userData);
					trackingLast[idx] = cur;
				}
			}
		} else if (idx >= 0) {
			// Track Out (Leave)
			GLTouchPoint last = trackingLast[idx];
			GLTouchPoint delta = GLTouchPoint( (x - last.x), (y - last.y), (time - last.time) );
			track->onevent(CString::format(L"leave%ls", (wchar_t*)name), &cur, &delta, &trackSpeed, track->userData);
			trackingList.remove(idx);
			trackingLast.remove(idx);
		}
	}
	
	if (stop) {
		trackSpeed.update(0, 0, 0);
		trackLast = NULL;
	}
}

void GLTouchScreen::record(float x, float y) {
	x = roundf(x * width / context->frame->width);
	y = roundf(y * height / context->frame->height);
	
	if (context->sceneRatio > context->ratio) { // Экран уже чем треубется
		x = roundf(x * context->width / width);
		y = roundf((((y / height) - 0.5) * context->sceneRatio / context->ratio + 0.5) * context->height);
	} else {
		x = roundf((((x / width) - 0.5) * context->ratio / context->sceneRatio + 0.5) * context->width);
		y = roundf(y * context->height / height);
	}

	recordTrack(x, y, false);
	
	GLfloat dist = NAN;
	if (touchLast != NULL) {
		GLfloat dx = touchLast->x - x;
		GLfloat dy = touchLast->y - y;
		dist = floorf(sqrtf(dx * dx + dy * dy));
	}
	
	if ((isnan(dist)) || (dist > recordDistance)) {
		_touchLast.x = x;
		_touchLast.y = y;
		touchLast = &_touchLast;
		touchList.push(_touchLast);
	}
}

