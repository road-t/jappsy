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

#include "uOpenSLContext.h"
#include <sound/uOpenSLPlayer.h>

bool OpenSLContext::init() {
	lock();

	shutdown();

	SLresult result;

	const SLuint32 lEngineMixIIDCount = 1;
	const SLInterfaceID lEngineMixIIDs[] = {SL_IID_ENGINE};
	const SLboolean lEngineMixReqs[] = {SL_BOOLEAN_TRUE};
	const SLuint32 lOutputMixIIDCount = 0;
	const SLInterfaceID lOutputMixIIDs[] = {};
	const SLboolean lOutputMixReqs[] = {};

	result = slCreateEngine(&engineObj, //pointer to object
							0, // count of elements is array of additional options
							NULL, // array of additional options
							lEngineMixIIDCount, // interface count
							lEngineMixIIDs, // array of interface ids
							lEngineMixReqs);

	if (result != SL_RESULT_SUCCESS ) {
		unlock();

		LOG("OpenSL: Error create engine");
		return false;
	}

	result = (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE );

	if (result != SL_RESULT_SUCCESS ) {
		unlock();

		LOG("OpenSL: Error realize engine");
		return false;
	}

	result = (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engine);

	if (result != SL_RESULT_SUCCESS ) {
		unlock();

		LOG("OpenSL: Error get engine interface");
		return false;
	}

	result = (*engine)->CreateOutputMix(engine, &(outputMixObj), lOutputMixIIDCount, lOutputMixIIDs, lOutputMixReqs);

	if (result != SL_RESULT_SUCCESS ) {
		unlock();

		LOG("OpenSL: Error create output mixer");
		return false;
	}

	result = (*outputMixObj)->Realize(outputMixObj, SL_BOOLEAN_FALSE );

	if (result != SL_RESULT_SUCCESS ) {
		unlock();

		LOG("OpenSL: Error realize output mixer");
		return false;
	}

	unlock();

	return true;
}

void OpenSLContext::shutdown() {
	lock();

	for (int i = 0; i < OPENSL_CHANNELS; i++) {
		OpenSLPlayer* player = AtomicExchange(&(players[i]), NULL);
		if (player != NULL) {
			delete player;
		}
	}

	if (outputMixObj != NULL) {
		(*outputMixObj)->Destroy(outputMixObj);
		outputMixObj = NULL;
	}

	if (engine != NULL) {
		(*engineObj)->Destroy(engineObj);
		engineObj = NULL;
		engine = NULL;
	}

	unlock();
}

bool OpenSLContext::isPaused() {
	return AtomicGet(&paused);
}

void OpenSLContext::pause() {
	lock();

	if (!AtomicExchange(&paused, true)) {
		for (int i = 0; i < OPENSL_CHANNELS; i++) {
			OpenSLPlayer *player = AtomicGet(&(players[i]));
			if (player != NULL) {
				player->lock();

				if (player->isPlaying()) {
					player->pause();
					AtomicSet(&player->resume, true);
				}

				player->unlock();
			}
		}
	}

	unlock();
}

void OpenSLContext::resume() {
	lock();

	if (AtomicExchange(&paused, false)) {
		for (int i = 0; i < OPENSL_CHANNELS; i++) {
			OpenSLPlayer *player = AtomicGet(&(players[i]));
			if (player != NULL) {
				player->lock();
				player->restore();
				player->unlock();
			}
		}
	}

	unlock();
}

OpenSLPlayer* OpenSLContext::getFreePlayer(OpenSLSound* sound) {
	lock();

	int empty = -1;
	int old_index = -1;
	uint64_t old_access = 0;

OpenSLContext_getFreePlayer_repeat:

	for (int i = 0; i < OPENSL_CHANNELS; i++) {
		OpenSLPlayer *player = AtomicGet(&(players[i]));
		if (player == NULL) {
			empty = i;
		} else {
			player->lock();

			if ((old_access == 0) || (player->accessTime <= old_access)) {
				old_index = i;
				old_access = player->accessTime;
			}

			if (player->isStopped()) {
				unlock();

				AtomicSet(&player->resume, false);
				return player;
			}

			player->unlock();
		}
	}

	if (empty >= 0) {
		OpenSLPlayer *player;
		AtomicSet(&(players[empty]), player = new OpenSLPlayer(this));

		player->lock();
		unlock();

		return player;
	}

	if (old_index >= 0) {
		OpenSLPlayer *player = AtomicGet(&(players[old_index]));
		if (player != NULL) {
			player->lock();

			if (player->isPlaying()) {
				player->stop();
			}

			AtomicSet(&player->resume, false);

			unlock();

			return player;
		}
	}

	goto OpenSLContext_getFreePlayer_repeat;
}
