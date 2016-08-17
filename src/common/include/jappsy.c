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

#include "jappsy.h"
#include <core/uMemory.h>
#include <core/uSystem.h>

#if defined(__JNI__)
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
            msg.callback(msg.data);
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

    static void* Jappsy_testMainThreadMessageCallback(void* data) {
        LOG("Running Testing Message: %s", (char*)data);

        return NULL;
    }

    static void* Jappsy_testMainThreadMessageLooper(void* data) {
        LOG("Running Testing Thread: %s", (char*)data);
        struct ThreadMessage msg;
        msg.callback = Jappsy_testMainThreadMessageCallback;
        msg.data = data;

        postMainThreadMessage(&msg);
        return NULL;
    }
#endif

void jappsyInit() {
    mmInit();
    uSystemInit();

#if defined(__JNI__)
    setupMainThreadMessageLooper();

    LOG("Start Testing Thread");
    pthread_t thread;
    const char* data = "TestUserData";
    pthread_create(&thread, NULL, Jappsy_testMainThreadMessageLooper, (void*)data);
#endif
}

void jappsyQuit() {
#ifdef DEBUG
	memLogSort();
    memLogStats(NULL, NULL, NULL, NULL);
#endif

    uSystemQuit();
    mmQuit();
}