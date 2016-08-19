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
#include <core/uSystem.h>

const wchar_t TypeHandler[] = L"Handler::";

RefHandler::HandlerCallback::HandlerCallback(Callback callback, int delay, const Object& userData) {
	THIS.callback = callback;
	THIS.delay = delay;
	THIS.userData = new Object(userData);
}

#define HANDLER_QUEUE_BLOCK_SIZE	16

RefHandler::HandlerCallback** RefHandler::resize(uint32_t count) throw(const char*) {
	uint32_t newSize = count * sizeof(HandlerCallback*);
	
	HandlerCallback** queue = (HandlerCallback**)AtomicGetPtr((void**)&(THIS.queue));
	if (newSize == 0) {
		if (queue != NULL) {
			memFree(queue);
			AtomicSetPtr(&(THIS.queue), NULL);
		}
		AtomicSet(&(THIS.count), 0);
		AtomicSet(&(THIS.memorySize), 0);
		return NULL;
	}
	
	uint32_t newMemSize = newSize - (newSize % HANDLER_QUEUE_BLOCK_SIZE) + HANDLER_QUEUE_BLOCK_SIZE;
	if (AtomicGet(&memorySize) != newMemSize) {
		HandlerCallback** newQueue = memRealloc(HandlerCallback*, newQueue, queue, newMemSize);
		if (newQueue) {
			AtomicSetPtr(&(THIS.queue), newQueue);
			AtomicSet(&(THIS.memorySize), newMemSize);
			queue = newQueue;
		} else {
			throw eOutOfMemory;
		}
	}
	
	AtomicSet(&(THIS.count), count);
	return queue;
}

void RefHandler::release() {
	THIS.wait();
	uint32_t count = AtomicGet(&(THIS.count));
	HandlerCallback** queue = (HandlerCallback**)AtomicGetPtr((void**)&(THIS.queue));
	for (uint32_t i = 0; i < count; i++) {
		HandlerCallback* runner = AtomicGetPtr(&(queue[i]));
		AtomicSet(&(runner->shutdown), 1);
	}
	
	AtomicSet(&shutdown, 1);
	while (AtomicGet(&(THIS.count)) > 0) {
		THIS.notifyAll();
		sleep(1);
		THIS.wait();
	}
	THIS.notifyAll();
}

RefHandler::~RefHandler() {
	release();
}

void RefHandler::push(HandlerCallback* runner) {
	uint32_t count = AtomicGet(&(THIS.count));
	HandlerCallback** queue = resize(count+1);
	AtomicSetPtr(&(queue[count]), runner);
}

void RefHandler::remove(HandlerCallback* runner) {
	uint32_t count = AtomicGet(&(THIS.count));
	HandlerCallback** queue = (HandlerCallback**)AtomicGetPtr((void**)&(THIS.queue));
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

struct RefHandler_threadData {
	Handler* handler;
	void* runner;
};

void* RefHandler_threadPost(void* data) {
	struct RefHandler_threadData* threadData = (struct RefHandler_threadData*)data;
	RefHandler::onthread(threadData->handler, threadData->runner);
	mmfree(threadData);

	return NULL;
}

void* RefHandler_threadRun(void* data) {
	struct RefHandler_threadData* threadData = (struct RefHandler_threadData*)data;
	RefHandler::onrun(threadData->handler, threadData->runner);
	mmfree(threadData);

	return NULL;
}

void* RefHandler::postDelayed(Callback callback, int delay, const Object& userData) {
	THIS.wait();
	if (AtomicGet(&shutdown) == 0) {
		HandlerCallback* runner = memNew(runner, HandlerCallback(callback, delay, userData));
		push(runner);
		Handler* handler = new Handler(this);

//		LOG("DETACH %08X", (int)(intptr_t)runner);
		struct RefHandler_threadData *threadData = (struct RefHandler_threadData*)mmalloc(sizeof(struct RefHandler_threadData));
		if (threadData == NULL)
			throw eOutOfMemory;
		threadData->handler = handler;
		threadData->runner = runner;
		NewThreadAsync(RefHandler_threadPost, NULL, threadData);

		THIS.notifyAll();
		return runner;
	}
	THIS.notifyAll();
	return NULL;
}

void* RefHandler::post(Callback callback, const Object& userData) {
	THIS.wait();
	if (AtomicGet(&shutdown) == 0) {
		HandlerCallback* runner = memNew(runner, HandlerCallback(callback, 0, userData));
		push(runner);
		Handler* handler = new Handler(this);
		
//		LOG("DETACH %08X", (int)(intptr_t)runner);
		struct RefHandler_threadData *threadData = (struct RefHandler_threadData*)mmalloc(sizeof(struct RefHandler_threadData));
		if (threadData == NULL)
			throw eOutOfMemory;
		threadData->handler = handler;
		threadData->runner = runner;
		NewThreadAsync(RefHandler_threadPost, NULL, threadData);

		THIS.notifyAll();
		return runner;
	}
	THIS.notifyAll();
	return NULL;
}

void RefHandler::removeCallbacks(Callback callback) {
	THIS.wait();
	uint32_t count = AtomicGet(&(THIS.count));
	HandlerCallback** queue = (HandlerCallback**)AtomicGetPtr((void**)&(THIS.queue));
	for (uint32_t i = 0; i < count; i++) {
		HandlerCallback* runner = AtomicGetPtr(&(queue[i]));
		if (AtomicGetPtr(&(runner->callback)) == callback)
			AtomicSet(&(runner->shutdown), 1);
	}
	THIS.notifyAll();
}

void RefHandler::removeRunner(void* runnerid) {
	THIS.wait();
	HandlerCallback* runner = (HandlerCallback*)runnerid;
	uint32_t count = AtomicGet(&(THIS.count));
	HandlerCallback** queue = (HandlerCallback**)AtomicGetPtr((void**)&(THIS.queue));
	for (uint32_t i = 0; i < count; i++) {
		if (AtomicGetPtr(&(queue[i])) == runner)
			AtomicSet(&(runner->shutdown), 1);
	}
	THIS.notifyAll();
}

void RefHandler::onthread(Handler* handler, void* runnerid) {
	HandlerCallback* runner = (HandlerCallback*)runnerid;

	int32_t delay = AtomicGet(&(runner->delay));
	systemSleep(delay);

	if (AtomicGet(&(runner->shutdown)) == 0) {
//		LOG("GOMAIN %08X", (int)(intptr_t)runner);
		struct RefHandler_threadData *threadData = (struct RefHandler_threadData*)mmalloc(sizeof(struct RefHandler_threadData));
		if (threadData == NULL)
			throw eOutOfMemory;
		threadData->handler = handler;
		threadData->runner = runner;
		
		MainThreadAsync(RefHandler_threadRun, NULL, threadData);
	} else {
		__sync_synchronize();
		
//		LOG("REMOVE %08X", (int)(intptr_t)runner);
		(*handler).wait();
		(*handler).ref().remove(runner);
		memDelete(runner);
		(*handler).notifyAll();
		delete handler;
//		LOG("FINISH %08X", (int)(intptr_t)runner);
	}
}

void RefHandler::onrun(Handler* handler, void* runnerid) {
	HandlerCallback* runner = (HandlerCallback*)runnerid;

	__sync_synchronize();
	
//	LOG("REMOVE %08X", (int)(intptr_t)runner);
	runner->callback(runner->userData);
	(*handler).wait();
	(*handler).ref().remove(runner);
	memDelete(runner);
	(*handler).notifyAll();
	delete handler;
//	LOG("FINISH %08X", (int)(intptr_t)runner);
}
