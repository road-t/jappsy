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

#include "uMixer.h"

#if defined(__JNI__)

#include <sound/uOpenSLContext.h>

#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__JNI__)

static OpenSLContext context;

#endif

void initAudioPlayer() {
#if defined(__JNI__)
	LOG("OpenSL: initAudioPlayer");

	context.init();
#else
	#error Unsupported platform!
#endif
}

bool shutdownAudioPlayer() {
#if defined(__JNI__)
	LOG("OpenSL: shutdownAudioPlayer");

	context.shutdown();
#else
	#error Unsupported platform!
#endif
	return false;
}

void pauseAudioPlayer() {
#if defined(__JNI__)
	#warning TODO!
	LOG("TODO: pauseAudioPlayer");
#else
	#error Unsupported platform!
#endif
}

void resumeAudioPlayer() {
#if defined(__JNI__)
	#warning TODO!
	LOG("TODO: resumeAudioPlayer");
#else
	#error Unsupported platform!
#endif
}

bool prepareAudio(const CString& filePath, void* userData, bool threaded, AudioReadyCallback onAudioReady, AudioErrorCallback onAudioError) {
#if defined(__JNI__)
	#warning TODO!
	LOG("TODO: prepareAudio");
#else
	#error Unsupported platform!
#endif
	return false;
}

void destroyAudio(void* audioHandle) {
#if defined(__JNI__)
	#warning TODO!
	LOG("TODO: destroyAudio");
#else
	#error Unsupported platform!
#endif
}

void playAudio(void* audioHandle, bool loop) {
#if defined(__JNI__)
	#warning TODO!
	LOG("TODO: playAudio");
#else
	#error Unsupported platform!
#endif
}

bool playingAudio(void* audioHandle) {
#if defined(__JNI__)
	#warning TODO!
	LOG("TODO: playingAudio");
#else
	#error Unsupported platform!
#endif
	return false;
}

void volumeAudio(void* audioHandle, float volume) {
#if defined(__JNI__)
	#warning TODO!
	LOG("TODO: volumeAudio");
#else
	#error Unsupported platform!
#endif
}

bool pauseAudio(void* audioHandle) {
#if defined(__JNI__)
	#warning TODO!
	LOG("TODO: pauseAudio");
#else
	#error Unsupported platform!
#endif
	return false;
}

void stopAudio(void* audioHandle) {
#if defined(__JNI__)
	#warning TODO!
	LOG("TODO: stopAudio");
#else
	#error Unsupported platform!
#endif
}

int32_t freeChannels() {
#if defined(__JNI__)
	#warning TODO!
	LOG("TODO: freeChannels");
#else
	#error Unsupported platform!
#endif
	return 0;
}

int32_t activeChannels() {
#if defined(__JNI__)
	#warning TODO!
	LOG("TODO: activeChannels");
#else
	#error Unsupported platform!
#endif
	return 0;
}

#ifdef __cplusplus
}
#endif
