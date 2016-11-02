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
#include <io/uFileIO.h>

OpenSLSound::OpenSLSound() {
}

void OpenSLSound::clear() {
	lock();

	int fd = AtomicExchange(&this->fd, -1);

	if (fd != -1) {
		if (AtomicGet(&playing)) {
			stop();
		}

		const char* error = eOK;
		if (!fio_close(fd, &error)) {
			LOG("OpenSLSound: fio_close: %s", error);
		}
	}

	unlock();
}

OpenSLSound::~OpenSLSound() {
	lock();

	clear();
}

bool OpenSLSound::load(const CString& path) {
	lock();

	clear();

	char *filePath;
	try {
		filePath = path.toChar(NULL);
	} catch (...) {
		unlock();
		return false;
	}

	const char* error = eOK;

	int fd = fio_open(filePath, &error);
	if (fd == -1) {
		memFree(filePath);
		unlock();
		return false;
	}

	memFree(filePath);

	off_t fileSize = 0;
	if (!fio_begin(fd, NULL, &fileSize, &error)) {
		fio_close(fd, NULL);
		unlock();
		return false;
	}

	AtomicSet(&this->fd, fd);
	AtomicSet(&this->fileSize, fileSize);

	AtomicSet(&paused, false);
	AtomicSet(&playing, false);
	AtomicSet(&rewind, false);

	unlock();

	return true;
}

void OpenSLSound::reset() {
	lock();

	AtomicSet(&paused, false);
	AtomicSet(&playing, false);
	AtomicSet(&rewind, true);

	unlock();
}

void OpenSLSound::setVolume(float volume) {
	lock();

	this->volume = volume;
	OpenSLPlayer* player = AtomicGet(&(this->player));
	if (player != NULL) {
		player->lock();

		if (AtomicCompareExchange(&(player->sound), NULL, NULL) == this) {
			player->setVolume(volume);
		}

		player->unlock();
	}

	unlock();
}

float OpenSLSound::getVolume() {
	return this->volume;
}

void OpenSLSound::pause() {
	lock();

	if (AtomicGet(&playing)) {
		AtomicSet(&paused, true);

		OpenSLPlayer *player = AtomicGet(&(this->player));
		if (player != NULL) {
			player->lock();

			AtomicSet(&player->resume, false);
			player->pause();

			player->unlock();
		}
	}

	unlock();
}

void OpenSLSound::play(OpenSLContext* context, bool loop) {
	lock();

	if (fd == -1) {
		unlock();
		return;
	}

	//if (AtomicExchange(&rewind, false)) {
		//LOG("OpenSLSound: sound->resetBuffer()");
	//	sound->resetBuffer(sound);
	//}

	OpenSLPlayer* player = AtomicGet(&(this->player));
	if (player != NULL) {
		player->lock();

		if (AtomicCompareExchange(&(player->sound), NULL, NULL) == this) {
			if (!player->isPlaying()) {
				//LOG("OpenSLSound: !player->isPlaying()");

				player->setVolume(volume);

				if (context->isPaused()) {
					AtomicSet(&player->resume, true);
					AtomicSet(&player->reset, AtomicExchange(&rewind, false));
				} else {
					//LOG("OpenSLSound: player->play()");

					player->play(loop, AtomicExchange(&rewind, false));
				}
			}

			AtomicSet(&paused, false);
			AtomicSet(&playing, true);

			player->unlock();
			unlock();

			return;
		}

		player->unlock();
	}

	AtomicSet(&(this->player), player = context->getFreePlayer(this));

	if (player == NULL) {
		unlock();

		return;
	}

	player->setSound(this);
	player->setVolume(volume);
	AtomicSet(&rewind, false);

	if (context->isPaused()) {
		AtomicSet(&player->resume, true);
		AtomicSet(&player->reset, false);
	} else {
		//LOG("OpenSLSound: player->play()");

		player->play(loop, false);
	}

	AtomicSet(&paused, false);
	AtomicSet(&playing, true);

	player->unlock();

	unlock();
}

void OpenSLSound::stop() {
	lock();

	if (AtomicGet(&playing)) {
		AtomicSet(&paused, false);
		AtomicSet(&playing, false);
		AtomicSet(&rewind, true);

		OpenSLPlayer *player = AtomicGet(&(this->player));
		if (player != NULL) {
			player->lock();

			if (AtomicCompareExchange(&(player->sound), NULL, NULL) == this) {
				player->stop();
			}

			player->unlock();
		}
	}

	unlock();
}

bool OpenSLSound::isPlaying() {
	return AtomicGet(&playing);
}

bool OpenSLSound::isPaused() {
	return AtomicGet(&paused);
}
