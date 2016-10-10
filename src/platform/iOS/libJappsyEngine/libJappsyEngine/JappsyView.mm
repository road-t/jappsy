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
#include <core/uMemory.h>
#include <jappsy.h>
#include <platform.h>

#ifdef __IOS__
	#include <openal/uMixer.h>
#endif


@interface JappsyView () {
	GLContext* _renderer;
    EAGLContext* _context;
    NSInteger _interval;
    CADisplayLink* _displayLink;
	GLfloat viewScale;
}

@property(nonatomic,retain)NSMutableArray* activeTouches;

@end

@implementation JappsyView

- (void) engine:(void*)refEngine {
	if (_renderer != NULL) {
		_renderer->initialize((GLEngine*)refEngine);
	}
}

+ (Class) layerClass {
    return [CAEAGLLayer class];
}

- (instancetype) init {
	if ((self = [super init])) {
		CAEAGLLayer *eaglLayer = (CAEAGLLayer*)self.layer;
		
		eaglLayer.opaque = TRUE;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
										[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
										kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		
		_renderer = NULL;
		_running = FALSE;
		_stopping = FALSE;
		_interval = 1;
		_displayLink = nil;
		
		// Touch Init
		[self setMultipleTouchEnabled:YES];
		[self setExclusiveTouch:YES];
		self.activeTouches = [[NSMutableArray alloc] init];
	}
	
	return self;
}

- (instancetype) initWithFrame:(CGRect)frame {
	if ((self = [super initWithFrame:frame])) {
		CAEAGLLayer *eaglLayer = (CAEAGLLayer*)self.layer;
		
		eaglLayer.opaque = TRUE;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
										[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
										kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		
		_renderer = NULL;
		_running = FALSE;
		_stopping = FALSE;
		_interval = 1;
		_displayLink = nil;
		
		// Touch Init
		[self setMultipleTouchEnabled:YES];
		[self setExclusiveTouch:YES];
		self.activeTouches = [[NSMutableArray alloc] init];
	}
	
	return self;
}

- (instancetype) initWithCoder:(NSCoder*)coder {
    if ((self = [super initWithCoder:coder])) {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer*)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		
		_renderer = NULL;
		_running = FALSE;
		_stopping = FALSE;
		_interval = 1;
		_displayLink = nil;
		
		// Touch Init
		[self setMultipleTouchEnabled:YES];
		[self setExclusiveTouch:YES];
		self.activeTouches = [[NSMutableArray alloc] init];
    }
	
	return self;
}

- (void) didMoveToWindow
{
#if !defined(__SIMULATOR__)
	// AutoScale Pixels
	GLfloat screenScale = [[UIScreen mainScreen] scale];
	/*GLfloat dpi = screenScale * 160.0f;*/
	CGRect screen = [[UIScreen mainScreen] bounds];
	GLfloat width = screen.size.width * screenScale;
	GLfloat height = screen.size.height * screenScale;
	GLfloat max = (width > height) ? width : height;
	viewScale = 1.0;

	if (max > 1920) {
		viewScale = self.window.screen.nativeScale; // enable pixel scale for big screens
	} else {
		viewScale = screenScale; // disable pixel scale for small screens
	}
#else
	viewScale = 1.0;
#endif
	/*GLfloat diag = (float)(round(sqrt(width * width + height * height) * 2.0f / dpi) / 2.0f);*/
	
	self.contentScaleFactor = viewScale;
}

- (void) drawView:(id)sender
{
	if (_renderer != NULL) {
		if(![self isHidden]) {
			[EAGLContext setCurrentContext:_context];
			_renderer->render();
		}
	}
}

- (void) layoutSubviews
{
	if (_renderer != NULL) {
		_renderer->update((CAEAGLLayer*)self.layer);
		[self drawView:nil];
	}
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

- (BOOL) onStart
{
	if (!_stopping) {
		if (_renderer == NULL) {
			if (_context && [EAGLContext setCurrentContext:_context]) {
				try {
					CAEAGLLayer *eaglLayer = (CAEAGLLayer*)self.layer;
					_renderer = memNew(_renderer, GLContext(_context, eaglLayer, self.contentScaleFactor));
				} catch (const char* e) {
				}
			}
		}
		
		if (_renderer != NULL) {
			//[self onResume:true];
			return YES;
		}
	}
	
	return NO;
}

- (BOOL) onStop
{
	if (!_stopping) {
		_stopping = YES;
		
		if (_renderer != NULL) {
			[NSThread detachNewThreadSelector:@selector(onShutdownRequest) toTarget:self withObject:nil];
		
			return YES;
		} else {
#ifdef DEBUG
			memLogStats(NULL, NULL, NULL, NULL);
#endif

			[self onPause];
		}
	}
	
	return NO;
}

- (void) onShutdownRequest
{
	_renderer->engine->shutdown();
		
	[self performSelectorOnMainThread:@selector(onShutdown) withObject:self waitUntilDone:NO];
}

- (void) onShutdown
{
	[self onPause];
	
	if (_renderer != NULL) {
		memDelete(_renderer);
		_renderer = NULL;
	}
		
	if ([EAGLContext currentContext] == _context)
		[EAGLContext setCurrentContext:nil];
	
	jappsyQuit();

	_stopping = NO;
}

- (void) onResume
{
	if (!_stopping) {
		if (!_running) {
#ifdef __IOS__
			resumeAudioPlayer();
#endif
			_displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
			if ([[UIDevice currentDevice].systemVersion floatValue] >= 10.0f) {
				[_displayLink setPreferredFramesPerSecond:60];
			} else {
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 100000
				[_displayLink setFrameInterval:_interval];
#endif
			}
			[_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
			_running = TRUE;
		}
	}
}

- (void) onPause
{
	if (!_stopping) {
		if (_running) {
			[_displayLink invalidate];
			_displayLink = nil;
			_running = FALSE;
#ifdef __IOS__
			pauseAudioPlayer();
#endif
		}
	}
}

- (void) dealloc
{
	[self onStop];
}

static MotionEvent motionEvent;
static int32_t pointerId = 1;

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	if (_renderer == NULL)
		return;
	
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
			pointer->x = pos.x * viewScale;
			pointer->y = pos.y * viewScale;
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
	if (_renderer == NULL)
		return;
	
	for (UITouch *touch in touches) {
		NSUInteger index = [_activeTouches indexOfObject:touch];
		if (index != NSNotFound) {
			MotionPointer* pointer = motionEvent.getPointer(index);
			CGPoint pos = [touch locationInView:self];
			pointer->x = pos.x * viewScale;
			pointer->y = pos.y * viewScale;
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
	if (_renderer == NULL)
		return;
	
	for (UITouch *touch in touches) {
		NSUInteger index = [_activeTouches indexOfObject:touch];
		if (index != NSNotFound) {
			MotionPointer* pointer = motionEvent.getPointer(index);
			CGPoint pos = [touch locationInView:self];
			pointer->x = pos.x * viewScale;
			pointer->y = pos.y * viewScale;
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
