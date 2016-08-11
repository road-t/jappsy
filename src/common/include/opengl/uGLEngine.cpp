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

#include "uGLEngine.h"

void GLEngine::onRender() {
	this->context->frame->loop();
}

void GLEngine::onUpdate(int width, int height) {
	this->context->frame->width = width;
	this->context->frame->height = height;
}

void GLEngine::onTouch(MotionEvent* event) {
	switch (event->actionEvent) {
		case MotionEvent::ACTION_DOWN:
			this->context->touchScreen->onTouchStart(event);
			break;
			
		case MotionEvent::ACTION_MOVE:
			this->context->touchScreen->onTouchMove(event);
			break;
			
		case MotionEvent::ACTION_UP:
			this->context->touchScreen->onTouchEnd(event);
			break;
		
		default:;
	}
}
