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
#include <sound/mp3/uMP3Sound.h>
#include <sound/wav/uWAVSound.h>
#include <math.h>

OpenSLSound::OpenSLSound() {
}

OpenSLSound::~OpenSLSound() {
	lock();

	if (sound != NULL) {
		stop();

		sound->closeBuffer(sound);
		sound->deleteBuffer(sound);
		memDelete(sound);
		sound = NULL;
	}
}

bool OpenSLSound::load(Stream* stream) {
	lock();

	if (AtomicGet(&playing)) {
		stop();
	}

	//LOG("OpenSLSound: LoadStream");

	if ((stream == NULL) || (stream->getLimit() < 4)) {
		unlock();

		return false;
	}

	// Проверяем формат данных
	uint32_t sig;
	uint32_t retOfs = stream->getPosition();
	// Fix: Некоторые файлы mp3 содержат в начале нули или тэги, навсякий случай пропускаем
	// Fix: Некоторые файлы mp3 содержат в начале ID3v2 таг, его необходимо пропустить
	try {
		sig = stream->readU32();
		if ((sig & 0x00FFFFFF) == '3DI') {
			//LOG("OpenSLSound: ID3");

			sig = 0;
			do {
				sig = (sig << 8) | (uint32_t)(stream->readU8());
				if (((sig & 0xFFE00000) == 0xFFE00000) && (sig & (3 << 17)) &&
					((sig & (3 << 10)) != (3 << 10))) {

					//LOG("OpenSLSound: Sync at %08X", stream->getPosition() - 4);

					break;
				}
			} while (true);

			sig = __builtin_bswap32(sig);
		} else {
			while (sig == 0) {
				sig = stream->readU32();
			}
		}
	} catch (...) {
		stream->setPosition(retOfs);

		unlock();

		return false;
	}
	stream->setPosition(retOfs);

	sound = NULL;
	if (is_mp3(sig)) {
		//LOG("OpenSLSound: MP3");
		try {
			sound = memNew(sound, MP3Sound());
			if (sound->createBuffer(sound)) {
				if (sound->available(sound) == 0) {
					sound->deleteBuffer(sound);
					memDelete(sound);
					sound = NULL;
				}
			} else {
				memDelete(sound);
				sound = NULL;
			}
		} catch (...) {
		}
	}

	if ((sound == NULL) && (is_wav(sig))) {
		//LOG("OpenSLSound: WAV/RIFF");
		try {
			sound = memNew(sound, WAVSound());
			if (sound->createBuffer(sound)) {
				if (sound->available(sound) == 0) {
					sound->deleteBuffer(sound);
					memDelete(sound);
					sound = NULL;
				}
			} else {
				memDelete(sound);
				sound = NULL;
			}
		} catch (...) {
		}
	}

	if (sound == NULL) {
		unlock();

		return false;
	}

	if ( !sound->openBuffer(sound, stream) ) {
		sound->deleteBuffer(sound);
		memDelete(sound);
		sound = NULL;

		unlock();

		return false;
	}

	// Вычисляем параметры подходящего аудео плеера
	if (!sound->converter.build(AUDIO_FORMAT_S16, 2, 48000)) {
		sound->closeBuffer(sound);
		sound->deleteBuffer(sound);
		memDelete(sound);
		sound = NULL;

		unlock();

		return false;
	}

	LOG("OpenSLSound: OpenBuffer (Convert %d > %d)", sound->converter.inSpec.frequency, sound->converter.outSpec.frequency);

	// Проверяем, требуются ли преобразования
	if (sound->converter.convert) {
		LOG("OpenSLSound: ConvertFormat Buffer %d > %d", sound->converter.formatBufferInLength, sound->converter.formatBufferOutLength);
	}

	LOG("OpenSLSound: TotalSamples %d", sound->totalSamples);

	lastConvertOfs = 0;
	lastConvertSize = 0;

	AtomicSet(&paused, false);
	AtomicSet(&playing, false);
	AtomicSet(&rewind, false);

	unlock();

	return true;
}

size_t OpenSLSound::memoryUsed() {
	lock();

	if ( (sound != NULL) && (sound->converter.convert) ) {
		size_t size = sound->converter.formatBufferInLength > sound->converter.formatBufferOutLength ? sound->converter.formatBufferInLength : sound->converter.formatBufferOutLength;

		unlock();

		return size;
	}

	unlock();

	return 0;
}

