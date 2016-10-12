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

@implementation OMView

@synthesize engine;

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
        engine = NULL;
        
        calendarView = [[OMWebView alloc] init];
        [calendarView setOpaque:NO];
        calendarView.backgroundColor = [UIColor clearColor];
        [calendarView setTranslatesAutoresizingMaskIntoConstraints:NO];
        [calendarView loadHTMLString:@"<!DOCTYPE html><html><head></head><body style=\"background:#000;\"></body></html>" baseURL:nil];
        calendarView.scrollView.bounces = NO;
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
        helpView.scrollView.bounces = NO;
        [self addSubview:helpView];
        
        const char *sOMLoadHtml =
            #include "../../example/om/OMLoadHtml.res"
        ;
        CString OMLoadHtml = sOMLoadHtml;
        OMLoadHtml = OMLoadHtml.replace(L"{LANG}", L"RU"); // TODO: Real Language
        
        loadView = [[OMWebView alloc] init];
        [loadView setOpaque:NO];
        loadView.backgroundColor = [UIColor clearColor];
        [loadView setTranslatesAutoresizingMaskIntoConstraints:NO];
        [loadView loadHTMLString:(NSString*)OMLoadHtml baseURL:nil];
        loadView.scrollView.bounces = NO;
        [self addSubview:loadView];
        
        const char *sOMErrorHtml =
            #include "../../example/om/OMErrorHtml.res"
        ;
        CString OMErrorHtml = sOMErrorHtml;
        OMErrorHtml = OMErrorHtml.replace(L"{LANG}", L"RU"); // TODO: Real Language
        
        errorView = [[OMWebView alloc] init];
        [errorView setOpaque:NO];
        errorView.backgroundColor = [UIColor clearColor];
        [errorView setTranslatesAutoresizingMaskIntoConstraints:NO];
        [errorView loadHTMLString:(NSString*)OMErrorHtml baseURL:nil];
        errorView.scrollView.bounces = NO;
        [self addSubview:errorView];
        
        lastLayout = NULL;
        layout = NULL;
        lpnCalendar = lpmCalendar = llnCalendar = llmCalendar = NULL;
        lpnGame = lpmGame = llnGame = llmGame = NULL;
        lpvHelp = lphHelp = llvHelp = llhHelp = NULL;
        lpvLoad = lphLoad = llvLoad = llhLoad = NULL;
        lpvError = lphError = llvError = llhError = NULL;
        
        _startup = TRUE;
        _state = OMVIEW_HIDE | OMVIEW_STOP | OMVIEW_RESUME | OMVIEW_LOAD;

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
        
        _state |= OMVIEW_RESTORE;
    } else {
        _defaultPortrait = FALSE;
        _portrait = FALSE;

        layout = [[NSMutableArray alloc] init];
        [layout addObjectsFromArray:llmCalendar];
        [layout addObjectsFromArray:llmGame];
        [layout addObjectsFromArray:llhHelp];
        [layout addObjectsFromArray:llvLoad];
        [layout addObjectsFromArray:llhError];
        
        _state |= OMVIEW_MINIMIZE;
    }

    lastLayout = layout;
    [self addConstraints:layout];

    [calendarView setHidden:YES];
    [gameView setHidden:YES];
    helpView.alpha = 0.0f;
    [helpView setHidden:YES];
    errorView.alpha = 0.0f;
    [errorView setHidden:YES];
    [self setHidden:YES];
}

- (void) orientationChanged:(NSNotification *)notification {
    UIDeviceOrientation deviceOrientation = [UIDevice currentDevice].orientation;
    
    if (deviceOrientation == UIDeviceOrientationFaceDown) {
        [self updateState:OMVIEW_PAUSE];
    } else if (deviceOrientation == UIDeviceOrientationFaceUp) {
        [self updateState:OMVIEW_RESUME];
    } else {
        if (deviceOrientation != UIDeviceOrientationUnknown) {
            if (_startup) {
                _startup = FALSE;
                if (deviceOrientation != UIDeviceOrientationPortrait) {
                    _portrait = false;
                    if (_defaultPortrait) {
                        [self updateState:OMVIEW_RESTORE];
                    } else {
                        [self updateState:OMVIEW_MINIMIZE];
                    }
                } else {
                    _portrait = true;
                    [self updateState:OMVIEW_RESTORE];
                }
            } else {
                if (deviceOrientation != UIDeviceOrientationPortrait) {
                    if (_portrait) {
                        _portrait = false;
                        [self updateState:OMVIEW_UPDATE];
                    }
                } else {
                    if (!_portrait) {
                        _portrait = true;
                        [self updateState:OMVIEW_UPDATE];
                    }
                }
            }
        }
    }
}

struct onUpdateStateThreadData {
    OMView* omView;
    int state;
};

