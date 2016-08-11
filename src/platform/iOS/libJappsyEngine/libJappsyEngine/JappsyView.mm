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

#import "JappsyView.h"
#import "GLContext.h"
#include <event/uMotionEvent.h>
#include <core/uSystem.h>
#include <opengl/uGLEngine.h>

@interface JappsyView () {
	GLContext* _renderer;
    EAGLContext* _context;
    NSInteger _interval;
    CADisplayLink* _displayLink;
}

@property(nonatomic,retain)NSMutableArray* activeTouches;

@end

@implementation JappsyView

- (void) engine:(void*)engine {
	_renderer->initialize((GLEngine*)engine);
}

+ (Class) layerClass {
    return [CAEAGLLayer class];
}

- (instancetype) initWithCoder:(NSCoder*)coder {
    if ((self = [super initWithCoder:coder])) {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer*)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		
		if (!_context || ![EAGLContext setCurrentContext:_context])
			return nil;
		
		try {
			_renderer = new GLContext(_context, eaglLayer);
			if (_renderer == NULL)
				return nil;
		} catch (const char* e) {
			return nil;
		}
		
		_running = FALSE;
		_interval = 1;
		_displayLink = nil;
		
		// Touch Init
		[self setMultipleTouchEnabled:YES];
		[self setExclusiveTouch:YES];
		self.activeTouches = [[NSMutableArray alloc] init];
    }
	
	return self;
}

- (void) drawView:(id)sender
{
	[EAGLContext setCurrentContext:_context];
	_renderer->render();
}

- (void) layoutSubviews
{
	_renderer->update((CAEAGLLayer*)self.layer);
	[self drawView:nil];
}

- (void) setAnimationFrameInterval:(NSInteger)frameInterval
{
	if (frameInterval >= 1) {
		_interval = frameInterval;
		
		if (_running) {
			[self onPause];
			[self onResume];
		}
	}
}

- (void) onResume
{
	if (!_running) {
		_displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
		[_displayLink setFrameInterval:_interval];
		[_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		_running = TRUE;
	}
}

- (void) onPause
{
	if (_running) {
		[_displayLink invalidate];
		_displayLink = nil;
		_running = FALSE;
	}
}

- (void) dealloc
{
	if ([EAGLContext currentContext] == _context)
		[EAGLContext setCurrentContext:nil];
}

static MotionEvent motionEvent;
static int32_t pointerId = 1;

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	for (UITouch *touch in touches) {
		MotionPointer* pointer = NULL;
		
		if (![_activeTouches containsObject:touch]) {
			[_activeTouches addObject:touch];
			motionEvent.multitouch = (pointerId != 1);
			pointer = motionEvent.insert(pointerId++);
		} else {
			int32_t index = (int32_t)[_activeTouches indexOfObject:touch];
			pointer = motionEvent.getPointer(index);
		}
		
		if (pointer != NULL) {
			CGPoint pos = [touch locationInView:self];
			pointer->x = pos.x;
			pointer->y = pos.y;
			pointer->event = MotionEvent::ACTION_DOWN;
			pointer->time = currentTimeMillis();
		}
	}
	
	motionEvent.actionEvent = MotionEvent::ACTION_DOWN;
	
	for (UITouch *touch in touches) {
		NSUInteger index = [_activeTouches indexOfObject:touch];
		if (index != NSNotFound) {
			motionEvent.actionIndex = (int32_t)index;
			_renderer->touch(&motionEvent);
		}
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	for (UITouch *touch in touches) {
		NSUInteger index = [_activeTouches indexOfObject:touch];
		if (index != NSNotFound) {
			MotionPointer* pointer = motionEvent.getPointer(index);
			CGPoint pos = [touch locationInView:self];
			pointer->x = pos.x;
			pointer->y = pos.y;
			pointer->event = MotionEvent::ACTION_MOVE;
			pointer->time = currentTimeMillis();
		}
	}
	
	motionEvent.actionEvent = MotionEvent::ACTION_MOVE;
	
	for (UITouch *touch in touches) {
		NSUInteger index = [_activeTouches indexOfObject:touch];
		if (index != NSNotFound) {
			motionEvent.actionIndex = (int32_t)index;
			_renderer->touch(&motionEvent);
		}
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	for (UITouch *touch in touches) {
		NSUInteger index = [_activeTouches indexOfObject:touch];
		if (index != NSNotFound) {
			MotionPointer* pointer = motionEvent.getPointer(index);
			CGPoint pos = [touch locationInView:self];
			pointer->x = pos.x;
			pointer->y = pos.y;
			pointer->event = MotionEvent::ACTION_UP;
			pointer->time = currentTimeMillis();
		}
	}
	
	motionEvent.actionEvent = MotionEvent::ACTION_UP;
	
	for (UITouch *touch in touches) {
		NSUInteger index = [_activeTouches indexOfObject:touch];
		if (index != NSNotFound) {
			motionEvent.actionIndex = (int32_t)index;
			_renderer->touch(&motionEvent);
		}
	}
	
	for (UITouch *touch in touches) {
		NSUInteger index = [_activeTouches indexOfObject:touch];
		if (index != NSNotFound) {
			motionEvent.remove(index);
			[_activeTouches removeObjectAtIndex:index];
		}
	}
	
	if (motionEvent.getPointerCount() == 0) {
		motionEvent.multitouch = false;
		pointerId = 1;
	}
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	[self touchesEnded:touches withEvent:event];
}

@end