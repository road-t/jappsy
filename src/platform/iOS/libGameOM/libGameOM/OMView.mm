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
#import "OMWebView.h"

#import <libJappsyEngine/libJappsyEngine.h>
#include "OMGame.h"
#include <core/uMemory.h>

@interface OMView()

@property(nonatomic)OMGame *engine;

@property(nonatomic)BOOL defaultPortrait;
@property(nonatomic)BOOL portrait;
@property(nonatomic)BOOL startup;

@property(nonatomic)int state;

@property(strong,nonatomic)OMWebView *calendarView;
@property(strong,nonatomic)JappsyView *gameView;
@property(strong,nonatomic)OMWebView *helpView;
@property(strong,nonatomic)OMWebView *loadView;
@property(strong,nonatomic)OMWebView *errorView;

@property(strong,nonatomic)NSArray *lastLayout;
@property(strong,nonatomic)NSMutableArray *layout;

@property(strong,nonatomic)NSMutableArray *lpnCalendar;
@property(strong,nonatomic)NSMutableArray *lpmCalendar;
@property(strong,nonatomic)NSMutableArray *llnCalendar;
@property(strong,nonatomic)NSMutableArray *llmCalendar;

@property(strong,nonatomic)NSMutableArray *lpnGame;
@property(strong,nonatomic)NSMutableArray *lpmGame;
@property(strong,nonatomic)NSMutableArray *llnGame;
@property(strong,nonatomic)NSMutableArray *llmGame;

@property(strong,nonatomic)NSMutableArray *lpvHelp;
@property(strong,nonatomic)NSMutableArray *lphHelp;
@property(strong,nonatomic)NSMutableArray *llvHelp;
@property(strong,nonatomic)NSMutableArray *llhHelp;

@property(strong,nonatomic)NSMutableArray *lpvLoad;
@property(strong,nonatomic)NSMutableArray *lphLoad;
@property(strong,nonatomic)NSMutableArray *llvLoad;
@property(strong,nonatomic)NSMutableArray *llhLoad;

@property(strong,nonatomic)NSMutableArray *lpvError;
@property(strong,nonatomic)NSMutableArray *lphError;
@property(strong,nonatomic)NSMutableArray *llvError;
@property(strong,nonatomic)NSMutableArray *llhError;

@end

enum {
    OMVIEW_LOAD_ERROR = -2,
    OMVIEW_LOAD = -1,
    OMVIEW_GAME = 0,
    OMVIEW_HELP = 1,
    OMVIEW_ERROR = 2
} OMViewState;

@implementation OMView

@synthesize engine;
@synthesize state;

@synthesize calendarView;
@synthesize gameView;
@synthesize helpView;
@synthesize loadView;
@synthesize errorView;

@synthesize lastLayout;
@synthesize layout;

@synthesize lpnCalendar;
@synthesize lpmCalendar;
@synthesize llnCalendar;
@synthesize llmCalendar;

@synthesize lpnGame;
@synthesize lpmGame;
@synthesize llnGame;
@synthesize llmGame;

@synthesize lpvHelp;
@synthesize lphHelp;
@synthesize llvHelp;
@synthesize llhHelp;

@synthesize lpvLoad;
@synthesize lphLoad;
@synthesize llvLoad;
@synthesize llhLoad;

@synthesize lpvError;
@synthesize lphError;
@synthesize llvError;
@synthesize llhError;

NSLayoutConstraint* ConstraintPriotiry(NSLayoutConstraint* constraint, UILayoutPriority priority) {
    constraint.priority = priority;
    return constraint;
}

