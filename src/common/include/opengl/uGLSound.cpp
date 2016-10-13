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

#include "uGLSound.h"
#include <opengl/uGLRender.h>

#include <sound/uMixer.h>

void onUpdateFx(GLSoundVolumeFx* fx) {
	uint64_t time = currentTimeMillis();
	
	if (time < fx->timeout) {
		int64_t elapsed = (int64_t)(time - fx->timeStartIn);
		GLfloat v = (GLfloat)elapsed / (GLfloat)(fx->speed);
		if (v > 1.0) v= 1.0;
		fx->target->volumeFxScale = v;
		fx->target->invalid = true;
		fx->target->update();
	} else {
		int64_t elapsed = (int64_t)(time - fx->timeStartOut);
		GLfloat v = 1.0 - (GLfloat)elapsed / (GLfloat)(fx->speed);
		if (v < 0.0) v = 0.0;
		fx->target->volumeFxScale = v;
		fx->target->invalid = true;
		fx->target->update();
		if (v <= 0.0) {
			fx->target->mixPause(true);
			delete fx;
		}
	}
}

GLSoundVolumeFx::GLSoundVolumeFx(GLSound* target, uint64_t timeout, uint64_t speed) {
	uint64_t now = currentTimeMillis();
	
	this->target = target;
	this->timeout = now + timeout;
	
	this->timeStartIn = now - (uint64_t)(int64_t)floorf(target->volumeFxScale * (GLfloat)speed);
	
	GLfloat v = target->volumeFxScale + ((GLfloat)timeout / (GLfloat)speed);
	if (v > 1.0) v = 1.0;
	
	this->timeStartOut = this->timeout - (uint64_t)(int64_t)floorf((1.0 - v) * (GLfloat)speed);
	
	target->mixer->addFx(target);
	
	this->speed = speed;
	
	this->onupdate = onUpdateFx;
}

GLSoundVolumeFx::~GLSoundVolumeFx() {
	if (target != NULL) {
		target->mixer->removeFx(target);
		target->volumeFx = NULL;
		target = NULL;
	}
}

GLSound::GLSound(GLSoundMixer* mixer, const CString& key, void* audio, uint32_t size) throw(const char*) {
	this->mixer = mixer;
	this->key = key;
	
	this->audio = audio;
	this->size = size;
}

GLSound::~GLSound() {
	mixStop();
	destroyAudio(audio);
}

void GLSound::update() {
	if (invalid) {
		invalid = false;
		
		GLfloat v = volume;
		if (volex) {
			if (v < 0.0) v = 1.0;
			if (v > 1.0) v = 1.0;
			v *= volumeFxScale;
			volumeAudio(audio, v);
		} else {
			if (mixer->mixerVolume == 0.0) {
				if (volume < -1.0) {
					GLfloat v = volumeFxScale;
					volumeAudio(audio, v);
				} else {
					volumeAudio(audio, 0.0);
				}
			} else {
				if (v == -1.0) v = 1.0;
				else if (v < 0) v = mixer->mixerMaxVolume;
				v *= volumeFxScale;
				v *= mixer->mixerVolume / mixer->mixerMaxVolume;
				volumeAudio(audio, v);
			}
		}
	}
}

void GLSound::mixVolume(GLfloat volume, bool volex) {
	this->volex = volex;
	this->volume = volume;
	volumeFxScale = 1.0;
	invalid = true;
	update();
}

void GLSound::mixPlayEx(bool loop, GLfloat volume, bool reset, bool volex) {
	if (reset) {
		stopAudio(audio);
	}
	
	this->loop = loop;
	
	mixVolume(volume, volex);
	
	if ((paused) || (!playingAudio(audio))) {
		paused = false;
		
		playAudio(audio, loop);
	}
}

void GLSound::mixPlay(bool loop, GLfloat volume, bool reset, bool volex) {
	if ((reset) || (!playingAudio(audio)) || (paused)) {
		volumeFxScale = 1.0;
		if (volumeFx != NULL) {
			delete volumeFx;
		}
	}

	mixPlayEx(loop, volume, reset, volex);
}

bool GLSound::mixPlaying() {
	return playingAudio(audio);
}

void GLSound::mixPause(bool pause) {
	if (pause) {
		if (!paused) {
			paused = true;
			pauseAudio(audio);
		}
	} else {
		if ((paused) || (!playingAudio(audio))) {
			paused = false;
			
			playAudio(audio, loop);
		}
	}
}

void GLSound::mixStop() {
	if (playingAudio(audio)) {
		paused = false;
		
		volumeFxScale = 1.0;
		if (volumeFx != NULL) {
			delete volumeFx;
		}
		
		stopAudio(audio);
	}
}

void GLSound::mixPlayTimeout(bool loop, GLfloat volume, bool reset, bool volex, uint64_t timeout, uint64_t speed) {
	if ((reset) || (!playingAudio(audio)) || (paused)) {
		volumeFxScale = 1.0;
	}
	
	mixPlay(loop, volume, reset, volex);
	volumeFx = new GLSoundVolumeFx(this, timeout, speed);
}

GLSoundMixer::GLSoundMixer(GLRender* context) throw(const char*) {
	this->context = context;
}

GLSoundMixer::~GLSoundMixer() {
	int32_t count = list.count();
	GLSound** items = list.items();
	for (int i = 0; i < count; i++) {
		delete items[i];
	}
}

GLSound* GLSoundMixer::get(const CString& key) throw(const char*) {
	return list.get(key);
}

GLSound* GLSoundMixer::createSound(const CString& key, void* audio, uint32_t size) throw(const char*) {
	try {
		list.removedelete(key);
		GLSound* sound = new GLSound(this, key, audio, size);
		try {
			list.put(key, sound);
		} catch (...) {
			delete sound;
			throw;
		}
		return sound;
	} catch (...) {
		throw;
	}
}

void GLSoundMixer::addFx(GLSound* sound) {
	try {
		listFx.put(sound->key, sound);
	} catch (...) {
		throw;
	}
}

void GLSoundMixer::removeFx(GLSound* sound) {
	try {
		listFx.remove(sound->key);
	} catch (...) {
		throw;
	}
}

void GLSoundMixer::mixMaxVolume(GLfloat volume, GLfloat maxvolume) {
	mixerVolume = (volume < 0) ? 1.0 : volume;
	mixerMaxVolume = (maxvolume < 0) ? 1.0 : maxvolume;
	
	int32_t count = list.count();
	GLSound** items = list.items();
	for (int i = 0; i < count; i++) {
		items[i]->invalid = true;
		items[i]->update();
	}
}

void GLSoundMixer::update() {
	int32_t count = listFx.count();
	GLSound** items = listFx.items();
	for (int i = 0; i < count; i++) {
		items[i]->volumeFx->onupdate(items[i]->volumeFx);
	}
}

void GLSoundMixer::stop() {
	int32_t count = list.count();
	GLSound** items = list.items();
	for (int i = 0; i < count; i++) {
		items[i]->mixStop();
	}
}
