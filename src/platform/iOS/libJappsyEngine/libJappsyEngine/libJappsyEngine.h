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

//! Project version number for libJappsyEngine.
FOUNDATION_EXPORT double libJappsyEngineVersionNumber;

//! Project version string for libJappsyEngine.
FOUNDATION_EXPORT const unsigned char libJappsyEngineVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <libJappsyEngine/PublicHeader.h>

FOUNDATION_EXPORT void jappsyInit(const char* cacheDir, void* system);
FOUNDATION_EXPORT void jappsyQuit();

#import <libJappsyEngine/JappsyView.h>

