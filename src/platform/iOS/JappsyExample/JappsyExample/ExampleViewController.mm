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
#import <libJappsyEngine/JappsyView.h>

@interface ExampleViewController ()

@property(strong,nonatomic)NSMutableArray *lv; // Menu Visible
@property(strong,nonatomic)NSMutableArray *lh; // Menu Hidden

@property(strong,nonatomic)UIView* layoutView01;
@property(strong,nonatomic)UIView* layoutView02;
@property(strong,nonatomic)UIView* layoutView03;
@property(strong,nonatomic)UIButton* hiddenView;

@end

@implementation ExampleViewController

@synthesize lv;
@synthesize lh;

@synthesize layoutView01;
@synthesize layoutView02;
@synthesize layoutView03;
@synthesize hiddenView;

- (void)viewDidLoad {
    [super viewDidLoad];

	// Background Color
	
	self.view.backgroundColor = [UIColor blackColor];
	
	// Menu
	
	UIView* menuView = [[UIView alloc] init];
	[menuView setOpaque:NO];
	menuView.backgroundColor = [UIColor clearColor];
	[menuView setTranslatesAutoresizingMaskIntoConstraints:NO];
	[self.view addSubview: menuView];

	lv = [[NSMutableArray alloc] init];
	[lv addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
	
	lh = [[NSMutableArray alloc] init];
	[lh addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
	
	[self.view addConstraints:lv];
	
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self.topLayoutGuide attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:200.0],999)];
	
	// Menu Item 01
	
	UIButton* menuItem01 = [UIButton buttonWithType:UIButtonTypeCustom];
	[menuItem01 addTarget:self action:@selector(menuSelect01:) forControlEvents:UIControlEventTouchUpInside];
	[menuItem01 setOpaque:NO];
	menuItem01.backgroundColor = [UIColor blueColor];
	[menuItem01 setTranslatesAutoresizingMaskIntoConstraints:NO];
	[menuItem01 setTitle:@"Example01" forState:UIControlStateNormal];
	[menuView addSubview: menuItem01];
		
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuItem01 attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeTop multiplier:1.0 constant:8.0],999)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuItem01 attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeLeft multiplier:1.0 constant:8.0],999)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuItem01 attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeRight multiplier:1.0 constant:-8.0],999)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuItem01 attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:50.0],999)];
	
	// Menu Item 02
	
	UIButton* menuItem02 = [UIButton buttonWithType:UIButtonTypeCustom];
	[menuItem02 addTarget:self action:@selector(menuSelect02:) forControlEvents:UIControlEventTouchUpInside];
	[menuItem02 setOpaque:NO];
	menuItem02.backgroundColor = [UIColor grayColor];
	[menuItem02 setTranslatesAutoresizingMaskIntoConstraints:NO];
	[menuItem02 setTitle:@"Example02" forState:UIControlStateNormal];
	[menuView addSubview: menuItem02];
	
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuItem02 attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:menuItem01 attribute:NSLayoutAttributeBottom multiplier:1.0 constant:8.0],998)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuItem02 attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeLeft multiplier:1.0 constant:8.0],999)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuItem02 attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeRight multiplier:1.0 constant:-8.0],999)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuItem02 attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:50.0],999)];
	
	// Menu Item 03
	
	UIButton* menuItem03 = [UIButton buttonWithType:UIButtonTypeCustom];
	[menuItem03 addTarget:self action:@selector(menuSelect03:) forControlEvents:UIControlEventTouchUpInside];
	[menuItem03 setOpaque:NO];
	menuItem03.backgroundColor = [UIColor blueColor];
	[menuItem03 setTranslatesAutoresizingMaskIntoConstraints:NO];
	[menuItem03 setTitle:@"Game" forState:UIControlStateNormal];
	[menuView addSubview: menuItem03];
	
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuItem03 attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:menuItem02 attribute:NSLayoutAttributeBottom multiplier:1.0 constant:8.0],997)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuItem03 attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeLeft multiplier:1.0 constant:8.0],999)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuItem03 attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeRight multiplier:1.0 constant:-8.0],999)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuItem03 attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:50.0],999)];
	
	// Layout 01
	
	layoutView01 = [[UIView alloc] init];
	[layoutView01 setOpaque:NO];
	layoutView01.backgroundColor = [UIColor redColor];
	[layoutView01 setTranslatesAutoresizingMaskIntoConstraints:NO];
	[self.view addSubview: layoutView01];
	
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:layoutView01 attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],998)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:layoutView01 attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],998)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:layoutView01 attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],998)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:layoutView01 attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0],998)];
	
	{
		UIButton* menuButton = [UIButton buttonWithType:UIButtonTypeCustom];
		[menuButton addTarget:self action:@selector(showMenu:) forControlEvents:UIControlEventTouchUpInside];
		[menuButton setOpaque:NO];
		menuButton.backgroundColor = [UIColor blackColor];
		[menuButton setTranslatesAutoresizingMaskIntoConstraints:NO];
		[menuButton setTitle:@"Menu" forState:UIControlStateNormal];
		[layoutView01 addSubview: menuButton];
		
		[layoutView01 addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuButton attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:layoutView01 attribute:NSLayoutAttributeTop multiplier:1.0 constant:8.0],999)];
		[layoutView01 addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuButton attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:layoutView01 attribute:NSLayoutAttributeLeft multiplier:1.0 constant:8.0],999)];
		[layoutView01 addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuButton attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:50.0],999)];
		[layoutView01 addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuButton attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:50.0],999)];
	}

	// Layout 02
	
	layoutView02 = [[UIView alloc] init];
	[layoutView02 setOpaque:NO];
	layoutView02.backgroundColor = [UIColor cyanColor];
	[layoutView02 setTranslatesAutoresizingMaskIntoConstraints:NO];
	[self.view addSubview: layoutView02];
	
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:layoutView02 attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],998)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:layoutView02 attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],998)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:layoutView02 attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],998)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:layoutView02 attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0],998)];
	
	{
		UIButton* menuButton = [UIButton buttonWithType:UIButtonTypeCustom];
		[menuButton addTarget:self action:@selector(showMenu:) forControlEvents:UIControlEventTouchUpInside];
		[menuButton setOpaque:NO];
		menuButton.backgroundColor = [UIColor blackColor];
		[menuButton setTranslatesAutoresizingMaskIntoConstraints:NO];
		[menuButton setTitle:@"Menu" forState:UIControlStateNormal];
		[layoutView02 addSubview: menuButton];
		
		[layoutView02 addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuButton attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:layoutView02 attribute:NSLayoutAttributeTop multiplier:1.0 constant:8.0],999)];
		[layoutView02 addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuButton attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:layoutView02 attribute:NSLayoutAttributeLeft multiplier:1.0 constant:8.0],999)];
		[layoutView02 addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuButton attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:50.0],999)];
		[layoutView02 addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:menuButton attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:50.0],999)];
	}

	// Layout 03
	
	layoutView03 = [[UIView alloc] init];
	[layoutView03 setOpaque:NO];
	layoutView03.backgroundColor = [UIColor greenColor];
	[layoutView03 setTranslatesAutoresizingMaskIntoConstraints:NO];
	[self.view addSubview: layoutView03];
	
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:layoutView03 attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],998)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:layoutView03 attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],998)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:layoutView03 attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],998)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:layoutView03 attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0],998)];

	// Layout Hidden
	
	hiddenView = [UIButton buttonWithType:UIButtonTypeCustom];
	[hiddenView addTarget:self action:@selector(showLayout:) forControlEvents:UIControlEventTouchUpInside];
	[hiddenView setOpaque:NO];
	hiddenView.backgroundColor = [UIColor blackColor];
	hiddenView.alpha = 0.5;
	[hiddenView setTranslatesAutoresizingMaskIntoConstraints:NO];
	[self.view addSubview: hiddenView];
	
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:hiddenView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],998)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:hiddenView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],998)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:hiddenView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:menuView attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],998)];
	[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:hiddenView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0],998)];
	
	[layoutView01 setHidden:NO];
	[layoutView02 setHidden:YES];
	[layoutView03 setHidden:YES];
	
	// Media Controls
	
