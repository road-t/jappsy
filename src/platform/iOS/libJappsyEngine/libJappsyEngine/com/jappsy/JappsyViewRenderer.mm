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

#import "JappsyViewRenderer.h"

@interface JappsyViewRenderer ()
{
	EAGLContext* _context;
	
	GLuint _colorRenderbuffer;
	GLuint _depthRenderbuffer;
	
	GLuint _defaultFBOName;
	
	GLuint _viewWidth;
	GLuint _viewHeight;
}
@end

@implementation JappsyViewRenderer

- (instancetype) initWithContext:(EAGLContext*)context AndDrawable:(id<EAGLDrawable>)drawable
{
	// Create default framebuffer object. The backing will be allocated for the
	// current layer in -resizeFromLayer
	glGenFramebuffers(1, &_defaultFBOName);
	
	glGenRenderbuffers(1, &_colorRenderbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _defaultFBOName);
	glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
	_context = context;
	
	// This call associates the storage for the current render buffer with the
	// EAGLDrawable (our CAEAGLLayer) allowing us to draw into a buffer that
	// will later be rendered to the screen wherever the layer is (which
	// corresponds with our view).
	[_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:drawable];
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderbuffer);
	
	// Get the drawable buffer's width and height so we can create a depth buffer for the FBO
	GLint backingWidth;
	GLint backingHeight;
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
	
	// Create a depth buffer to use with our drawable FBO
	glGenRenderbuffers(1, &_depthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)	{
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		return nil;
	}
	
	NSLog(@"%s %s", glGetString(GL_RENDERER), glGetString(GL_VERSION));
	
	_viewWidth = 100;
	_viewHeight = 100;
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	[self render];
	GetGLError();
	
	if (nil == self)
	{
		glDeleteFramebuffers(1, &_defaultFBOName);
		glDeleteRenderbuffers(1, &_colorRenderbuffer);
		glDeleteRenderbuffers(1, &_depthRenderbuffer);
	}
	
	return self;
}

- (BOOL)resizeFromLayer:(CAEAGLLayer*)layer
{
	// The pixel dimensions of the CAEAGLLayer
	GLint width;
	GLint height;
	
	// Allocate color buffer backing based on the current layer size
	glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
	[_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
	
	glGenRenderbuffers(1, &_depthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
	
	glViewport(0, 0, width, height);
	
	_viewWidth = width;
	_viewHeight = height;
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		return NO;
	}
	
	return YES;
}

static int color = 0;

- (void) render
{
	glBindFramebuffer(GL_FRAMEBUFFER, _defaultFBOName);
	
	float c = (float)color / 255.0f;
	glClearColor(c, c, c, 1.0f);
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	color++;
	if (color >= 256) color = 0;
	
	glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
	[_context presentRenderbuffer:GL_RENDERBUFFER];
}

- (void) dealloc
{
	if (_defaultFBOName)
	{
		glDeleteFramebuffers(1, &_defaultFBOName);
		_defaultFBOName = 0;
	}
	
	if (_colorRenderbuffer)
	{
		glDeleteRenderbuffers(1, &_colorRenderbuffer);
		_colorRenderbuffer = 0;
	}
}

@end