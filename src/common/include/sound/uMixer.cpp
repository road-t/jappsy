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
#include <sound/uOpenSLSound.h>
#include <core/uSystem.h>

#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__JNI__)

static OpenSLContext* context = NULL;
static jlock context_lock = false;

#endif

void initAudioPlayer() {
#if defined(__JNI__)
	//LOG("OpenSL: initAudioPlayer");

	AtomicLock(&context_lock);
	if (context == NULL) {
		try {
			AtomicSetPtr(&context, new OpenSLContext());
		} catch (...) {
		}
	}
	if (context != NULL) {
		context->init();
	}

	AtomicUnlock(&context_lock);
#else
	#error Unsupported platform!
#endif
}

bool shutdownAudioPlayer() {
#if defined(__JNI__)
	//LOG("OpenSL: shutdownAudioPlayer");

	AtomicLock(&context_lock);
	OpenSLContext* ctx = AtomicGetPtr(&context);
	if (ctx != NULL) {
		ctx->shutdown();

		delete ctx;

		AtomicSetPtr(&context, NULL);
	}
	AtomicUnlock(&context_lock);
#else
	#error Unsupported platform!
#endif
	return false;
}

void pauseAudioPlayer() {
#if defined(__JNI__)
	//LOG("OpenSL: pauseAudioPlayer");

	AtomicLock(&context_lock);
	OpenSLContext* ctx = AtomicGetPtr(&context);
	if (ctx != NULL) {
		ctx->pause();
	}
	AtomicUnlock(&context_lock);
#else
	#error Unsupported platform!
#endif
}

void resumeAudioPlayer() {
#if defined(__JNI__)
	//LOG("OpenSL: resumeAudioPlayer");

	AtomicLock(&context_lock);
	OpenSLContext* ctx = AtomicGetPtr(&context);
	if (ctx != NULL) {
		ctx->resume();
	}
	AtomicUnlock(&context_lock);
#else
	#error Unsupported platform!
#endif
}

bool prepareAudio(const CString& filePath, void* userData, bool threaded, AudioReadyCallback onAudioReady, AudioErrorCallback onAudioError) {
#if defined(__JNI__)
	//LOG("OpenSL: prepareAudio");

	OpenSLSound* sound = new OpenSLSound();
	if (sound->load(filePath)) {
		onAudioReady(sound, 0, 0, userData);
	} else {
		delete sound;
		onAudioError(userData);
	}
#else
	#error Unsupported platform!
#endif
	return true;
}

void destroyAudio(void* audioHandle) {
#if defined(__JNI__)
	//LOG("OpenSL: destroyAudio");

	OpenSLSound* sound = (OpenSLSound*)audioHandle;
	delete sound;
#else
	#error Unsupported platform!
#endif
}

void playAudio(void* audioHandle, bool loop) {
#if defined(__JNI__)
	//LOG("OpenSL: playAudio");

	AtomicLock(&context_lock);
	OpenSLContext* ctx = AtomicGetPtr(&context);
	if (ctx != NULL) {
		OpenSLSound* sound = (OpenSLSound*)audioHandle;
		sound->play(ctx, loop);
	}
	AtomicUnlock(&context_lock);
#else
	#error Unsupported platform!
#endif
}

bool playingAudio(void* audioHandle) {
#if defined(__JNI__)
	//LOG("OpenSL: playingAudio");

	OpenSLSound* sound = (OpenSLSound*)audioHandle;
	return sound->isPlaying();
#else
	#error Unsupported platform!
#endif
	return false;
}

void volumeAudio(void* audioHandle, float volume) {
#if defined(__JNI__)
	//LOG("OpenSL: volumeAudio");

	OpenSLSound* sound = (OpenSLSound*)audioHandle;
	sound->setVolume(volume);
#else
	#error Unsupported platform!
#endif
}

bool pauseAudio(void* audioHandle) {
#if defined(__JNI__)
	//LOG("OpenSL: pauseAudio");

	OpenSLSound* sound = (OpenSLSound*)audioHandle;
	sound->pause();
#else
	#error Unsupported platform!
#endif
	return false;
}

void stopAudio(void* audioHandle) {
#if defined(__JNI__)
	//LOG("OpenSL: stopAudio");

	OpenSLSound* sound = (OpenSLSound*)audioHandle;
	sound->stop();
#else
	#error Unsupported platform!
#endif
}

int32_t freeChannels() {
#if defined(__JNI__)
	// Unused on Android
#else
	#error Unsupported platform!
#endif
	return 0;
}

int32_t activeChannels() {
#if defined(__JNI__)
	// Unused on Android
#else
	#error Unsupported platform!
#endif
	return 0;
}

#ifdef __cplusplus
}
#endif
