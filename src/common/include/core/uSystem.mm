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

#include "uSystem.h"
#include <core/uAtomic.h>
#include <core/uMemory.h>

void* CurrentThreadId() {
#if defined(__IOS__)
	return (__bridge void *)([NSThread currentThread]);
#elif defined(__JNI__)
	return (void*)((intptr_t)(pthread_self()));
#elif defined(__WINNT__)
	return (void*)((intptr_r)(GetCurrentThreadId()));
#else
	#error Unsupported platform!
#endif
}

static void* MainThreadId = NULL;

struct ThreadSyncData {
	volatile jlock syncLock;
	volatile void* resultData;
	volatile const char* errorData;
};

struct ThreadMessage {
	volatile ThreadRunCallback runCallback;
	volatile ThreadResultCallback resultCallback;
	volatile void* userData;

	volatile jbool syncCall;

	volatile struct ThreadSyncData* syncData;
};

void* NewThreadRun(void* data) {
	struct ThreadMessage* msg = (struct ThreadMessage*)data;
	volatile struct ThreadSyncData* syncData = AtomicGetPtr(&(msg->syncData));
	
	ThreadRunCallback runCallback = AtomicGetPtr(&(msg->runCallback));
	volatile void *userData = AtomicGetPtr(&(msg->userData));
	void *result = NULL;
	
	try {
		__sync_synchronize();

		result = runCallback((void*)userData);
	} catch (const char* error) {
		if (AtomicGet(&(msg->syncCall))) {
			AtomicSetPtr(&(syncData->errorData), (char*)error);
			AtomicUnlock(&(syncData->syncLock));
			return NULL;
		} else {
			memFree(msg);
			throw;
		}
	} catch (...) {
		if (AtomicGet(&(msg->syncCall))) {
			AtomicSetPtr(&(syncData->errorData), (char*)eUnknown);
			AtomicUnlock(&(syncData->syncLock));
			return NULL;
		} else {
			memFree(msg);
			throw;
		}
	}
	
	if (AtomicGet(&(msg->syncCall))) {
		AtomicSetPtr(&(syncData->resultData), result);
		AtomicUnlock(&(syncData->syncLock));
	} else {
		ThreadResultCallback resultCallback = AtomicGetPtr(&(msg->resultCallback));
		memFree(msg);
		if (resultCallback != NULL) {
			try {
				resultCallback((void*)userData, result);
			} catch (...) {
				throw;
			}
		}
	}
	
	return NULL;
}

#if defined(__IOS__)

	@interface JappsyThreadHandler : NSObject

	+(void)ThreadRunner:(NSValue *)threadData;

	@end

	@implementation JappsyThreadHandler

	+(void)ThreadRunner:(NSValue *)threadData {
		@autoreleasepool {
			struct ThreadMessage* msg = (struct ThreadMessage*)([threadData pointerValue]);
			(void)NewThreadRun(msg);
		}
	}

	@end

