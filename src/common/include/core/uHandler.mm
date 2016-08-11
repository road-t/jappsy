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

#include "uHandler.h"
#include <core/uMemory.h>
#include <core/uAtomic.h>

HandlerRunner::HandlerRunner(Callback callback, int delay, void* userData) {
	m_lock = 0;
	m_callback = callback;
	m_delay = delay;
	m_enabled = true;
	m_userData = userData;
}

@interface iOSHandler : NSObject

+(void)threadPost:(NSValue *)queueData;
+(void)threadRun:(NSArray *)threadData;

@end

Handler::Handler() {
	m_retain = 0;
	m_lock = 0;

	m_queue = NULL;
	m_count = 0;
	m_memorySize = 0;
}

#define HANDLER_QUEUE_BLOCK_SIZE	16

void Handler::resize(uint32_t count) throw(const char*) {
	uint32_t newSize = count * sizeof(HandlerRunner*);
	
	if (newSize == 0) {
		if (this->m_queue != NULL) {
			memFree(this->m_queue);
			this->m_queue = NULL;
		}
		this->m_count = 0;
		this->m_memorySize = 0;
		return;
	}
	
	uint32_t newMemSize = newSize - (newSize % HANDLER_QUEUE_BLOCK_SIZE) + HANDLER_QUEUE_BLOCK_SIZE;
	if (this->m_memorySize != newMemSize) {
		HandlerRunner** newQueue = memRealloc(HandlerRunner*, newQueue, this->m_queue, newMemSize);
		if (newQueue) {
			this->m_queue = newQueue;
			this->m_memorySize = newMemSize;
		} else {
			throw eOutOfMemory;
		}
	}
	
	this->m_count = count;
}

Handler::~Handler() {
	AtomicLock(&m_lock);
	for (uint32_t i = 0; i < m_count; i++) {
		HandlerRunner* runner = m_queue[i];
		AtomicLock(&(runner->m_lock));
		runner->m_enabled = false;
		AtomicUnlock(&(runner->m_lock));
	}
	AtomicUnlock(&m_lock);
	
	do {
		AtomicLock(&m_lock);
		if (m_count == 0) {
			AtomicSet(&m_retain, -1);
			break;
		}
		AtomicUnlock(&m_lock);
	} while (true);
	AtomicUnlock(&m_lock);
}

void Handler::push(HandlerRunner* runner) {
	resize(m_count+1);
	m_queue[m_count-1] = runner;
}

void Handler::_remove(HandlerRunner* runner) {
	for (uint32_t i = 0; i < m_count; i++) {
		if (m_queue[i] == runner) {
			if (i < (m_count - 1)) {
				memmove(m_queue[i], m_queue[i+1], (m_count-i-1) * sizeof(HandlerRunner*));
			}
			resize(m_count-1);
			break;
		}
	}
}

HandlerRunner* Handler::postDelayed(HandlerRunner::Callback callback, int delay, void* userData) {
	HandlerRunner* runner = new HandlerRunner(callback, delay, userData);
	AtomicLock(&m_lock);
	if (AtomicCompareExchange(&m_retain, -1, -1) == -1) {
		AtomicUnlock(&m_lock);
		delete runner;
	}
	AtomicIncrement(&m_retain);
	push(runner);
	AtomicUnlock(&m_lock);
	[NSThread detachNewThreadSelector:@selector(threadPost:) toTarget:[iOSHandler class] withObject:@[[NSValue valueWithPointer:this], [NSValue valueWithPointer:runner]]];
	
	return runner;
}

HandlerRunner* Handler::post(HandlerRunner::Callback callback, void* userData) {
	HandlerRunner* runner = new HandlerRunner(callback, 0, userData);
	AtomicLock(&m_lock);
	if (AtomicCompareExchange(&m_retain, -1, -1) == -1) {
		AtomicUnlock(&m_lock);
		delete runner;
	}
	AtomicIncrement(&m_retain);
	push(runner);
	AtomicUnlock(&m_lock);
	[NSThread detachNewThreadSelector:@selector(threadPost:) toTarget:[iOSHandler class] withObject:@[[NSValue valueWithPointer:this], [NSValue valueWithPointer:runner]]];
	
	return runner;
}

void Handler::removeCallbacks(HandlerRunner::Callback callback) {
	AtomicLock(&m_lock);
	for (uint32_t i = 0; i < m_count; i++) {
		HandlerRunner* runner = m_queue[i];
		AtomicLock(&(runner->m_lock));
		if (runner->m_callback == callback)
			runner->m_enabled = false;
		AtomicUnlock(&(runner->m_lock));
	}
	AtomicUnlock(&m_lock);
}

void Handler::remove(HandlerRunner* runner) {
	AtomicLock(&(runner->m_lock));
	runner->m_enabled = false;
	AtomicUnlock(&(runner->m_lock));
}

void Handler::run(HandlerRunner* runner) {
	float delay = 0;
	AtomicLock(&m_lock);
	AtomicLock(&(runner->m_lock));
	delay = (float)(runner->m_delay) / 1000.0;
	AtomicUnlock(&(runner->m_lock));
	AtomicUnlock(&m_lock);
	
	if (delay > 0) {
		[NSThread sleepForTimeInterval:delay];
	}
	
	AtomicLock(&m_lock);
	AtomicLock(&(runner->m_lock));
	if (runner->m_enabled) {
		[[iOSHandler class] performSelectorOnMainThread:@selector(threadRun:) withObject:@[[NSValue valueWithPointer:(void*)(runner->m_callback)], [NSValue valueWithPointer:(void*)(runner->m_userData)]] waitUntilDone:NO];
		runner->m_enabled = false;
	}
	_remove(runner);
	AtomicUnlock(&(runner->m_lock));
	delete runner;
	AtomicDecrement(&m_retain);
	AtomicUnlock(&m_lock);
}

@implementation iOSHandler

+(void)threadPost:(NSArray *)queueData {
	NSValue* val0 = queueData[0];
	Handler* handler = (Handler*)([val0 pointerValue]);
	NSValue* val1 = queueData[1];
	HandlerRunner* runner = (HandlerRunner*)([val1 pointerValue]);
	handler->run(runner);
}

+(void)threadRun:(NSArray *)threadData {
	NSValue* val0 = threadData[0];
	HandlerRunner::Callback m_callback = (HandlerRunner::Callback)([val0 pointerValue]);
	NSValue* val1 = threadData[1];
	void* m_userData = (void*)([val1 pointerValue]);
	m_callback(m_userData);
}

@end
