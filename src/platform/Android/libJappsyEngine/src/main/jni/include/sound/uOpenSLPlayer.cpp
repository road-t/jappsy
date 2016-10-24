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

void OpenSLPlayerEventCallback(SLPlayItf caller, void* userData, SLuint32 event) {
	OpenSLPlayer* player = (OpenSLPlayer*)userData;

	LOG("OpenSL: PlayerEvent %d", event);
}

void OpenSLPlayerBufferCallback(SLBufferQueueItf caller, void* userData) {
	LOG("OpenSL: BufferEvent");
}

OpenSLPlayer::OpenSLPlayer(OpenSLContext* context) {
	AtomicSet(&(this->context), context);

	SLresult result;

	locatorBufferQueue.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
	locatorBufferQueue.numBuffers = 16;

	formatPCM.formatType = SL_DATAFORMAT_PCM;
	formatPCM.numChannels = 2;
	formatPCM.samplesPerSec = SL_SAMPLINGRATE_44_1;
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

	result = (*player)->RegisterCallback(player, OpenSLPlayerEventCallback, (void*)this);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		return;
	}

	result = (*bufferQueue)->RegisterCallback(bufferQueue, OpenSLPlayerBufferCallback, (void*)this);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		return;
	}

	result = (*player)->SetCallbackEventsMask(player, SL_PLAYEVENT_HEADATEND | SL_PLAYEVENT_HEADATMARKER | SL_PLAYEVENT_HEADATNEWPOS | SL_PLAYEVENT_HEADMOVING | SL_PLAYEVENT_HEADSTALLED);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		return;
	}

	//result = (*playerSeek)->SetLoop(playerSeek, sound->isLooping() ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE, 0, SL_TIME_UNKNOWN);
}

void OpenSLPlayer::clear() {
	if (playerObj != NULL) {
		(*playerObj)->Destroy(playerObj);
		player = NULL;
		playerMuteSolo = NULL;
		playerVolume = NULL;
		playerObj = NULL;
		bufferQueue = NULL;
	}
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
	if (gain >= 3.0f) {
		(*playerVolume)->GetMaxVolumeLevel(playerVolume, &volume_mb);
	} else if (gain <= 0.02f) {
		volume_mb = SL_MILLIBEL_MIN;
	} else {
		volume_mb = (SLmillibel)(M_LN2 / logf(3.0f / (3.0f - gain)) * -1000.0f);
		if (volume_mb > 0) {
			volume_mb = SL_MILLIBEL_MIN;
		}
	}

	return volume_mb;
}

float OpenSLPlayer::gain_from_attenuation(float attenuation) {
	return (float)powf(10.0f, attenuation / (1000.0f * 20.0f ));
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

void OpenSLPlayer::pause(){
	if (player == NULL) return;

	//SLresult result;

	/*result = */(*player)->SetPlayState(player, SL_PLAYSTATE_PAUSED);
}

void OpenSLPlayer::setSound(OpenSLSound* sound) {
	if (bufferQueue != NULL) {
		AtomicSet(&(this->sound), sound);

		(*bufferQueue)->Clear(bufferQueue);
		(*bufferQueue)->Enqueue(bufferQueue, sound->getBuffer(), (SLuint32) sound->getSize());
	}
}

void OpenSLPlayer::play() {
	if (player == NULL) return;

	//SLresult result;
	/*result= */(*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING);
}

void OpenSLPlayer::stop() {
	if (player == NULL) return;

	//SLresult result;
	/*result=  */(*player)->SetPlayState(player, SL_PLAYSTATE_STOPPED);
}