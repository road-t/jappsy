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
#include <io/uFileIO.h>
#include <core/uSystem.h>

bool OpenSLPlayer::isPlaying() {
	return AtomicGet(&state) == SL_PLAYSTATE_PLAYING;
}

bool OpenSLPlayer::isPaused() {
	return AtomicGet(&state) == SL_PLAYSTATE_PAUSED;
}

bool OpenSLPlayer::isStopped() {
	return AtomicGet(&state) == SL_PLAYSTATE_STOPPED;
}

void OpenSLPlayer::EventCallback(SLPlayItf caller, void* userData, SLuint32 event) {
	OpenSLPlayer* player = (OpenSLPlayer*)userData;
	if (event == SL_PLAYEVENT_HEADATEND) {
		player->stop();
	}

	//LOG("OpenSL: PlayerEvent %d", event);
}

OpenSLPlayer::OpenSLPlayer(OpenSLContext* context) {
	AtomicSet(&(this->context), context);
}

void OpenSLPlayer::clear() {
	lock();

	if (playerObj != NULL) {
		stop();

		(*playerObj)->Destroy(playerObj);
		player = NULL;
		playerMuteSolo = NULL;
		playerVolume = NULL;
		playerObj = NULL;
	}

	unlock();
}

OpenSLPlayer::~OpenSLPlayer() {
	clear();
}

void OpenSLPlayer::setSound(OpenSLSound* sound) {
	lock();

	clear();

	SLresult result;

	locatorFD.locatorType = SL_DATALOCATOR_ANDROIDFD;
	locatorFD.fd = AtomicGet(&sound->fd);
	locatorFD.offset = 0;
	locatorFD.length = AtomicGet(&sound->fileSize);

	formatMIME.formatType = SL_DATAFORMAT_MIME;
	formatMIME.mimeType = NULL;
	formatMIME.containerType = SL_CONTAINERTYPE_UNSPECIFIED;

	audioSrc.pLocator = &locatorFD;
	audioSrc.pFormat = &formatMIME;

	locatorOutMix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
	locatorOutMix.outputMix = context->outputMixObj;

	audioSnk.pLocator = (void*)&locatorOutMix;
	audioSnk.pFormat = NULL;

	const SLInterfaceID ids[4] = {SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
	const SLboolean req[4] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

	result = (*context->engine)->CreateAudioPlayer(context->engine, &playerObj, &audioSrc, &audioSnk, 3, ids, req);
	if (result != SL_RESULT_SUCCESS) {
		playerObj = NULL;
		unlock();
		return;
	}

	result = (*playerObj)->Realize(playerObj, SL_BOOLEAN_FALSE);
	if (result != SL_RESULT_SUCCESS) {
		playerObj = NULL;
		unlock();
		return;
	}

	result = (*playerObj)->GetInterface(playerObj, SL_IID_PLAY, &player);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		unlock();
		return;
	}

	result = (*playerObj)->GetInterface(playerObj, SL_IID_SEEK, &playerSeek);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		unlock();
		return;
	}

	result = (*playerObj)->GetInterface(playerObj, SL_IID_MUTESOLO, &playerMuteSolo);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		unlock();
		return;
	}

	result = (*playerObj)->GetInterface(playerObj, SL_IID_VOLUME, &playerVolume);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		unlock();
		return;
	}

	result = (*player)->RegisterCallback(player, EventCallback, (void*)this);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		unlock();
		return;
	}

	result = (*player)->SetCallbackEventsMask(player, SL_PLAYEVENT_HEADATEND/* | SL_PLAYEVENT_HEADATMARKER | SL_PLAYEVENT_HEADATNEWPOS | SL_PLAYEVENT_HEADMOVING | SL_PLAYEVENT_HEADSTALLED*/);
	if (result != SL_RESULT_SUCCESS) {
		clear();
		unlock();
		return;
	}

	AtomicSetPtr(&(this->sound), sound);

	accessTime = currentTimeMillis();

	unlock();
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
	lock();

	accessTime = currentTimeMillis();

	//SLresult result;

	if (playerVolume != NULL) {
		SLmillibel newVolume = gain_to_attenuation(volume);

		/*result = */(*playerVolume)->SetVolumeLevel(playerVolume, newVolume);
	}

	unlock();
}

float OpenSLPlayer::getVolume() {
	lock();

	accessTime = currentTimeMillis();

	SLresult result;

	if (playerVolume != NULL) {

		SLmillibel millibel;

		result = (*playerVolume)->GetVolumeLevel(playerVolume, &millibel);

		if (result == SL_RESULT_SUCCESS) {
			unlock();

			return gain_from_attenuation(millibel);
		}
	}

	unlock();

	return 0.0;
}

void OpenSLPlayer::pause() {
	lock();

	accessTime = currentTimeMillis();

	if (player == NULL) {
		unlock();

		return;
	}

	AtomicCompareExchange(&state, SL_PLAYSTATE_PAUSED, SL_PLAYSTATE_PLAYING);

	//SLresult result;
	/*result = */(*player)->SetPlayState(player, SL_PLAYSTATE_PAUSED);

	unlock();
}

void OpenSLPlayer::play(bool loop, bool reset) {
	lock();

	accessTime = currentTimeMillis();

	if (player == NULL) {
		unlock();

		return;
	}

	//SLresult result;
	if (reset) (*playerSeek)->SetPosition(playerSeek, 0, SL_SEEKMODE_FAST);
	/*result = */(*playerSeek)->SetLoop(playerSeek, loop ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE, 0, SL_TIME_UNKNOWN);
	/*result= */(*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING);

	AtomicSet(&state, SL_PLAYSTATE_PLAYING);
	AtomicSet(&this->loop, loop);

	unlock();
}

void OpenSLPlayer::restore() {
	lock();

	accessTime = currentTimeMillis();

	if (player == NULL) {
		unlock();

		return;
	}

	if (isPaused() && AtomicExchange(&resume, false)) {
		if (AtomicExchange(&reset, false)) (*playerSeek)->SetPosition(playerSeek, 0, SL_SEEKMODE_FAST);
		(*playerSeek)->SetLoop(playerSeek, AtomicGet(&loop) ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE, 0, SL_TIME_UNKNOWN);
		(*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING);

		AtomicSet(&state, SL_PLAYSTATE_PLAYING);
	}

	unlock();
}

void OpenSLPlayer::stop() {
	lock();

	accessTime = currentTimeMillis();
	AtomicSet(&state, SL_PLAYSTATE_STOPPED);
	AtomicSet(&resume, false);
	AtomicSet(&reset, false);

	if (player == NULL) {
		unlock();

		return;
	}

	//SLresult result;
	/*result=  */(*player)->SetPlayState(player, SL_PLAYSTATE_STOPPED);

	OpenSLSound* sound = AtomicGetPtr(&this->sound);
	if (sound != NULL) {
		sound->reset();
	}

	unlock();
}
