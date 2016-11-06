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

#ifndef OMViewH
#define OMViewH

// При переходе в другой раздел приложения прячется игра
#define OMVIEW_VISIBILITY   0x0300
#define OMVIEW_HIDE         0x0100
#define OMVIEW_SHOW         0x0200

// При сворачивании приложения останавливается игра
#define OMVIEW_RUNNING      0x0C00
#define OMVIEW_STOP         0x0400
#define OMVIEW_RUN          0x0800
    
// При перевороте экрана в низ останавливается игра и весь вывод на экран
#define OMVIEW_SUSPENDED    0xC000
#define OMVIEW_PAUSE        0x4000
#define OMVIEW_RESUME       0x8000
    
#define OMVIEW_MINIMIZED    0x3000
#define OMVIEW_MINIMIZE     0x1000
#define OMVIEW_RESTORE      0x2000
    
#define OMVIEW_ANIMATE      0x10000
    
#define OMVIEW_UPDATE       0
    
#define OMVIEW_ACTIVITY     0x00FF
#define OMVIEW_LOAD_ERROR   1
#define OMVIEW_LOAD         2
#define OMVIEW_GAME         3
#define OMVIEW_HELP         4
#define OMVIEW_ERROR        5

#endif

#ifdef __OBJC__

#import <UIKit/UIKit.h>

typedef void (*OMViewCloseCallback)(void* userData);

@interface OMView : UIView

- (instancetype) init:(NSString*)basePath token:(NSString*)token sessid:(NSString*)sessid devid:(NSString*)devid locale:(NSString*)locale onclose:(OMViewCloseCallback)callback userData:(void*)userData;

- (void) orientationChanged:(NSNotification *)notification;

- (BOOL) onStart;
- (BOOL) onStop;

- (void) updateState:(int)state;

@end

#endif