- (instancetype) init {
    if ((self = [super init])) {
        calendarView = [[OMWebView alloc] init];
        [calendarView setOpaque:NO];
        calendarView.backgroundColor = [UIColor clearColor];
        [calendarView setTranslatesAutoresizingMaskIntoConstraints:NO];
        [calendarView loadHTMLString:@"<!DOCTYPE html><html><head></head><body style=\"background:#000;\"></body></html>" baseURL:nil];
        [self addSubview:calendarView];

        gameView = [[JappsyView alloc] initWithFrame:CGRectMake(0,0,100,100)];
        [gameView setTranslatesAutoresizingMaskIntoConstraints:NO];
        gameView.backgroundColor = [UIColor blackColor];
        [self addSubview:gameView];

        helpView = [[OMWebView alloc] init];
        [helpView setOpaque:NO];
        helpView.backgroundColor = [UIColor clearColor];
        [helpView setTranslatesAutoresizingMaskIntoConstraints:NO];
        [helpView loadHTMLString:@"<!DOCTYPE html><html><head></head><body style=\"background:#000;\"></body></html>" baseURL:nil];
        [self addSubview:helpView];
        
        loadView = [[OMWebView alloc] init];
        [loadView setOpaque:NO];
        loadView.backgroundColor = [UIColor clearColor];
        [loadView setTranslatesAutoresizingMaskIntoConstraints:NO];
        [loadView loadHTMLString:@"<!DOCTYPE html><html><head></head><body style=\"background:#000;\"></body></html>" baseURL:nil];
        [self addSubview:loadView];
        
        errorView = [[OMWebView alloc] init];
        [errorView setOpaque:NO];
        errorView.backgroundColor = [UIColor clearColor];
        [errorView setTranslatesAutoresizingMaskIntoConstraints:NO];
        [errorView loadHTMLString:@"<!DOCTYPE html><html><head></head><body style=\"background:#000;\"></body></html>" baseURL:nil];
        [self addSubview:errorView];
        
        lastLayout = NULL;
        layout = NULL;
        lpnCalendar = lpmCalendar = llnCalendar = llmCalendar = NULL;
        lpnGame = lpmGame = llnGame = llmGame = NULL;
        lpvHelp = lphHelp = llvHelp = llhHelp = NULL;
        lpvLoad = lphLoad = llvLoad = llhLoad = NULL;
        lpvError = lphError = llvError = llhError = NULL;
        
        _startup = TRUE;
        state = OMVIEW_LOAD;

        [self setTranslatesAutoresizingMaskIntoConstraints:NO];
        self.backgroundColor = [UIColor blackColor];
    }
    return self;
}