#ifdef DEMO
	{
		UIButton* volumeDownButton = [UIButton buttonWithType:UIButtonTypeCustom];
		[volumeDownButton addTarget:self action:@selector(volumeDown:) forControlEvents:UIControlEventTouchUpInside];
		[volumeDownButton setOpaque:NO];
		volumeDownButton.backgroundColor = [UIColor darkGrayColor];
		[volumeDownButton setTranslatesAutoresizingMaskIntoConstraints:NO];
		[volumeDownButton setTitle:@"-" forState:UIControlStateNormal];
		[self.view addSubview: volumeDownButton];
		
		[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:volumeDownButton attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeLeft multiplier:1.0 constant:8.0],999)];
		[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:volumeDownButton attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterY multiplier:1.0 constant:0.0],999)];
		[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:volumeDownButton attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:50.0],999)];
		[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:volumeDownButton attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:50.0],999)];

		UIButton* volumeUpButton = [UIButton buttonWithType:UIButtonTypeCustom];
		[volumeUpButton addTarget:self action:@selector(volumeUp:) forControlEvents:UIControlEventTouchUpInside];
		[volumeUpButton setOpaque:NO];
		volumeUpButton.backgroundColor = [UIColor darkGrayColor];
		[volumeUpButton setTranslatesAutoresizingMaskIntoConstraints:NO];
		[volumeUpButton setTitle:@"+" forState:UIControlStateNormal];
		[self.view addSubview: volumeUpButton];
		
		[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:volumeUpButton attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:volumeDownButton attribute:NSLayoutAttributeRight multiplier:1.0 constant:8.0],998)];
		[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:volumeUpButton attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterY multiplier:1.0 constant:0.0],999)];
		[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:volumeUpButton attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:50.0],999)];
		[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:volumeUpButton attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:50.0],999)];

		UIButton* mixerCheckButton = [UIButton buttonWithType:UIButtonTypeCustom];
		[mixerCheckButton addTarget:self action:@selector(mixerCheck:) forControlEvents:UIControlEventTouchUpInside];
		[mixerCheckButton setOpaque:NO];
		mixerCheckButton.backgroundColor = [UIColor darkGrayColor];
		[mixerCheckButton setTranslatesAutoresizingMaskIntoConstraints:NO];
		[mixerCheckButton setTitle:@"*" forState:UIControlStateNormal];
		[self.view addSubview: mixerCheckButton];
		
		[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:mixerCheckButton attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:volumeUpButton attribute:NSLayoutAttributeRight multiplier:1.0 constant:8.0],997)];
		[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:mixerCheckButton attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterY multiplier:1.0 constant:0.0],999)];
		[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:mixerCheckButton attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:50.0],999)];
		[self.view addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:mixerCheckButton attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:50.0],999)];
	}
