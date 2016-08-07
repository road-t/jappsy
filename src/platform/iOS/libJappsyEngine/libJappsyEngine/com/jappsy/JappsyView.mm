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

//#import <QuartzCore/QuartzCore.h>

//#import <OpenGLES/EAGL.h>
//#import <OpenGLES/EAGLDrawable.h>

#import "JappsyRenderer.h"

@interface JappsyView () {
	GLContext* _renderer;
    EAGLContext* _context;
    NSInteger _interval;
    CADisplayLink* _displayLink;
}
@end

@implementation JappsyView

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

@end