- (void) initConstraints:(UIViewController*)controller {
    lpnCalendar = [[NSMutableArray alloc] init];
    [lpnCalendar addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:controller.topLayoutGuide attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [lpnCalendar addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0],999)];
    [lpnCalendar addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeCenterX multiplier:1.0 constant:0.0],999)];
    [lpnCalendar addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:gameView attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0], 997)];

    lpmCalendar = [[NSMutableArray alloc] init];
    [lpmCalendar addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:controller.topLayoutGuide attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [lpmCalendar addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [lpmCalendar addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
    [lpmCalendar addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];
    
    llnCalendar = llmCalendar = [[NSMutableArray alloc] init];
    [llmCalendar addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],999)];
    [llmCalendar addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [llmCalendar addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
    [llmCalendar addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:calendarView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];
    
    
    lpnGame = [[NSMutableArray alloc] init];
    [lpnGame addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [lpnGame addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0],999)];
    [lpnGame addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeCenterX relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeCenterX multiplier:1.0 constant:0.0],999)];
    [lpnGame addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:gameView attribute:NSLayoutAttributeWidth multiplier:(1080.0 / 1920.0) constant:0.0],998)];
    
    lpmGame = llmGame = [[NSMutableArray alloc] init];
    [lpmGame addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:-10.0],999)];
    [lpmGame addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:-10.0],999)];
    [lpmGame addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:150.0],999)];
    [lpmGame addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:gameView attribute:NSLayoutAttributeWidth multiplier:(1080.0 / 1920.0) constant:0.0],998)];

    llnGame = [[NSMutableArray alloc] init];
    [llnGame addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],999)];
    [llnGame addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [llnGame addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
    [llnGame addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:gameView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];
    
    
    lpvHelp = [[NSMutableArray alloc] init];
    [lpvHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:controller.topLayoutGuide attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [lpvHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [lpvHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
    [lpvHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];

    lphHelp = [[NSMutableArray alloc] init];
    [lphHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:controller.topLayoutGuide attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [lphHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [lphHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];
    [lphHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0],999)];
 
    llvHelp = [[NSMutableArray alloc] init];
    [llvHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],999)];
    [llvHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [llvHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
    [llvHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];

    llhHelp = [[NSMutableArray alloc] init];
    [llhHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],999)];
    [llhHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [llhHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];
    [llhHelp addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:helpView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeWidth multiplier:1.0 constant:0.0],999)];


    lpvLoad = lphLoad = [[NSMutableArray alloc] init];
    [lpvLoad addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:loadView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:controller.topLayoutGuide attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [lpvLoad addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:loadView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [lpvLoad addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:loadView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
    [lpvLoad addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:loadView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];
    
    llvLoad = llhLoad = [[NSMutableArray alloc] init];
    [llvLoad addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:loadView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],999)];
    [llvLoad addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:loadView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [llvLoad addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:loadView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
    [llvLoad addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:loadView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];

    
    lpvError = lphError = [[NSMutableArray alloc] init];
    [lpvError addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:errorView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:controller.topLayoutGuide attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [lpvError addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:errorView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [lpvError addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:errorView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
    [lpvError addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:errorView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];
    
    llvError = llhError = [[NSMutableArray alloc] init];
    [llvError addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:errorView attribute:NSLayoutAttributeTop relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeTop multiplier:1.0 constant:0.0],999)];
    [llvError addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:errorView attribute:NSLayoutAttributeBottom relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeBottom multiplier:1.0 constant:0.0],999)];
    [llvError addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:errorView attribute:NSLayoutAttributeLeft relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeLeft multiplier:1.0 constant:0.0],999)];
    [llvError addObject:ConstraintPriotiry([NSLayoutConstraint constraintWithItem:errorView attribute:NSLayoutAttributeRight relatedBy:NSLayoutRelationEqual toItem:self attribute:NSLayoutAttributeRight multiplier:1.0 constant:0.0],999)];

    
    UIInterfaceOrientation interfaceOrientation = [[UIApplication sharedApplication] statusBarOrientation];
    if (UIInterfaceOrientationIsPortrait(interfaceOrientation)) {
        _defaultPortrait = TRUE;
        _portrait = TRUE;
        
        layout = [[NSMutableArray alloc] init];
        [layout addObjectsFromArray:lpnCalendar];
        [layout addObjectsFromArray:lpnGame];
        [layout addObjectsFromArray:lphHelp];
        [layout addObjectsFromArray:lpvLoad];
        [layout addObjectsFromArray:lphError];
    } else {
        _defaultPortrait = FALSE;
        _portrait = FALSE;

        layout = [[NSMutableArray alloc] init];
        [layout addObjectsFromArray:llmCalendar];
        [layout addObjectsFromArray:llmGame];
        [layout addObjectsFromArray:llhHelp];
        [layout addObjectsFromArray:llvLoad];
        [layout addObjectsFromArray:llhError];
    }

    lastLayout = layout;
    [self addConstraints:layout];

    [calendarView setHidden:YES];
    [gameView setHidden:YES];
    helpView.alpha = 0.0f;
    [helpView setHidden:YES];
    errorView.alpha = 0.0f;
    [errorView setHidden:YES];
}

- (void) orientationChanged:(NSNotification *)notification {
    UIDeviceOrientation deviceOrientation = [UIDevice currentDevice].orientation;
    
    if (deviceOrientation == UIDeviceOrientationFaceDown) {
        [self onPause:false];
    } else if (deviceOrientation == UIDeviceOrientationFaceUp) {
        [self onResume:false];
    } else {
        if (deviceOrientation != UIDeviceOrientationUnknown) {
            if (_startup) {
                _startup = FALSE;
                if (deviceOrientation != UIDeviceOrientationPortrait) {
                    _portrait = false;
                    if (_defaultPortrait) {
                        [gameView layout:false animate:false];
                    } else {
                        [gameView layout:true animate:false];
                    }
                } else {
                    _portrait = true;
                    [gameView layout:false animate:false];
                }
            } else {
                if (deviceOrientation != UIDeviceOrientationPortrait) {
                    if (_portrait) {
                        _portrait = false;
                        [gameView layout:[gameView minimized] animate:false];
                    }
                } else {
                    if (!_portrait) {
                        _portrait = true;
                        [gameView layout:[gameView minimized] animate:false];
                    }
                }
            }
        }
    }
}

void onLayout(bool minimize, bool animate, void* userData) {
    OMView* omView = (__bridge OMView*)userData;
    [omView onLayout:minimize animate:animate];
}

- (void) onLayout:(BOOL)minimize animate:(BOOL)animate {
    layout = [[NSMutableArray alloc] init];
    
    if (_portrait) {
        if (minimize) {
            [layout addObjectsFromArray:lpmCalendar];
            [layout addObjectsFromArray:lpmGame];
        } else {
            [layout addObjectsFromArray:lpnCalendar];
            [layout addObjectsFromArray:lpnGame];
        }
        switch (state) {
            case OMVIEW_LOAD_ERROR:
                [layout addObjectsFromArray:lphHelp];
                [layout addObjectsFromArray:lpvLoad];
                [layout addObjectsFromArray:lpvError];
                break;
            case OMVIEW_LOAD:
                [layout addObjectsFromArray:lphHelp];
                [layout addObjectsFromArray:lpvLoad];
                [layout addObjectsFromArray:lphError];
                break;
            case OMVIEW_GAME:
                [layout addObjectsFromArray:lphHelp];
                [layout addObjectsFromArray:lphLoad];
                [layout addObjectsFromArray:lphError];
                break;
            case OMVIEW_HELP:
                [layout addObjectsFromArray:lpvHelp];
                [layout addObjectsFromArray:lphLoad];
                [layout addObjectsFromArray:lphError];
                break;
            case OMVIEW_ERROR:
                [layout addObjectsFromArray:lphHelp];
                [layout addObjectsFromArray:lphLoad];
                [layout addObjectsFromArray:lpvError];
                break;
        }
    } else {
        if (minimize) {
            [layout addObjectsFromArray:llmCalendar];
            [layout addObjectsFromArray:llmGame];
        } else {
            [layout addObjectsFromArray:llnCalendar];
            [layout addObjectsFromArray:llnGame];
        }
        switch (state) {
            case OMVIEW_LOAD_ERROR:
                [layout addObjectsFromArray:llhHelp];
                [layout addObjectsFromArray:llvLoad];
                [layout addObjectsFromArray:llvError];
                break;
            case OMVIEW_LOAD:
                [layout addObjectsFromArray:llhHelp];
                [layout addObjectsFromArray:llvLoad];
                [layout addObjectsFromArray:llhError];
                break;
            case OMVIEW_GAME:
                [layout addObjectsFromArray:llhHelp];
                [layout addObjectsFromArray:llhLoad];
                [layout addObjectsFromArray:llhError];
                break;
            case OMVIEW_HELP:
                [layout addObjectsFromArray:llvHelp];
                [layout addObjectsFromArray:llhLoad];
                [layout addObjectsFromArray:llhError];
                break;
            case OMVIEW_ERROR:
                [layout addObjectsFromArray:llhHelp];
                [layout addObjectsFromArray:llhLoad];
                [layout addObjectsFromArray:llvError];
                break;
        }
    }
    
    if ((lastLayout == NULL) || (![layout isEqualToArray:lastLayout])) {
        if (animate) {
            [self layoutIfNeeded];
        } else {
            if (minimize) {
                gameView.alpha = 0.8f;
            } else {
                gameView.alpha = 1.0f;
            }
            switch (state) {
                case OMVIEW_LOAD_ERROR:
                    [calendarView setHidden:YES];
                    [gameView setHidden:YES];
                    helpView.alpha = 0.0f;
                    [helpView setHidden:YES];
                    loadView.alpha = 1.0f;
                    [loadView setHidden:NO];
                    errorView.alpha = 1.0f;
                    [errorView setHidden:NO];
                    break;
                case OMVIEW_LOAD:
                    [calendarView setHidden:YES];
                    [gameView setHidden:YES];
                    helpView.alpha = 0.0f;
                    [helpView setHidden:YES];
                    loadView.alpha = 1.0f;
                    [loadView setHidden:NO];
                    errorView.alpha = 0.0f;
                    [errorView setHidden:YES];
                    break;
                case OMVIEW_GAME:
                    [calendarView setHidden:NO];
                    [gameView setHidden:NO];
                    helpView.alpha = 0.0f;
                    [helpView setHidden:YES];
                    loadView.alpha = 0.0f;
                    [loadView setHidden:YES];
                    errorView.alpha = 0.0f;
                    [errorView setHidden:YES];
                    break;
                case OMVIEW_HELP:
                    [calendarView setHidden:NO];
                    [gameView setHidden:NO];
                    helpView.alpha = 1.0f;
                    [helpView setHidden:NO];
                    loadView.alpha = 0.0f;
                    [loadView setHidden:YES];
                    errorView.alpha = 0.0f;
                    [errorView setHidden:YES];
                    break;
                case OMVIEW_ERROR:
                    [calendarView setHidden:NO];
                    [gameView setHidden:NO];
                    helpView.alpha = 0.0f;
                    [helpView setHidden:YES];
                    loadView.alpha = 0.0f;
                    [loadView setHidden:YES];
                    errorView.alpha = 1.0f;
                    [errorView setHidden:NO];
                    break;
            }
        }
        
        [self removeConstraints:lastLayout];
        [self addConstraints:layout];
        lastLayout = layout;
        
        if (animate) {
            switch (state) {
                case OMVIEW_LOAD_ERROR:
                    [loadView setHidden:NO];
                    [errorView setHidden:NO];
                    break;
                case OMVIEW_LOAD:
                    [loadView setHidden:NO];
                    break;
                case OMVIEW_GAME:
                    [calendarView setHidden:NO];
                    [gameView setHidden:NO];
                    break;
                case OMVIEW_HELP:
                    [calendarView setHidden:NO];
                    [gameView setHidden:NO];
                    [helpView setHidden:NO];
                    break;
                case OMVIEW_ERROR:
                    [calendarView setHidden:NO];
                    [gameView setHidden:NO];
                    [errorView setHidden:NO];
                    break;
            }

            [UIView animateWithDuration:0.3 animations:^{
                if (minimize) {
                    gameView.alpha = 0.8f;
                } else {
                    gameView.alpha = 1.0f;
                }

                switch (state) {
                    case OMVIEW_LOAD_ERROR:
                        helpView.alpha = 0.0f;
                        loadView.alpha = 1.0f;
                        errorView.alpha = 1.0f;
                        break;
                    case OMVIEW_LOAD:
                        helpView.alpha = 0.0f;
                        loadView.alpha = 1.0f;
                        errorView.alpha = 0.0f;
                        break;
                    case OMVIEW_GAME:
                        helpView.alpha = 0.0f;
                        loadView.alpha = 0.0f;
                        errorView.alpha = 0.0f;
                        break;
                    case OMVIEW_HELP:
                        helpView.alpha = 1.0f;
                        loadView.alpha = 0.0f;
                        errorView.alpha = 0.0f;
                        break;
                    case OMVIEW_ERROR:
                        helpView.alpha = 0.0f;
                        loadView.alpha = 0.0f;
                        errorView.alpha = 1.0f;
                        break;
                }

                [self layoutIfNeeded];
            } completion: ^(BOOL finished) {
                if (finished) {
                    switch (state) {
                        case OMVIEW_LOAD_ERROR:
                            [calendarView setHidden:YES];
                            [gameView setHidden:YES];
                            [helpView setHidden:YES];
                            break;
                        case OMVIEW_LOAD:
                            [calendarView setHidden:YES];
                            [gameView setHidden:YES];
                            [helpView setHidden:YES];
                            [errorView setHidden:YES];
                            break;
                        case OMVIEW_GAME:
                            [helpView setHidden:YES];
                            [loadView setHidden:YES];
                            [errorView setHidden:YES];
                            break;
                        case OMVIEW_HELP:
                            [loadView setHidden:YES];
                            [errorView setHidden:YES];
                            break;
                        case OMVIEW_ERROR:
                            [helpView setHidden:YES];
                            [loadView setHidden:YES];
                            break;
                    }
                }
            }];
        }
    }
}

void onLocation(int index, const CString& url, void* userData) {
    OMView* omView = (__bridge OMView*)userData;
    [omView onLocation:index location:(NSString*)url];
}

CString onScript(int index, const CString& script, void* userData) {
    OMView* omView = (__bridge OMView*)userData;
    return [omView onScript:index script:(NSString*)script];
}

- (void) onLocation:(int)index location:(NSString*)url {
    UIWebView* webView = NULL;
    
    switch (index) {
        case 0: webView = calendarView; break;
        case 1: webView = helpView; break;
    }
    
    if (webView != NULL) {
        [webView loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:url]]];
    }
}

- (NSString*) onScript:(int)index script:(NSString*)script {
    UIWebView* webView = NULL;
    
    switch (index) {
        case 0: webView = calendarView; break;
        case 1: webView = helpView; break;
    }
    
    if (webView != NULL) {
        return [webView stringByEvaluatingJavaScriptFromString:script];
    }
    
    return NULL;
}

- (BOOL) onStart {
    if ([gameView onStart]) {
        engine = new class OMGame();
        engine->setOnLayout(onLayout, (__bridge void*)self);
        engine->setWebCallbacks(onLocation, onScript, (__bridge void*)self);
        [gameView engine:engine];
        [calendarView engine:engine index:0];
        //[helpView engine:engine index:1];
        
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
