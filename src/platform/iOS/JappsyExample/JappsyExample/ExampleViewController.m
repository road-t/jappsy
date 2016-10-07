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
#import "ExampleJappsyView.h"
#import "AppDelegate.h"

@interface ExampleViewController ()

@end

NSArray* lastConstraintList = NULL;
NSMutableArray* potrtaitConstraintList = NULL;
NSMutableArray* landscapeConstraintList = NULL;

NSLayoutConstraint* ConstraintPriotiry(NSLayoutConstraint* constraint, UILayoutPriority priority) {
	constraint.priority = priority;
	return constraint;
}

@implementation ExampleViewController

- (void)viewDidLoad {
    [super viewDidLoad];
	
	// Root View
	UIView *contentView = [[UIView alloc] init];
	contentView.backgroundColor = [UIColor blackColor];
	[contentView setTranslatesAutoresizingMaskIntoConstraints:NO];
	self.view = contentView;
	
	// Top View
	UIView *topView = [[UIView alloc] init];
	topView.backgroundColor = [UIColor redColor];
	[topView setTranslatesAutoresizingMaskIntoConstraints:NO];
	[self.view addSubview:topView];
	
	// Game View
	if (omView == NULL) {
		omView = [[OMView alloc] initWithFrame:CGRectMake(0,0,100,100)];
		/*
		if ([[UIDevice currentDevice].systemVersion floatValue] >= 9.0f) {
			omView = [[OMView alloc] initWithFrame:[UIScreen mainScreen].bounds];
		} else {
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 70000
			omView = [[OMView alloc] initWithFrame:[UIScreen mainScreen].applicationFrame];
#endif
		}
		 */
		
		[omView setTranslatesAutoresizingMaskIntoConstraints:NO];
		omView.backgroundColor = [UIColor blackColor];
	}
	[self.view addSubview:omView];
/*
	[self.view addConstraint:[NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0]];
	[self.view addConstraint:[NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeHeight multiplier:1.0 constant:0.0]];
	[self.view addConstraint:[NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterX multiplier:1.0 constant:0.0]];
	[self.view addConstraint:[NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterY multiplier:1.0 constant:0.0]];
*/
	
	// Bottom View
	UIView *bottomView = [[UIView alloc] init];
	bottomView.backgroundColor = [UIColor blueColor];
	[bottomView setTranslatesAutoresizingMaskIntoConstraints:NO];
	[self.view addSubview:bottomView];
	
	// Constraints
	potrtaitConstraintList = [[NSMutableArray alloc] init];
	[potrtaitConstraintList addObject:[NSLayoutConstraint constraintWithItem:topView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self.topLayoutGuide attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0]];
	[potrtaitConstraintList addObject:[NSLayoutConstraint constraintWithItem:topView attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:100.0]];
	[potrtaitConstraintList addObject:[NSLayoutConstraint constraintWithItem:topView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0]];
	[potrtaitConstraintList addObject:[NSLayoutConstraint constraintWithItem:topView attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterX multiplier:1.0 constant:0.0]];

	[potrtaitConstraintList addObject:[NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:topView attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0]];
	[potrtaitConstraintList addObject:[NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0]];
	[potrtaitConstraintList addObject:[NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterX multiplier:1.0 constant:0.0]];
	[potrtaitConstraintList addObject:[NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:omView attribute:NSLayoutAttributeWidth multiplier:(1080.0 / 1920.0) constant:0.0]];

	[potrtaitConstraintList addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:bottomView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:omView attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0], 999)];
	[potrtaitConstraintList addObject:[NSLayoutConstraint constraintWithItem:bottomView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0]];
	[potrtaitConstraintList addObject:[NSLayoutConstraint constraintWithItem:bottomView attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterX multiplier:1.0 constant:0.0]];
	[potrtaitConstraintList addObject:[NSLayoutConstraint constraintWithItem:bottomView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0]];
	
	landscapeConstraintList = [[NSMutableArray alloc] init];
	[landscapeConstraintList addObject:[NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0]];
	[landscapeConstraintList addObject:[NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeHeight multiplier:1.0 constant:0.0]];
	[landscapeConstraintList addObject:[NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterX multiplier:1.0 constant:0.0]];
	[landscapeConstraintList addObject:[NSLayoutConstraint constraintWithItem:omView attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterY multiplier:1.0 constant:0.0]];

	[landscapeConstraintList addObject:[NSLayoutConstraint constraintWithItem:topView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:omView attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0]];
	[landscapeConstraintList addObject:[NSLayoutConstraint constraintWithItem:topView attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:100.0]];
	[landscapeConstraintList addObject:[NSLayoutConstraint constraintWithItem:topView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0]];
	[landscapeConstraintList addObject:[NSLayoutConstraint constraintWithItem:topView attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterX multiplier:1.0 constant:0.0]];

	[landscapeConstraintList addObject:[NSLayoutConstraint constraintWithItem:bottomView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:omView attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0]];
	[landscapeConstraintList addObject:[NSLayoutConstraint constraintWithItem:bottomView attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:300.0]];
	[landscapeConstraintList addObject:[NSLayoutConstraint constraintWithItem:bottomView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0]];
	[landscapeConstraintList addObject:[NSLayoutConstraint constraintWithItem:bottomView attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self.view attribute:NSLayoutAttributeCenterX multiplier:1.0 constant:0.0]];
	
	UIInterfaceOrientation interfaceOrientation = [[UIApplication sharedApplication] statusBarOrientation];
	if (UIInterfaceOrientationIsPortrait(interfaceOrientation)) {
		lastConstraintList = potrtaitConstraintList;
	} else {
		lastConstraintList = landscapeConstraintList;
	}
	[self.view addConstraints:lastConstraintList];

	// Disable Animation Rotation and Keyboard
	//[UIView setAnimationsEnabled:NO];
	
	// Keep Screen On
	[[UIApplication sharedApplication] setIdleTimerDisabled:YES];
	
	// Black Background With White Status Bar
	[self setNeedsStatusBarAppearanceUpdate];
	self.view.backgroundColor = [UIColor blackColor];
	
	jappsyMixerInit();
}

