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
#include <opengl/uOpenGL.h>
#include <net/uHTTPClient.h>
#include <event/uMotionEvent.h>

#ifdef __cplusplus
extern "C" {
#endif

void *CurrentThreadId() {
#if defined(__IOS__)
	return (__bridge void *)([NSThread currentThread]);
#elif defined(__JNI__)
	return (void *) ((intptr_t) (pthread_self()));
#elif defined(__WINNT__)
	return (void*)((intptr_r)(GetCurrentThreadId()));
#endif
}

static void *MainThreadId = NULL;
static void *OpenGLThreadId = NULL;

bool IsMainThread() {
	return MainThreadId == CurrentThreadId();
}

bool IsOpenGLThread() {
	return AtomicGet(&OpenGLThreadId) == CurrentThreadId();
}

struct ThreadSyncData {
	volatile jlock syncLock;
	volatile void *resultData;
	volatile const char *errorData;
};

struct ThreadMessage {
	volatile ThreadRunCallback runCallback;
	volatile ThreadResultCallback resultCallback;
	volatile void *userData;

	volatile jbool syncCall;

	volatile struct ThreadSyncData *syncData;
};

void *NewThreadRun(void *data) {
	struct ThreadMessage *msg = (struct ThreadMessage *) data;
	volatile struct ThreadSyncData *syncData = AtomicGetPtr(&(msg->syncData));

	ThreadRunCallback runCallback = AtomicGetPtr(&(msg->runCallback));
	volatile void *userData = AtomicGetPtr(&(msg->userData));
	void *result = NULL;

	try {
		__sync_synchronize();

		result = runCallback((void *) userData);
	} catch (const char *error) {
		if (AtomicGet(&(msg->syncCall))) {
			AtomicSetPtr(&(syncData->errorData), (char *) error);
			AtomicUnlock(&(syncData->syncLock));
			return NULL;
		} else {
			memFree(msg);
			throw;
		}
	} catch (...) {
		if (AtomicGet(&(msg->syncCall))) {
			AtomicSetPtr(&(syncData->errorData), (char *) eUnknown);
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
				resultCallback((void *) userData, result);
			} catch (...) {
				throw;
			}
		}
	}

#if defined(__JNI__)
	pthread_exit(NULL);
#endif
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

static JavaVM *javaVM = NULL;
static JNIEnv *mainEnv = NULL;

__thread JNIEnv *threadEnv = NULL;
__thread int32_t threadEnvCounter = 0;

JNIEnv *GetThreadEnv() {
	if (CurrentThreadId() == MainThreadId) {
		return mainEnv;
	} else {
		if (threadEnvCounter == 0) {
			javaVM->AttachCurrentThread(&threadEnv, NULL);
		}
		threadEnvCounter++;
		return threadEnv;
	}
}

void ReleaseThreadEnv() {
	if (CurrentThreadId() != MainThreadId) {
		if (threadEnvCounter > 0) {
			threadEnvCounter--;
			if (threadEnvCounter == 0) {
				javaVM->DetachCurrentThread();
				threadEnv = NULL;
			}
		}
	}
}

static int mainThreadMessagePipe[2];
static int openglThreadMessagePipe[2];
volatile uint32_t openglThreadMessagePipeEvents = 0;

static const int LOOPER_ID_MESSAGEPIPE = 'JPSY';

void postMainThreadMessage(struct ThreadMessage *msg) {
	if (write(mainThreadMessagePipe[1], msg, sizeof(struct ThreadMessage)) !=
		sizeof(struct ThreadMessage)) {
		LOG("Main thread message buffer overrun!")
	}
}

void postOpenGLThreadMessage(struct ThreadMessage *msg) {
	if (write(openglThreadMessagePipe[1], msg, sizeof(struct ThreadMessage)) !=
		sizeof(struct ThreadMessage)) {
		LOG("OpenGL thread message buffer overrun!")
	} else {
		AtomicIncrement(&openglThreadMessagePipeEvents);
	}
}

static int threadMessagePipeCallback(int fd, int events, void *data) {
	struct ThreadMessage msg;

	LOG("MessageLoop(%d): Read Messages (Start)", fd);

	while (read(fd, &msg, sizeof(struct ThreadMessage)) == sizeof(struct ThreadMessage)) {
		if (fd == openglThreadMessagePipe[0]) {
			AtomicDecrement(&openglThreadMessagePipeEvents);
		}
		//LOG("(%d) Got message!", fd);

		volatile struct ThreadSyncData *syncData = AtomicGetPtr(&(msg.syncData));

		ThreadRunCallback runCallback = AtomicGetPtr(&(msg.runCallback));
		volatile void *userData = AtomicGetPtr(&(msg.userData));
		void *result = NULL;

		try {
			result = runCallback((void *) userData);
		} catch (const char *error) {
			if (AtomicGet(&(msg.syncCall))) {
				AtomicSetPtr(&(syncData->errorData), (char *) error);
				AtomicUnlock(&(syncData->syncLock));

				LOG("MessageLoop(%d): Read Messages (Error)", fd);

				return 1;
			} else {
				LOG("MessageLoop(%d): Read Messages (Error)", fd);

				throw;
			}
		} catch (...) {
			if (AtomicGet(&(msg.syncCall))) {
				AtomicSetPtr(&(syncData->errorData), (char *) eUnknown);
				AtomicUnlock(&(syncData->syncLock));
				LOG("MessageLoop(%d): Read Messages (Error)", fd);

				return 1;
			} else {
				LOG("MessageLoop(%d): Read Messages (Error)", fd);

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
					resultCallback((void *) userData, result);
				} catch (...) {
					LOG("MessageLoop(%d): Read Messages (Error)", fd);

					throw;
				}
			}
		}
	}

	LOG("MessageLoop(%d): Read Messages (Done)", fd);

	return 1;
}

