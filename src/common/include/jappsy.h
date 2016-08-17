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

#ifndef JAPPSY_H
#define JAPPSY_H

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize all core static variables
void jappsyInit();

// Deinitialize all core static variables
void jappsyQuit();

#if defined(__JNI__)
	typedef void* (*ThreadMessageCallback)(void*);

	struct ThreadMessage {
		ThreadMessageCallback callback;
		void* data;
	};

	void postMainThreadMessage(struct ThreadMessage* msg);
#endif

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_H
