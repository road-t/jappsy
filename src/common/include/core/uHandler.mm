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

const wchar_t TypeHandler[] = L"Handler::";

RefHandler::HandlerCallback::HandlerCallback(Callback callback, int delay, const Object& userData) {
	this->callback = callback;
	this->delay = delay;
	this->userData = new Object(userData);
}

@interface iOSHandler : NSObject

+(void)threadPost:(NSValue *)queueData;
+(void)threadRun:(NSArray *)threadData;

@end

#define HANDLER_QUEUE_BLOCK_SIZE	16

RefHandler::HandlerCallback** RefHandler::resize(uint32_t count) throw(const char*) {
	uint32_t newSize = count * sizeof(HandlerCallback*);
	
	HandlerCallback** queue = (HandlerCallback**)AtomicGetPtr((void**)&(this->queue));
	if (newSize == 0) {
		if (queue != NULL) {
			memFree(queue);
			AtomicSetPtr(&(this->queue), NULL);
		}
		AtomicSet(&(this->count), 0);
		AtomicSet(&(this->memorySize), 0);
		return NULL;
	}
	
	uint32_t newMemSize = newSize - (newSize % HANDLER_QUEUE_BLOCK_SIZE) + HANDLER_QUEUE_BLOCK_SIZE;
	if (AtomicGet(&memorySize) != newMemSize) {
		HandlerCallback** newQueue = memRealloc(HandlerCallback*, newQueue, queue, newMemSize);
		if (newQueue) {
			AtomicSetPtr(&(this->queue), newQueue);
			AtomicSet(&(this->memorySize), newMemSize);
			queue = newQueue;
		} else {
			throw eOutOfMemory;
		}
	}
	
	AtomicSet(&(this->count), count);
	return queue;
}

void RefHandler::release() {
	this->wait();
	uint32_t count = AtomicGet(&(this->count));
	HandlerCallback** queue = (HandlerCallback**)AtomicGetPtr((void**)&(this->queue));
	for (uint32_t i = 0; i < count; i++) {
		HandlerCallback* runner = AtomicGetPtr(&(queue[i]));
		AtomicSet(&(runner->shutdown), 1);
	}
	
	AtomicSet(&shutdown, 1);
	while (AtomicGet(&(this->count)) > 0) {
		this->notifyAll();
		sleep(1);
		this->wait();
	}
	this->notifyAll();
}

RefHandler::~RefHandler() {
	release();
}

void RefHandler::push(HandlerCallback* runner) {
	uint32_t count = AtomicGet(&(this->count));
	HandlerCallback** queue = resize(count+1);
	AtomicSetPtr(&(queue[count]), runner);
}

void RefHandler::remove(HandlerCallback* runner) {
	uint32_t count = AtomicGet(&(this->count));
	HandlerCallback** queue = (HandlerCallback**)AtomicGetPtr((void**)&(this->queue));
	for (uint32_t i = 0; i < count; i++) {
		if (AtomicGetPtr(&(queue[i])) == runner) {
			if (i < (count - 1)) {
				atomic_memmove(&queue[i], &queue[i+1], (count-i-1) * sizeof(HandlerCallback*));
			}
			(void)resize(count-1);
			break;
		}
	}
}

void* RefHandler::postDelayed(Callback callback, int delay, const Object& userData) {
	this->wait();
	if (AtomicGet(&shutdown) == 0) {
		HandlerCallback* runner = memNew(runner, HandlerCallback(callback, delay, userData));
		push(runner);
		Handler* handler = new Handler(this);
		[NSThread detachNewThreadSelector:@selector(threadPost:) toTarget:[iOSHandler class] withObject:@[[NSValue valueWithPointer:handler], [NSValue valueWithPointer:runner]]];

		this->notifyAll();
		return runner;
	}
	this->notifyAll();
	return NULL;
}

void* RefHandler::post(Callback callback, const Object& userData) {
	this->wait();
	if (AtomicGet(&shutdown) == 0) {
		HandlerCallback* runner = memNew(runner, HandlerCallback(callback, 0, userData));
		push(runner);
		Handler* handler = new Handler(this);
		[NSThread detachNewThreadSelector:@selector(threadPost:) toTarget:[iOSHandler class] withObject:@[[NSValue valueWithPointer:handler], [NSValue valueWithPointer:runner]]];
	
		this->notifyAll();
		return runner;
	}
	this->notifyAll();
	return NULL;
}

void RefHandler::removeCallbacks(Callback callback) {
	this->wait();
	uint32_t count = AtomicGet(&(this->count));
	HandlerCallback** queue = (HandlerCallback**)AtomicGetPtr((void**)&(this->queue));
	for (uint32_t i = 0; i < count; i++) {
		HandlerCallback* runner = AtomicGetPtr(&(queue[i]));
		if (AtomicGetPtr(&(runner->callback)) == callback)
			AtomicSet(&(runner->shutdown), 1);
	}
	this->notifyAll();
}

void RefHandler::remove(void* runnerid) {
	this->wait();
	HandlerCallback* runner = (HandlerCallback*)runnerid;
	uint32_t count = AtomicGet(&(this->count));
	HandlerCallback** queue = (HandlerCallback**)AtomicGetPtr((void**)&(this->queue));
	for (uint32_t i = 0; i < count; i++) {
		if (AtomicGetPtr(&(queue[i])) == runner)
			AtomicSet(&(runner->shutdown), 1);
	}
	this->notifyAll();
}

void RefHandler::onthread(Handler* handler, void* runnerid) {
	HandlerCallback* runner = (HandlerCallback*)runnerid;
	
	float delay = (float)(AtomicGet(&(runner->delay))) / 1000.0;
	
	if (delay > 0) {
		[NSThread sleepForTimeInterval:delay];
	}
	
	if (AtomicGet(&(runner->shutdown)) == 0) {
		[[iOSHandler class] performSelectorOnMainThread:@selector(threadRun:) withObject:@[[NSValue valueWithPointer:handler], [NSValue valueWithPointer:runner]] waitUntilDone:NO];
	} else {
		__sync_synchronize();
		
		(*handler)->wait();
		(*handler)->remove(runner);
		memDelete(runner);
		(*handler)->notifyAll();
		delete handler;
	}
}

void RefHandler::onrun(Handler* handler, void* runnerid) {
	HandlerCallback* runner = (HandlerCallback*)runnerid;

	__sync_synchronize();
	
	runner->callback(runner->userData);
	(*handler)->wait();
	(*handler)->remove(runner);
	memDelete(runner);
	(*handler)->notifyAll();
	delete handler;
}

@implementation iOSHandler

+(void)threadPost:(NSArray *)threadData {
	@autoreleasepool {
		NSValue* val0 = threadData[0];
		Handler* handler = (Handler*)([val0 pointerValue]);
		NSValue* val1 = threadData[1];
		void* runner = (void*)([val1 pointerValue]);
		RefHandler::onthread(handler, runner);
	}
}

+(void)threadRun:(NSArray *)threadData {
	@autoreleasepool {
		NSValue* val0 = threadData[0];
		Handler* handler = (Handler*)([val0 pointerValue]);
		NSValue* val1 = threadData[1];
		void* runner = (void*)([val1 pointerValue]);
		RefHandler::onrun(handler, runner);
	}
}

@end
