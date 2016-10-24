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

#include "uOpenSLSound.h"
#include <sound/uOpenSLPlayer.h>

OpenSLSound::OpenSLSound(OpenSLContext* context) {
	AtomicSet(&(this->context), context);
}

OpenSLSound::~OpenSLSound() {
}

void OpenSLSound::setVolume(float volume) {
	this->volume = volume;
	OpenSLPlayer* player = AtomicGet(&(this->player));
	if (player != NULL) {
		if (AtomicCompareExchange(&(player->sound), NULL, NULL) == this) {
			player->setVolume(volume);
		}
	}
}

float OpenSLSound::getVolume() {
	return this->volume;
}

void OpenSLSound::pause() {
	OpenSLPlayer* player = AtomicGet(&(this->player));
	if (player != NULL) {
		player->pause();
	}
}
void OpenSLSound::setLoop(bool loop) {
	AtomicSet(&(this->loop), loop);
}

bool OpenSLSound::isLooping() {
	return AtomicGet(&(this->loop));
}

void OpenSLSound::play() {
	OpenSLPlayer* player = AtomicGet(&(this->player));
	if (player != NULL) {
		if (AtomicCompareExchange(&(player->sound), NULL, NULL) == this) {
			if (player->isPaused()) {
				player->setVolume(volume);
				player->play();
			}
			return;
		}
	}

	OpenSLContext* context = AtomicGet(&(this->context));
	AtomicSet(&(this->player), player = context->getFreePlayer(this));

	if(	player == NULL){
		return;
	}

	player->setSound(this);
	player->setVolume(volume);
	player->play();
}

void OpenSLSound::stop() {
	OpenSLPlayer* player = AtomicGet(&(this->player));
	if (player != NULL) {
		if (AtomicCompareExchange(&(player->sound), NULL, NULL) == this) {
			player->stop();
			AtomicSet(&(this->player), NULL);
		}
	}
}

SLuint32 OpenSLSound::state() {
	OpenSLPlayer* player = AtomicGet(&(this->player));
	if (player != NULL) {
		return player->state();
	}

	return SL_PLAYSTATE_STOPPED;
}

bool OpenSLSound::isActive() {
	return state() != SL_PLAYSTATE_STOPPED;
}

bool OpenSLSound::isPlaying(){
	return state() == SL_PLAYSTATE_PLAYING;
}

bool OpenSLSound::isPaused(){
	return state() == SL_PLAYSTATE_PAUSED;
}

bool OpenSLSound::isStopped(){
	return state() == SL_PLAYSTATE_STOPPED;
}