#endif
	
	// Disable Animation Rotation and Keyboard
	//[UIView setAnimationsEnabled:NO];
	
	// Keep Screen On
	[[UIApplication sharedApplication] setIdleTimerDisabled:YES];
	
	// Black Background With White Status Bar
	[self setNeedsStatusBarAppearanceUpdate];
}

float volume = 1.0f;

- (void) volumeDown:(UIButton*)sender {
	if (omView != NULL) {
		volume -= 0.1f;
		if (volume < 0.0f) volume = 0.0f;
		[omView mixerVolume:volume];
	}
}

- (void) volumeUp:(UIButton*)sender {
	if (omView != NULL) {
		volume += 0.1f;
		if (volume > 1.0f) volume = 1.0f;
		[omView mixerVolume:volume];
	}
}

- (void) mixerCheck:(UIButton*)sender {
	if (omView != NULL) {
		[omView isMixerPlaying];
	}
}

- (void) showLayout:(UIButton*)sender {
	[self.view layoutIfNeeded];
	
	[self.view removeConstraints:lv];
	[self.view addConstraints:lh];

	[UIView animateWithDuration:0.3 animations:^{
		hiddenView.alpha = 0.0f;
		[self.view layoutIfNeeded];
	} completion: ^(BOOL finished) {
		[hiddenView setHidden:YES];
	}];
	
	if (![layoutView03 isHidden]) {
		[omView updateState:OMVIEW_SHOW];
	}
}

- (void) showMenu:(UIButton*)sender {
	[self.view layoutIfNeeded];

	[self.view removeConstraints:lh];
	[self.view addConstraints:lv];
	[hiddenView setHidden:NO];

	[UIView animateWithDuration:0.3 animations:^{
		hiddenView.alpha = 0.5f;
		[self.view layoutIfNeeded];
	}];
}

- (void) menuSelect01:(UIButton*)sender {
	[layoutView01 setHidden:NO];
	[layoutView02 setHidden:YES];
	[layoutView03 setHidden:YES];
	[self showLayout:sender];
}

- (void) menuSelect02:(UIButton*)sender {
	[layoutView01 setHidden:YES];
	[layoutView02 setHidden:NO];
	[layoutView03 setHidden:YES];
	[self showLayout:sender];
}

void onCloseCallback(void* userData) {
	ExampleViewController* controller = (__bridge ExampleViewController*)userData;
	[controller showMenu:nil];
}

- (void) menuSelect03:(UIButton*)sender {
	if (omView == NULL) {
		NSString* basePath = @"https://dev03-om.jappsy.com/jappsy/"; // Demo server
		//NSString* basePath = @"https://om.jappsy.com/jappsy/"; // Production server
		NSString* token = @"e994a237491a85ff72b9f737bbf47047cfbc6dbb0897ea1eea5e75338a4b13c3";
		NSString* sessid = @"8ea5f70b15263872760d7e14ce8e579a";
		NSString* devid = @"";
		NSString* locale = @"RU";
		
		omView = [[OMView alloc] init:basePath token:token sessid:sessid devid:devid locale:locale onclose:onCloseCallback userData:(__bridge void*)self];
		[layoutView03 addSubview:omView];

		[layoutView03 addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:layoutView03 attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],999)];
		[layoutView03 addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:layoutView03 attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
		[layoutView03 addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:layoutView03 attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
		[layoutView03 addConstraint:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:layoutView03 attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];
		
		[omView onStart];
		[omView updateState:OMVIEW_RUN];
	}

	[layoutView01 setHidden:YES];
	[layoutView02 setHidden:YES];
	[layoutView03 setHidden:NO];
	[self showLayout:sender];
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