void* onUpdateStateThread(void* userData) {
    onUpdateStateThreadData* thread = (onUpdateStateThreadData*)userData;
    [thread->omView updateState:thread->state];
    return NULL;
}

void onUpdateState(int state, void* userData) {
    onUpdateStateThreadData thread;
    thread.omView = (__bridge OMView*)userData;
    thread.state = state;
    MainThreadSync(onUpdateStateThread, &thread);
}

- (void) updateState:(int)state {
    int update = 0;
    
    if ((state & OMVIEW_RUNNING) != (_state & OMVIEW_RUNNING)) {
        if (state & OMVIEW_STOP) {
            _state = (_state & (~OMVIEW_RUNNING)) | OMVIEW_STOP;
            update |= OMVIEW_STOP;
            state = state & (~OMVIEW_ANIMATE);
        } else if (state & OMVIEW_RUN) {
            _state = (_state & (~OMVIEW_RUNNING)) | OMVIEW_RUN;
            update |= OMVIEW_RUN;
        }
    }
    
    if ((state & OMVIEW_VISIBILITY) != (_state & OMVIEW_VISIBILITY)) {
        if (state & OMVIEW_HIDE) {
            _state = (_state & (~OMVIEW_VISIBILITY)) | OMVIEW_HIDE;
            update |= OMVIEW_HIDE;
        } else if (state & OMVIEW_SHOW) {
            _state = (_state & (~OMVIEW_VISIBILITY)) | OMVIEW_SHOW;
            update |= OMVIEW_SHOW;
        }
    }
    
    if ((state & OMVIEW_SUSPENDED) != (_state & OMVIEW_SUSPENDED)) {
        if (state & OMVIEW_PAUSE) {
            _state = (_state & (~OMVIEW_SUSPENDED)) | OMVIEW_PAUSE;
            update |= OMVIEW_PAUSE;
        } else if (state & OMVIEW_RESUME) {
            _state = (_state & (~OMVIEW_SUSPENDED)) | OMVIEW_RESUME;
            update |= OMVIEW_RESUME;
        }
    }
    
    if ((state & OMVIEW_MINIMIZED) != (_state & OMVIEW_MINIMIZED)) {
        if (state & OMVIEW_MINIMIZE) {
            _state = (_state & (~OMVIEW_MINIMIZED)) | OMVIEW_MINIMIZE;
            update |= OMVIEW_MINIMIZE;
            if (engine != NULL) {
                engine->minimized = true;
            }
        } else if (state & OMVIEW_RESTORE) {
            _state = (_state & (~OMVIEW_MINIMIZED)) | OMVIEW_RESTORE;
            update |= OMVIEW_RESTORE;
            if (engine != NULL) {
                engine->minimized = false;
            }
        }
    }
    
    if ( ((state & OMVIEW_ACTIVITY) != 0) && ((state & OMVIEW_ACTIVITY) != (_state & OMVIEW_ACTIVITY)) ) {
        _state = (_state & (~OMVIEW_ACTIVITY)) | (state & OMVIEW_ACTIVITY);
        update |= (state & OMVIEW_ACTIVITY);
    }
    
    if ((state == OMVIEW_UPDATE) || (update != 0)) {
        if ( ((_state & OMVIEW_STOP) != 0) || ((_state & OMVIEW_HIDE) != 0) || ((_state & OMVIEW_PAUSE) != 0) ) {
            [gameView onPause];
        } else {
            [gameView onResume];
        }
        
        int activity = (_state & OMVIEW_ACTIVITY);
        
        layout = [[NSMutableArray alloc] init];
        
        if (_portrait) {
            if ((_state & OMVIEW_MINIMIZE) != 0) {
                [layout addObjectsFromArray:lpmCalendar];
                [layout addObjectsFromArray:lpmGame];
            } else {
                [layout addObjectsFromArray:lpnCalendar];
                [layout addObjectsFromArray:lpnGame];
            }
            switch (activity) {
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
            if ((_state & OMVIEW_MINIMIZE) != 0) {
                [layout addObjectsFromArray:llmCalendar];
                [layout addObjectsFromArray:llmGame];
            } else {
                [layout addObjectsFromArray:llnCalendar];
                [layout addObjectsFromArray:llnGame];
            }
            switch (activity) {
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
        
        switch (activity) {
            case OMVIEW_LOAD_ERROR:
            case OMVIEW_ERROR:
                [errorView becomeFirstResponder];
                break;
            case OMVIEW_LOAD:
                [loadView becomeFirstResponder];
                break;
            case OMVIEW_GAME:
                [calendarView becomeFirstResponder];
                break;
            case OMVIEW_HELP:
                [helpView becomeFirstResponder];
                break;
        }

        if ((state & OMVIEW_ANIMATE) != 0) {
            [self layoutIfNeeded];
        } else {
            if ((_state & OMVIEW_HIDE) != 0) {
                [self setHidden:YES];
            } else {
                [self setHidden:NO];
            }
            
            if ((_state & OMVIEW_MINIMIZE) != 0) {
                gameView.alpha = 0.8f;
            } else {
                gameView.alpha = 1.0f;
            }
            
            if ((_state & OMVIEW_PAUSE) != 0) {
                [calendarView setHidden:YES];
                [gameView setHidden:YES];
                helpView.alpha = 0.0f;
                [helpView setHidden:YES];
                loadView.alpha = 0.0f;
                [loadView setHidden:YES];
                errorView.alpha = 0.0f;
                [errorView setHidden:YES];
            } else {
                switch (activity) {
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
        }
        
        [self removeConstraints:lastLayout];
        [self addConstraints:layout];
        lastLayout = layout;
        
        if ((state & OMVIEW_ANIMATE) != 0) {
            if ((_state & OMVIEW_HIDE) != 0) {
            } else {
                [self setHidden:NO];
            }

            if ((_state & OMVIEW_PAUSE) != 0) {
            } else {
                switch (activity) {
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
            }
            
            [UIView animateWithDuration:0.3 animations:^{
                if ((_state & OMVIEW_MINIMIZE) != 0) {
                    gameView.alpha = 0.8f;
                } else {
                    gameView.alpha = 1.0f;
                }
                
                if ((_state & OMVIEW_PAUSE) != 0) {
                    helpView.alpha = 0.0f;
                    loadView.alpha = 0.0f;
                    errorView.alpha = 0.0f;
                } else {
                    switch (activity) {
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
                }
                
                [self layoutIfNeeded];
            } completion: ^(BOOL finished) {
                if (finished) {
                    if ((_state & OMVIEW_HIDE) != 0) {
                        [self setHidden:YES];
                    } else {
                    }

                    if ((_state & OMVIEW_PAUSE) != 0) {
                        [calendarView setHidden:YES];
                        [gameView setHidden:YES];
                        [helpView setHidden:YES];
                        [loadView setHidden:YES];
                        [errorView setHidden:YES];
                    } else {
                        switch (activity) {
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
                }
            }];
        }
    }
}

void onLocation(int index, const CString& url, void* userData) {
    OMView* omView = (__bridge OMView*)userData;
    [omView onLocation:index location:(NSString*)url];
}

struct onScriptThreadData {
    OMView* omView;
    int index;
    CString* script;
};

void* onScriptThread(void* userData) {
    onScriptThreadData* thread = (onScriptThreadData*)userData;
    return new CString([thread->omView onScript:thread->index script:(NSString*)*(thread->script)]);
}

CString onScript(int index, const CString& script, void* userData) {
    onScriptThreadData thread;
    thread.omView = (__bridge OMView*)userData;
    thread.index = index;
    thread.script = (CString*)&script;
    CString* threadResult = (CString*)MainThreadSync(onScriptThread, &thread);
    CString result = *threadResult;
    delete threadResult;
    return result;
}

- (void) onLocation:(int)index location:(NSString*)url {
    UIWebView* webView = NULL;
    
    switch (index) {
        case OMVIEW_GAME: webView = calendarView; break;
        case OMVIEW_HELP: webView = helpView; break;
        case OMVIEW_LOAD: webView = loadView; break;
        case OMVIEW_ERROR: webView = errorView; break;
    }
    
    if (webView != NULL) {
        [webView loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:url]]];
    }
}

- (NSString*) onScript:(int)index script:(NSString*)script {
    UIWebView* webView = NULL;
    
    switch (index) {
        case OMVIEW_GAME: webView = calendarView; break;
        case OMVIEW_HELP: webView = helpView; break;
        case OMVIEW_LOAD: webView = loadView; break;
        case OMVIEW_ERROR: webView = errorView; break;
    }
    
    if (webView != NULL) {
        return [webView stringByEvaluatingJavaScriptFromString:script];
    }
    
    return NULL;
}

- (BOOL) onStart {
    if ([gameView onStart]) {
        engine = new class OMGame();
        engine->setOnUpdateState(onUpdateState, (__bridge void*)self);
        engine->setWebCallbacks(onLocation, onScript, (__bridge void*)self);
        [gameView engine:engine];
        [calendarView engine:engine index:OMVIEW_GAME];
        [helpView engine:engine index:OMVIEW_HELP];
        [loadView engine:engine index:OMVIEW_LOAD];
        [errorView engine:engine index:OMVIEW_ERROR];
        engine->minimized = ((_state & OMVIEW_MINIMIZE) != 0);
        
        return YES;
    }
    
    return NO;
}

- (BOOL) onStop {
    return [gameView onStop];
}

@end
