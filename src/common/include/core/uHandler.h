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

#ifndef JAPPSY_HANDLER_H
#define JAPPSY_HANDLER_H

#include "../platform.h"

class HandlerRunner {
public:
	typedef void (*Callback)(void* userData);
	
	volatile int32_t m_lock;
	
	Callback m_callback;
	volatile int m_delay;
	volatile bool m_enabled;
	volatile void* m_userData;
	
	HandlerRunner(Callback callback, int delay, void* userData);
};

class Handler {
private:
	volatile int32_t m_retain;
	volatile int32_t m_lock;

	HandlerRunner** m_queue;
	volatile uint32_t m_count;
	volatile uint32_t m_memorySize;
	
	void resize(uint32_t count) throw(const char*);
	void push(HandlerRunner* runner);
	void _remove(HandlerRunner* runner);
public:
	Handler();
	~Handler();

	void run(HandlerRunner* runner);

	HandlerRunner* postDelayed(HandlerRunner::Callback callback, int delay, void* userData);
	HandlerRunner* post(HandlerRunner::Callback callback, void* userData);
	void removeCallbacks(HandlerRunner::Callback callback);
	void remove(HandlerRunner* runner);
};

#endif //JAPPSY_HANDLER_H