#elif defined(__JNI__)
	#include <asm/fcntl.h>
	#include <unistd.h>
	#include <android/looper.h>
	#include <pthread.h>

	static int mainThreadMessagePipe[2];

	static const int LOOPER_ID_MESSAGEPIPE = 'JPSY';

	void postMainThreadMessage(struct ThreadMessage* msg) {
		if (write(mainThreadMessagePipe[1], msg, sizeof(msg)) != sizeof(msg)) {
			LOG("Thread message buffer overrun!")
		}
	}

	static int mainThreadMessagePipeCallback(int fd, int events, void *data) {
		struct ThreadMessage msg;
		
		while (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
			LOG("Got message!");

			volatile struct ThreadSyncData* syncData = AtomicGetPtr(&(msg.syncData));

			ThreadRunCallback runCallback = AtomicGetPtr(&(msg.runCallback));
			volatile void *userData = AtomicGetPtr(&(msg.userData));
			void *result = NULL;

			try {
				result = runCallback((void*)userData);
			} catch (const char* error) {
				if (AtomicGet(&(msg.syncCall))) {
					AtomicSetPtr(&(syncData->errorData), (char*)error);
					AtomicUnlock(&(syncData->syncLock));
					return 1;
				} else {
					throw;
				}
			} catch (...) {
				if (AtomicGet(&(msg.syncCall))) {
					AtomicSetPtr(&(syncData->errorData), (char*)eUnknown);
					AtomicUnlock(&(syncData->syncLock));
					return 1;
				} else {
					throw;
				}
			}

			if (AtomicGet(&(msg.syncCall))) {
				AtomicSetPtr(&(syncData->resultData), result);
				AtomicUnlock(&(syncData->syncLock));
			} else {
				ThreadResultCallback resultCallback = AtomicGetPtr(&(msg.resultCallback));
				if (resultCallback != NULL) {
					try {
						resultCallback((void*)userData, result);
					} catch (...) {
						throw;
					}
				}
			}
		}
	
		return 1;
	}

	static int setupMainThreadMessageLooper() {
		int err, opt, i;
		err = pipe(mainThreadMessagePipe);
		if (err == -1) {
			LOG("Create message pipe failed!");
			return -1;
		}
		for (i = 0; i < 2; i++) {
			opt = fcntl(mainThreadMessagePipe[i], F_GETFL);
			if ((opt & (O_CLOEXEC | O_NONBLOCK)) != (O_CLOEXEC | O_NONBLOCK)) {
				opt |= O_CLOEXEC | O_NONBLOCK;
				fcntl(mainThreadMessagePipe[i], F_SETFL, opt);
			}
		}
		
		ALooper* mainLooper = ALooper_forThread();
		if (mainLooper == NULL) {
			LOG("Main thread looper not found!")
			return -1;
		}
	
		ALooper_addFd(mainLooper, mainThreadMessagePipe[0], LOOPER_ID_MESSAGEPIPE, ALOOPER_EVENT_INPUT, mainThreadMessagePipeCallback, NULL);
	}
#endif

void* MainThreadSync(ThreadRunCallback callback, void* userData) throw(const char*) {
	if (CurrentThreadId() == MainThreadId) {
		return callback(userData);
	} else {
		struct ThreadMessage* msg = memAlloc(struct ThreadMessage, msg, sizeof(struct ThreadMessage));
		struct ThreadSyncData* syncData = memAlloc(struct ThreadSyncData, syncData, sizeof(struct ThreadSyncData));
		if ((msg == NULL) || (syncData == NULL)) {
			if (msg != NULL) {
				memFree(msg);
			}
			if (syncData != NULL) {
				memFree(syncData);
			}
			throw eOutOfMemory;
		}

		AtomicSetPtr(&(msg->runCallback), (void*)callback);
		AtomicSetPtr(&(msg->resultCallback), NULL);
		AtomicSetPtr(&(msg->userData), userData);
		AtomicSet(&(msg->syncCall), true);
		AtomicSetPtr(&(msg->syncData), syncData);

		AtomicSet(&(syncData->syncLock), false);
		AtomicSetPtr(&(syncData->resultData), NULL);
		AtomicSetPtr(&(syncData->errorData), NULL);

		AtomicLock(&(syncData->syncLock));
#if defined(__IOS__)
		[[JappsyThreadHandler class] performSelectorOnMainThread:@selector(ThreadRunner:) withObject:[NSValue valueWithPointer:msg] waitUntilDone:YES];
#elif defined(__JNI__)
		postMainThreadMessage(msg);
		memFree(msg);
#else
		#error Unsupported platform!
#endif
		AtomicLock(&(syncData->syncLock));

		volatile const char* throwError = AtomicGetPtr(&(syncData->errorData));
		if (throwError != NULL) {
			memFree(syncData);
			throw (const char*)throwError;
		}

		volatile void* result = AtomicGetPtr(&(syncData->resultData));
		memFree(syncData);

		__sync_synchronize();

		return (void*)result;
	}
}

