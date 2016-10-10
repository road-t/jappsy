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

#import "ExampleViewController.h"
#import "AppDelegate.h"

@interface ExampleViewController ()

@end

@implementation ExampleViewController

- (void)viewDidLoad {
    [super viewDidLoad];
	
	if (omView == NULL) {
		omView = [[OMView alloc] init];
		self.view = omView;
		[omView initConstraints:self];
	} else {
		self.view = omView;
	}
	[omView updateState:OMVIEW_SHOW];
	
	// Disable Animation Rotation and Keyboard
	//[UIView setAnimationsEnabled:NO];
	
	// Keep Screen On
	[[UIApplication sharedApplication] setIdleTimerDisabled:YES];
	
	// Black Background With White Status Bar
	[self setNeedsStatusBarAppearanceUpdate];
}

- (void)awakeFromNib {
	[super awakeFromNib];
	
	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationChanged:) name:UIDeviceOrientationDidChangeNotification object:nil];
}

- (void)orientationChanged:(NSNotification *)notification {
	[omView orientationChanged:notification];
}

- (UIStatusBarStyle)preferredStatusBarStyle {
	return UIStatusBarStyleLightContent;
}

@end