size_t OpenSLSound::audioSize() {
	lock();

	if (sound != NULL) {
		size_t size = sound->totalSamples * sound->converter.outSpec.sample;

		unlock();

		return size;
	}

	unlock();

	return 0;
}


size_t OpenSLSound::fillBuffer(uint8_t* buffer, size_t bufferSize) {
	//LOG("OpenSLSound::fillBuffer");

	lock();

	if (sound == NULL) {
		unlock();

		return 0;
	}

	size_t lastBufferSize = 0;

	if (sound->converter.convert) {
		size_t filled = 0;
		size_t left = bufferSize - (bufferSize % sound->converter.outSpec.sample);
		do {
			if ((lastConvertSize > 0) && (lastConvertOfs < lastConvertSize)) {
				filled = lastConvertSize - lastConvertOfs;
				if (filled > left) {
					//LOG("AudioCVT: memcpy(%d, %d, %d)", (uint32_t)lastBufferSize, (int)lastConvertOfs, (uint32_t)left);
					memcpy(buffer + lastBufferSize, sound->converter.formatBuffer + lastConvertOfs, left);
					lastBufferSize += left;
					lastConvertOfs += left;
					left = 0;
				} else {
					//LOG("AudioCVT: memcpy(%d, %d, %d)", (uint32_t)lastBufferSize, (int)lastConvertOfs, (uint32_t)filled);
					memcpy(buffer + lastBufferSize, sound->converter.formatBuffer + lastConvertOfs,
						   filled);
					lastBufferSize += filled;
					lastConvertOfs = 0;
					lastConvertSize = 0;
					left -= filled;
				}
			}

			if (left == 0) {
				break;
			}

			filled = sound->fillBuffer(sound, sound->converter.formatBuffer, sound->converter.formatBufferInLength);
			LOG("AudioCVT: fillBuffer(%d) = %d", sound->converter.formatBufferInLength, (uint32_t)filled);
			if (filled == 0) {
				if (isLooping()) {
					sound->resetBuffer(sound);
				} else {
					break;
				}
			} else {
				size_t used = 0;
				lastConvertSize = sound->converter.process(filled, used);
				LOG("AudioCVT: lastConvertSize = %d", (uint32_t)lastConvertSize);
			}
		} while (true);
	} else {
		size_t filled = 0;
		size_t left = bufferSize - (bufferSize % sound->converter.outSpec.sample);
		do {
			filled = sound->fillBuffer(sound, buffer + lastBufferSize, left);
			//LOG("AudioCVT: fillBuffer(%d) = %d", (uint32_t)left, (uint32_t)filled);

			if (filled == 0) {
				if (isLooping()) {
					sound->resetBuffer(sound);
				} else {
					break;
				}
			} else {
				left -= filled;
				lastBufferSize += filled;
			}

			if (left == 0) {
				break;
			}
		} while (true);

		//LOG("AudioCVT: lastBufferSize = %d", (uint32_t)lastBufferSize);
	}

	unlock();

	return lastBufferSize;
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

void OpenSLSound::setLoop(bool loop) {
	AtomicSet(&(this->loop), loop);
}

bool OpenSLSound::isLooping() {
	return AtomicGet(&(this->loop));
}

void OpenSLSound::play(OpenSLContext* context) {
	lock();

	if (sound == NULL) {
		unlock();
		return;
	}

	if (AtomicExchange(&rewind, false)) {
		//LOG("OpenSLSound: sound->resetBuffer()");
		sound->resetBuffer(sound);
	}

	OpenSLPlayer* player = AtomicGet(&(this->player));
	if (player != NULL) {
		player->lock();

		if (AtomicCompareExchange(&(player->sound), NULL, NULL) == this) {
			if (!player->isPlaying()) {
				//LOG("OpenSLSound: !player->isPlaying()");

				player->setVolume(volume);

				if (context->isPaused()) {
					AtomicSet(&player->resume, true);
				} else {
					//LOG("OpenSLSound: player->play()");

					player->play();
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

	if (context->isPaused()) {
		AtomicSet(&player->resume, true);
	} else {
		//LOG("OpenSLSound: player->play()");

		player->play();
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
				AtomicSet(&(this->player), NULL);
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