- (void)awakeFromNib {
	[super awakeFromNib];
	
	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationChanged:) name:UIDeviceOrientationDidChangeNotification object:nil];
}

- (void)orientationChanged:(NSNotification *)notification {
	UIDeviceOrientation deviceOrientation = [UIDevice currentDevice].orientation;
	
	if (deviceOrientation == UIDeviceOrientationFaceDown) {
		[omView onPause];
	} else if (deviceOrientation == UIDeviceOrientationFaceUp) {
		[omView onResume];
	} else {
		if (deviceOrientation != UIDeviceOrientationUnknown) {
			NSArray* nextConstraintList = NULL;
			if (deviceOrientation != UIDeviceOrientationPortrait) {
				nextConstraintList = landscapeConstraintList;
			} else {
				nextConstraintList = potrtaitConstraintList;
			}
			if (nextConstraintList != lastConstraintList) {
				[self.view removeConstraints:lastConstraintList];
				[self.view addConstraints:nextConstraintList];
				lastConstraintList = nextConstraintList;
			}
		}
	}
}

- (void)dealloc {
	jappsyMixerQuit();
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (UIStatusBarStyle)preferredStatusBarStyle {
	return UIStatusBarStyleLightContent;
}

/*
- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation {
	self.view.backgroundColor = [UIColor blackColor];
}
 */

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

static int enterCount = 0;

-(BOOL)textView:(UITextView *)textView shouldChangeTextInRange:
(NSRange)range replacementText:(NSString *)text{
	if ([text isEqualToString:@"\n"]) {
		enterCount++;
		if (enterCount > 1) {
			[textView resignFirstResponder];
			return NO;
		}
	} else {
		enterCount = 0;
	}
    return YES;
}
-(void)textViewDidBeginEditing:(UITextView *)textView{
    NSLog(@"Did begin editing");
}
-(void)textViewDidChange:(UITextView *)textView{
    NSLog(@"Did Change");
    
}
-(void)textViewDidEndEditing:(UITextView *)textView{
    NSLog(@"Did End editing");
    
}
-(BOOL)textViewShouldEndEditing:(UITextView *)textView{
    [textView resignFirstResponder];
    return YES;
}

@end
