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

#include "uOpenSLPlayer.h"
#include <core/uAtomic.h>
#include <sound/uOpenSLSound.h>

bool OpenSLPlayer::isPlaying() {
	return state() == SL_PLAYSTATE_PLAYING;
}

bool OpenSLPlayer::isPaused() {
	return state() == SL_PLAYSTATE_PAUSED;
}

bool OpenSLPlayer::isStopped() {
	return state() == SL_PLAYSTATE_STOPPED;
}

void OpenSLPlayer::EventCallback(SLPlayItf caller, void* userData, SLuint32 event) {
	OpenSLPlayer* player = (OpenSLPlayer*)userData;
	if (event == SL_PLAYEVENT_HEADATEND) { // Stop
		player->lock();

		AtomicSet(&player->buffersUsed, 0);

		OpenSLSound* sound = AtomicGetPtr(&player->sound);
		if (sound != NULL) {
			sound->reset();
		}

		player->unlock();
	}

	//LOG("OpenSL: PlayerEvent %d", event);
}

void OpenSLPlayer::BufferCallback(SLBufferQueueItf caller, void* userData) {
	OpenSLPlayer* player = (OpenSLPlayer*)userData;
	int used = AtomicDecrement(&player->buffersUsed) - 1;

	//LOG("OpenSL: BufferEvent (%d)", used);

	player->queue();
}

OpenSLPlayer::OpenSLPlayer(OpenSLContext* context) {
	AtomicSet(&(this->context), context);

	SLresult result;

	locatorBufferQueue.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
	locatorBufferQueue.numBuffers = 16;

	formatPCM.formatType = SL_DATAFORMAT_PCM;
	formatPCM.numChannels = 2;
	formatPCM.samplesPerSec = SL_SAMPLINGRATE_48;
	formatPCM.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
	formatPCM.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
	formatPCM.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT ;
	formatPCM.endianness = SL_BYTEORDER_LITTLEENDIAN;

	audioSrc.pLocator = &locatorBufferQueue;
	audioSrc.pFormat = &formatPCM;
	locatorOutMix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
	locatorOutMix.outputMix = context->outputMixObj;
	audioSnk.pLocator = (void*)&locatorOutMix;
	audioSnk.pFormat = NULL;

	const SLInterfaceID ids[4] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_MUTESOLO, SL_IID_VOLUME};
	const SLboolean req[4] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

	result = (*context->engine)->CreateAudioPlayer(context->engine, &playerObj, &audioSrc, &audioSnk, 3, ids, req);
	if (result != SL_RESULT_SUCCESS) {
		playerObj = NULL;
		return;
	}

	result = (*playerObj)->Realize(playerObj, SL_BOOLEAN_FALSE);
	if (result != SL_RESULT_SUCCESS) {
		playerObj = NULL;
		return;
	}

	result = (*playerObj)->GetInterface(playerObj, SL_IID_PLAY, &player);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		return;
	}

	result = (*playerObj)->GetInterface(playerObj, SL_IID_MUTESOLO, &playerMuteSolo);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		return;
	}

	result = (*playerObj)->GetInterface(playerObj, SL_IID_VOLUME, &playerVolume);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		return;
	}

	result = (*playerObj)->GetInterface(playerObj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &bufferQueue);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		return;
	}

	result = (*player)->RegisterCallback(player, EventCallback, (void*)this);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		return;
	}

	result = (*bufferQueue)->RegisterCallback(bufferQueue, BufferCallback, (void*)this);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		return;
	}

	result = (*player)->SetCallbackEventsMask(player, SL_PLAYEVENT_HEADATEND/* | SL_PLAYEVENT_HEADATMARKER | SL_PLAYEVENT_HEADATNEWPOS | SL_PLAYEVENT_HEADMOVING | SL_PLAYEVENT_HEADSTALLED*/);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		return;
	}

	bufferSize = 4096;
	for (int i = 0; i < 4; i++) {
		buffers[i] = memAlloc(uint8_t, buffers[i], bufferSize);
		if (buffers[i] == NULL) {
			clear();
			return;
		}
	}

	//result = (*playerSeek)->SetLoop(playerSeek, sound->isLooping() ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE, 0, SL_TIME_UNKNOWN);
}

void OpenSLPlayer::clear() {
	lock();

	if (playerObj != NULL) {
		for (int i = 0; i < 4; i++) {
			if (buffers[i] != NULL) {
				memFree(buffers[i]);
				buffers[i] = NULL;
			}
		}

		(*playerObj)->Destroy(playerObj);
		player = NULL;
		playerMuteSolo = NULL;
		playerVolume = NULL;
		playerObj = NULL;
		bufferQueue = NULL;
	}

	unlock();
}