static int setupThreadMessageLooper() {
	int err, opt, i;
	err = pipe(mainThreadMessagePipe);
	if (err == -1) {
		LOG("Main thread create message pipe failed!");
		return -1;
	} else {
		LOG("Main thread message pipe (%d, %d)", mainThreadMessagePipe[0], mainThreadMessagePipe[1]);
	}
	err = pipe(openglThreadMessagePipe);
	if (err == -1) {
		LOG("OpenGL thread create message pipe failed!");
		return -1;
	} else {
		LOG("OpenGL thread message pipe (%d, %d)", openglThreadMessagePipe[0], openglThreadMessagePipe[1]);
	}
	for (i = 0; i < 2; i++) {
		opt = fcntl(mainThreadMessagePipe[i], F_GETFL);
		if ((opt & (O_CLOEXEC | O_NONBLOCK)) != (O_CLOEXEC | O_NONBLOCK)) {
			opt |= O_CLOEXEC | O_NONBLOCK;
			fcntl(mainThreadMessagePipe[i], F_SETFL, opt);
		}

		opt = fcntl(openglThreadMessagePipe[i], F_GETFL);
		if ((opt & (O_CLOEXEC | O_NONBLOCK)) != (O_CLOEXEC | O_NONBLOCK)) {
			opt |= O_CLOEXEC | O_NONBLOCK;
			fcntl(openglThreadMessagePipe[i], F_SETFL, opt);
		}
	}

	ALooper *mainLooper = ALooper_forThread();
	if (mainLooper == NULL) {
		LOG("Main thread looper not found!")
		return -1;
	}

	ALooper_addFd(mainLooper, mainThreadMessagePipe[0], LOOPER_ID_MESSAGEPIPE, ALOOPER_EVENT_INPUT,
				  threadMessagePipeCallback, NULL);

	return 0;
}

