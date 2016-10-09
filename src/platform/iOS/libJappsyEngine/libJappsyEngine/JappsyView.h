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

#import <UIKit/UIKit.h>

@interface JappsyView : UIView

@property (readonly, nonatomic, getter=isRunning) BOOL running;
@property (nonatomic) NSInteger interval;
@property (readonly, nonatomic, getter=isRunning) BOOL stopping;

- (instancetype) init;
- (instancetype) initWithFrame:(CGRect)frame;
- (instancetype) initWithCoder:(NSCoder *)decoder;

- (BOOL) onStart;
- (BOOL) onStop;
- (void) onResume:(BOOL)app;
- (void) onPause:(BOOL)app;
- (void) drawView:(id)sender;
- (void) engine:(void*)refEngine;

- (void) minimize:(BOOL)minimize animate:(BOOL)animate;

@end