OpenSLPlayer::~OpenSLPlayer() {
	clear();
}

SLuint32 OpenSLPlayer::state() {
	SLresult result;

	if (player != NULL && bufferQueue != NULL) {
		SLBufferQueueState state;

		result = (*bufferQueue)->GetState(bufferQueue, &state);
		if (result != SL_RESULT_SUCCESS) {
			return SL_PLAYSTATE_STOPPED;
		}

		SLuint32 statePlayer;
		result = (*player)->GetPlayState(player, &statePlayer);
		if(statePlayer == SL_PLAYSTATE_PLAYING) {
			if (state.count == 0) {
				return SL_PLAYSTATE_STOPPED;
			}
		}

		return statePlayer;
	}

	return SL_PLAYSTATE_STOPPED;
}

#include <math.h>

SLmillibel OpenSLPlayer::gain_to_attenuation(float gain) {
	SLmillibel volume_mb;
	if (gain >= 1.0f) {
		(*playerVolume)->GetMaxVolumeLevel(playerVolume, &volume_mb);
	} else if (gain <= 0.02f) {
		volume_mb = SL_MILLIBEL_MIN;
	} else {
		volume_mb = (SLmillibel)(M_LN2 / logf(1.0f / (1.0f - gain)) * -1000.0f);
		if (volume_mb > 0) {
			volume_mb = SL_MILLIBEL_MIN;
		}
	}

	return volume_mb;
}

float OpenSLPlayer::gain_from_attenuation(float attenuation) {
	return powf(10.0f, attenuation / (1000.0f * 20.0f ));
}

void OpenSLPlayer::setVolume(float volume) {
	//SLresult result;

	if (playerVolume != NULL) {
		SLmillibel newVolume = gain_to_attenuation(volume);

		/*result = */(*playerVolume)->SetVolumeLevel(playerVolume, newVolume);
	}
}

float OpenSLPlayer::getVolume() {
	SLresult result;

	if (playerVolume != NULL) {
		SLmillibel millibel;

		result = (*playerVolume)->GetVolumeLevel(playerVolume, &millibel);

		if (result == SL_RESULT_SUCCESS) {
			return gain_from_attenuation(millibel);
		}
	}

	return 0.0;
}

void OpenSLPlayer::pause() {
	if (player == NULL) return;

	//SLresult result;

	/*result = */(*player)->SetPlayState(player, SL_PLAYSTATE_PAUSED);
}

void OpenSLPlayer::setSound(OpenSLSound* sound) {
	lock();

	if (bufferQueue != NULL) {
		AtomicSetPtr(&(this->sound), sound);

		(*bufferQueue)->Clear(bufferQueue);
		AtomicSet(&buffersUsed, 0);
		fillBuffers();
	}

	unlock();
}

void OpenSLPlayer::queue() {
	lock();

	if (bufferQueue != NULL) {
		OpenSLSound* sound = AtomicGetPtr(&this->sound);
		if (sound != NULL) {
			int index = AtomicIncrement(&nextBuffer) % 4;
			AtomicAnd(&nextBuffer, 0x3);

			uint8_t *buffer = AtomicGetPtr(&(buffers[index]));

			//LOG("OpenSLPlayer::queue (Sound:%08X / Buffer:%d:%08X)", (uint32_t)(intptr_t)sound, index, (uint32_t)(intptr_t)buffer);

			size_t size = sound->fillBuffer(buffer, bufferSize);

			if (size > 0) {
				AtomicIncrement(&buffersUsed);
				(*bufferQueue)->Enqueue(bufferQueue, buffer, (SLuint32)size);
			}
		}
	}

	unlock();
}

void OpenSLPlayer::fillBuffers() {
	lock();

	int count = 2 - AtomicGet(&buffersUsed);
	for (int i = 0; i < count; i++) {
		queue();
	}

	unlock();
}


void OpenSLPlayer::play() {
	if (player == NULL) return;

	fillBuffers();

	//SLresult result;
	/*result= */(*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING);
}

void OpenSLPlayer::stop() {
	if (player == NULL) return;

	lock();

	//SLresult result;
	/*result=  */(*player)->SetPlayState(player, SL_PLAYSTATE_STOPPED);

	if (bufferQueue != NULL) {
		(*bufferQueue)->Clear(bufferQueue);
		AtomicSet(&buffersUsed, 0);
	}

	unlock();
}

char* OpenSLPlayer::getBuffer(size_t* size) {
	lock();

	uint8_t *buffer = buffers[nextBuffer++];
	if (nextBuffer >= 4) nextBuffer = 0;

	unlock();

	return (char*)buffer;
}