void OpenGLThreadMessageLooper() {
	if (AtomicGet(&openglThreadMessagePipeEvents) != 0) {
		threadMessagePipeCallback(openglThreadMessagePipe[0], 0, NULL);
	}
}

#endif

void *MainThreadSync(ThreadRunCallback callback, void *userData) throw(const char*) {
	if (CurrentThreadId() == MainThreadId) {
		return callback(userData);
	} else {
		struct ThreadMessage *msg = memAlloc(struct ThreadMessage, msg,
											 sizeof(struct ThreadMessage));
		struct ThreadSyncData *syncData = memAlloc(struct ThreadSyncData, syncData,
												   sizeof(struct ThreadSyncData));
		if ((msg == NULL) || (syncData == NULL)) {
			if (msg != NULL) {
				memFree(msg);
			}
			if (syncData != NULL) {
				memFree(syncData);
			}
			throw eOutOfMemory;
		}

		AtomicSetPtr(&(msg->runCallback), callback);
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
#else
#error Unsupported platform!
#endif
		AtomicLock(&(syncData->syncLock));
		memFree(msg);

		volatile const char *throwError = AtomicGetPtr(&(syncData->errorData));
		if (throwError != NULL) {
			memFree(syncData);
			throw (const char *) throwError;
		}

		volatile void *result = AtomicGetPtr(&(syncData->resultData));
		memFree(syncData);

		__sync_synchronize();

		return (void *) result;
	}
}