void MainThreadAsync(ThreadRunCallback runCallback, ThreadResultCallback resultCallback, void* userData) throw(const char*) {
	if (CurrentThreadId() == MainThreadId) {
		void* result = runCallback(userData);
		if (resultCallback != NULL)
			resultCallback(userData, result);
	} else {
		struct ThreadMessage* msg = memAlloc(struct ThreadMessage, msg, sizeof(struct ThreadMessage));
		if (msg == NULL)
			throw eOutOfMemory;
		
		AtomicSetPtr(&(msg->runCallback), (void*)runCallback);
		AtomicSetPtr(&(msg->resultCallback), (void*)resultCallback);
		AtomicSetPtr(&(msg->userData), userData);
		AtomicSet(&(msg->syncCall), false);
		AtomicSetPtr(&(msg->syncData), NULL);

#if defined(__IOS__)
		[[JappsyThreadHandler class] performSelectorOnMainThread:@selector(ThreadRunner:) withObject:[NSValue valueWithPointer:msg] waitUntilDone:NO];
#elif defined(__JNI__)
		postMainThreadMessage(msg);
		memFree(msg);
#else
		#error Unsupported platform!
#endif
	}
}

void NewThreadAsync(ThreadRunCallback runCallback, ThreadResultCallback resultCallback, void* userData) throw(const char*) {
	struct ThreadMessage* msg = memAlloc(struct ThreadMessage, msg, sizeof(struct ThreadMessage));
	if (msg == NULL)
		throw eOutOfMemory;

	AtomicSetPtr(&(msg->runCallback), (void*)runCallback);
	AtomicSetPtr(&(msg->resultCallback), (void*)resultCallback);
	AtomicSetPtr(&(msg->userData), userData);
	AtomicSet(&(msg->syncCall), false);
	AtomicSetPtr(&(msg->syncData), NULL);

#if defined(__IOS__)
	[NSThread detachNewThreadSelector:@selector(ThreadRunner:) toTarget:[JappsyThreadHandler class] withObject:[NSValue valueWithPointer:msg]];
#elif defined(__JNI__)
	pthread_t thread;
	pthread_create(&thread, NULL, NewThreadRun, msg);
#else
	#error Unsupported platform!
#endif
}

#if defined(__IOS__)
extern "C" {
	uint64_t CNSDateNano() {
		return (uint64_t)floor([[NSDate date] timeIntervalSince1970] * 1000000000);
	}

	uint64_t CNSDateMillis() {
		return (uint64_t)floor([[NSDate date] timeIntervalSince1970] * 1000);
	}

	void CNSThreadSleep(int ms) {
		[NSThread sleepForTimeInterval:(double)ms / 1000.0];
	}
}
#endif

#if defined(__WINNT__)
    LARGE_INTEGER uSystem_Frequency;
    uint64_t uSystem_SystemTimeShiftNS;
    uint64_t uSystem_SystemTimeShiftMS;
    LARGE_INTEGER uSystem_CounterShift;
#endif

void uSystemInit() {
#if defined(__WINNT__)
    QueryPerformanceFrequency(&uSystem_Frequency);
    GetSystemTimeAsFileTime((FILETIME*)(&uSystem_SystemTimeShiftNS));
    QueryPerformanceCounter(&uSystem_CounterShift);
    uSystem_SystemTimeShiftNS -= 116444736000000000LL;
    uSystem_SystemTimeShiftMS = uSystem_SystemTimeShiftNS / 10000LL;
    uSystem_SystemTimeShiftNS *= 100LL;
#endif

	MainThreadId = CurrentThreadId();

	__sync_synchronize();

#if defined(__JNI__)
	setupMainThreadMessageLooper();
#endif
}

void uSystemQuit() {

}
