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

#import "OMView.h"

#import <libJappsyEngine/libJappsyEngine.h>
#include "OMGame.h"
#include <core/uMemory.h>

@interface OMView()

@property(nonatomic)BOOL defaultPortrait;
@property(nonatomic)BOOL portrait;

@property(strong,nonatomic)UIWebView *calendarView;
@property(strong,nonatomic)JappsyView *gameView;
@property(strong,nonatomic)UIWebView *helpView;
@property(strong,nonatomic)NSArray *lastConstraintList;
@property(strong,nonatomic)NSMutableArray *portraitNormal;
@property(strong,nonatomic)NSMutableArray *portraitMinimized;
@property(strong,nonatomic)NSMutableArray *landscapeNormal;
@property(strong,nonatomic)NSMutableArray *landscapeMinimized;

@end

@implementation OMView

@synthesize calendarView;
@synthesize gameView;
@synthesize helpView;

@synthesize lastConstraintList;
@synthesize portraitNormal;
@synthesize portraitMinimized;
@synthesize landscapeNormal;
@synthesize landscapeMinimized;

NSLayoutConstraint* ConstraintPriotiry(NSLayoutConstraint* constraint, UILayoutPriority priority) {
    constraint.priority = priority;
    return constraint;
}

- (instancetype) init {
    if ((self = [super init])) {
        calendarView = [[UIWebView alloc] init];
        [calendarView setOpaque:NO];
        calendarView.backgroundColor = [UIColor clearColor];
        [calendarView setTranslatesAutoresizingMaskIntoConstraints:NO];
        //[calendarView loadHTMLString:@"<!DOCTYPE html><html><head></head><body style=\"background:#000;\"></body></html>" baseURL:nil];
        [calendarView loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"https://om.jappsy.com/?mobile"]]];
        [self addSubview:calendarView];

        gameView = [[JappsyView alloc] initWithFrame:CGRectMake(0,0,100,100)];
        [gameView setTranslatesAutoresizingMaskIntoConstraints:NO];
        gameView.backgroundColor = [UIColor blackColor];
        [self addSubview:gameView];
        
        portraitNormal = NULL;
        portraitMinimized = NULL;
        landscapeNormal = NULL;
        landscapeMinimized = NULL;

        [self setTranslatesAutoresizingMaskIntoConstraints:NO];
        self.backgroundColor = [UIColor blackColor];
    }
    return self;
}

- (void) initConstraints:(UIViewController*)controller {
    portraitNormal = [[NSMutableArray alloc] init];
    [portraitNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [portraitNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0],999)];
    [portraitNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeCenterX multiplier:1.0 constant:0.0],999)];
    [portraitNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:gameView attribute:NSLayoutAttributeWidth multiplier:(1080.0 / 1920.0) constant:0.0],998)];
    
    [portraitNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:controller.topLayoutGuide attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [portraitNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0],999)];
    [portraitNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeCenterX multiplier:1.0 constant:0.0],999)];
    [portraitNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:gameView attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0], 997)];

    
    portraitMinimized = landscapeMinimized = [[NSMutableArray alloc] init];
    [landscapeMinimized addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],999)];
    [landscapeMinimized addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [landscapeMinimized addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
    [landscapeMinimized addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];
    
    [landscapeMinimized addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:-10.0],999)];
    [landscapeMinimized addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:-10.0],999)];
    [landscapeMinimized addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:150.0],999)];
    [landscapeMinimized addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:gameView attribute:NSLayoutAttributeWidth multiplier:(1080.0 / 1920.0) constant:0.0],998)];

    landscapeNormal = [[NSMutableArray alloc] init];
    [landscapeNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],999)];
    [landscapeNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [landscapeNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
    [landscapeNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];
    
    [landscapeNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],999)];
    [landscapeNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [landscapeNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
    [landscapeNormal addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];

    UIInterfaceOrientation interfaceOrientation = [[UIApplication sharedApplication] statusBarOrientation];
    if (UIInterfaceOrientationIsPortrait(interfaceOrientation)) {
        _defaultPortrait = TRUE;
        _portrait = TRUE;
        lastConstraintList = portraitNormal;
    } else {
        _defaultPortrait = FALSE;
        _portrait = FALSE;
        lastConstraintList = landscapeMinimized;
    }
    [self addConstraints:lastConstraintList];
}

- (void) orientationChanged:(NSNotification *)notification {
    UIDeviceOrientation deviceOrientation = [UIDevice currentDevice].orientation;
    
    if (deviceOrientation == UIDeviceOrientationFaceDown) {
        [self onPause:false];
    } else if (deviceOrientation == UIDeviceOrientationFaceUp) {
        [self onResume:false];
    } else {
        if (deviceOrientation != UIDeviceOrientationUnknown) {
            if (deviceOrientation != UIDeviceOrientationPortrait) {
                _portrait = false;
                if (_defaultPortrait) {
                    [gameView minimize:false animate:false];
                } else {
                    [gameView minimize:true animate:false];
                }
            } else {
                _portrait = true;
                [gameView minimize:false animate:false];
            }
        }
    }
}

void onMinimize(bool minimize, bool animate, void* userData) {
    OMView* omView = (__bridge OMView*)userData;
    [omView onMinimize:minimize animate:animate];
}

- (void) onMinimize:(BOOL)minimize animate:(BOOL)animate {
    NSArray* nextConstraintList = NULL;
    
    if (_portrait) {
        if (minimize) {
            nextConstraintList = portraitMinimized;
        } else {
            nextConstraintList = portraitNormal;
        }
    } else {
        if (minimize) {
            nextConstraintList = landscapeMinimized;
        } else {
            nextConstraintList = landscapeNormal;
        }
    }
    
    if (nextConstraintList != lastConstraintList) {
        if (animate) {
            [self layoutIfNeeded];
        } else {
            if (minimize) {
                gameView.alpha = 0.8f;
            } else {
                gameView.alpha = 1.0f;
            }
        }
        
        [self removeConstraints:lastConstraintList];
        [self addConstraints:nextConstraintList];
        lastConstraintList = nextConstraintList;
        
        if (animate) {
            [UIView animateWithDuration:0.5 animations:^{
                if (minimize) {
                    gameView.alpha = 0.8f;
                } else {
                    gameView.alpha = 1.0f;
                }

                [self layoutIfNeeded];
            }];
        }
    }
}

- (BOOL) onStart {
    if ([gameView onStart]) {
        OMGame* OMGame = new class OMGame();
        OMGame->setOnMinimize(onMinimize, (__bridge void*)self);
        [gameView engine:OMGame];
        
        return YES;
    }
    
    return NO;
}

- (BOOL) onStop {
    return [gameView onStop];
}

- (void) onResume:(BOOL)app {
    [gameView onResume:app];
}

- (void) onPause:(BOOL)app {
    [gameView onPause:app];
}

@end