void MainThreadAsync(ThreadRunCallback runCallback, ThreadResultCallback resultCallback,
					 void *userData) throw(const char*) {
	if (CurrentThreadId() == MainThreadId) {
		void *result = runCallback(userData);
		if (resultCallback != NULL)
			resultCallback(userData, result);
	} else {
		struct ThreadMessage *msg = memAlloc(struct ThreadMessage, msg,
											 sizeof(struct ThreadMessage));
		if (msg == NULL)
			throw eOutOfMemory;

		AtomicSetPtr(&(msg->runCallback), runCallback);
		AtomicSetPtr(&(msg->resultCallback), resultCallback);
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

void *OpenGLThreadSync(ThreadRunCallback callback, void *userData) throw(const char*) {
#if defined(__IOS__)
	if (CurrentThreadId() == MainThreadId) {
		return callback(userData);
	} else {
		return MainThreadSync(callback, userData);
	}
#elif defined(__JNI__)
	if (IsOpenGLThread()) {
		return callback(userData);
	} else {
		struct ThreadMessage *msg = memAlloc(struct ThreadMessage, msg,
											 sizeof(struct ThreadMessage));
		struct ThreadSyncData *syncData = memAlloc(struct ThreadSyncData, syncData,
												   sizeof(struct ThreadSyncData));
		if ((msg == NULL) || (syncData == NULL)) {
			if (msg != NULL) {
				memFree(msg);
			}
			if (syncData != NULL) {
				memFree(syncData);
			}
			throw eOutOfMemory;
		}

		AtomicSetPtr(&(msg->runCallback), callback);
		AtomicSetPtr(&(msg->resultCallback), NULL);
		AtomicSetPtr(&(msg->userData), userData);
		AtomicSet(&(msg->syncCall), true);
		AtomicSetPtr(&(msg->syncData), syncData);

		AtomicSet(&(syncData->syncLock), false);
		AtomicSetPtr(&(syncData->resultData), NULL);
		AtomicSetPtr(&(syncData->errorData), NULL);

		//LOG("OpenGL Thread Message (Start)");
		AtomicLock(&(syncData->syncLock));
		postOpenGLThreadMessage(msg);
		//LOG("OpenGL Thread Message (Sent)");
		AtomicLock(&(syncData->syncLock));
		//LOG("OpenGL Thread Message (Done)");
		memFree(msg);

		volatile const char *throwError = AtomicGetPtr(&(syncData->errorData));
		if (throwError != NULL) {
			memFree(syncData);
			throw (const char *) throwError;
		}

		volatile void *result = AtomicGetPtr(&(syncData->resultData));
		memFree(syncData);

		__sync_synchronize();

		return (void *) result;
	}
#else
	#error Unsupported platform!
#endif
}

void OpenGLThreadAsync(ThreadRunCallback runCallback, ThreadResultCallback resultCallback,
					   void *userData) throw(const char*) {
#if defined(__IOS__)
	if (CurrentThreadId() == MainThreadId) {
		void *result = runCallback(userData);
		if (resultCallback != NULL)
			resultCallback(userData, result);
	} else {
		MainThreadAsync(runCallback, resultCallback, userData);
	}
#elif defined(__JNI__)
	if (IsOpenGLThread()) {
		void *result = runCallback(userData);
		if (resultCallback != NULL)
			resultCallback(userData, result);
	} else {
		struct ThreadMessage *msg = memAlloc(struct ThreadMessage, msg,
											 sizeof(struct ThreadMessage));
		if (msg == NULL)
			throw eOutOfMemory;

		AtomicSetPtr(&(msg->runCallback), runCallback);
		AtomicSetPtr(&(msg->resultCallback), resultCallback);
		AtomicSetPtr(&(msg->userData), userData);
		AtomicSet(&(msg->syncCall), false);
		AtomicSetPtr(&(msg->syncData), NULL);

		postOpenGLThreadMessage(msg);
		memFree(msg);
	}
#else
	#error Unsupported platform!
#endif
}

void NewThreadAsync(ThreadRunCallback runCallback, ThreadResultCallback resultCallback,
					void *userData) throw(const char*) {
	struct ThreadMessage *msg = memAlloc(struct ThreadMessage, msg, sizeof(struct ThreadMessage));
	if (msg == NULL)
		throw eOutOfMemory;

	AtomicSetPtr(&(msg->runCallback), runCallback);
	AtomicSetPtr(&(msg->resultCallback), resultCallback);
	AtomicSetPtr(&(msg->userData), userData);
	AtomicSet(&(msg->syncCall), false);
	AtomicSetPtr(&(msg->syncData), NULL);

#if defined(__IOS__)
	[NSThread detachNewThreadSelector:@selector(ThreadRunner:) toTarget:[JappsyThreadHandler class] withObject:[NSValue valueWithPointer:msg]];
#elif defined(__JNI__)
	pthread_t thread;
	pthread_create(&thread, NULL, NewThreadRun, msg);
	pthread_detach(thread);
#else
#error Unsupported platform!
#endif
}

#if defined(__IOS__)

uint64_t CNSDateNano() {
	return (uint64_t)floor([[NSDate date] timeIntervalSince1970] * 1000000000);
}

uint64_t CNSDateMillis() {
	return (uint64_t)floor([[NSDate date] timeIntervalSince1970] * 1000);
}

void CNSThreadSleep(int ms) {
	[NSThread sleepForTimeInterval:(double)ms / 1000.0];
}

#endif

#if defined(__WINNT__)
	LARGE_INTEGER uSystem_Frequency;
	uint64_t uSystem_SystemTimeShiftNS;
	uint64_t uSystem_SystemTimeShiftMS;
	LARGE_INTEGER uSystem_CounterShift;
#endif

void initOpenGLThreadId() {
	AtomicSet(&OpenGLThreadId, CurrentThreadId());
}

void releaseOpenGLThreadId() {
	AtomicSet(&OpenGLThreadId, NULL);
}

void uSystemInit(void *system) {
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
	mainEnv = (JNIEnv *) system;
	mainEnv->GetJavaVM(&javaVM);

	setupThreadMessageLooper();
#endif

	uHTTPClientInit();
	uMotionEventInit();
}

void uSystemQuit() {
	uMotionEventQuit();
	uHTTPClientQuit();
}

#ifdef __cplusplus
}
#endif
