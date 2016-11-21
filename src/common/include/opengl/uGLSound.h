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

#ifndef JAPPSY_UGLSOUND_H
#define JAPPSY_UGLSOUND_H

#include <data/uObject.h>
#include <data/uVector.h>
#include <core/uAtomic.h>

class GLRender;
class GLSoundMixer;
class GLSound;

class GLSoundVolumeFx : public CObject {
public:
	GLSound* target = NULL;
	
	uint64_t timeout;
	uint64_t speed;
	
	uint64_t timeStartIn;
	uint64_t timeStartOut;
	
	GLSoundVolumeFx(GLSound* target, uint64_t timeout, uint64_t speed);
	~GLSoundVolumeFx();
	
	typedef void (*onUpdateCallback)(GLSoundVolumeFx* fx);
	
	onUpdateCallback onupdate;
};

class GLSound : public CObject {
public:
	GLSoundMixer* mixer = NULL;
	CString key;
	
	void* audio = NULL;
	uint32_t size = 0;
	
	GLfloat volume = 0.0;
	bool volex = false;
	
	bool loop = false;
	bool paused = false;
	
	bool invalid = false;
	
	GLfloat volumeFxScale = 1.0;
	GLSoundVolumeFx* volumeFx = NULL;
	
	GLSound(GLSoundMixer* mixer, const CString& key, void* audio, uint32_t size) throw(const char*);
	~GLSound();
	
	void update();
	
	void mixVolume(GLfloat volume, bool volex = false);
	void mixPlayEx(bool loop, GLfloat volume, bool reset, bool volex = false);
	void mixPlay(bool loop, GLfloat volume, bool reset, bool volex = false);
	bool mixPlaying();
	void mixPause(bool pause);
	void mixStop();
	void mixPlayTimeout(bool loop, GLfloat volume, bool reset, bool volex, uint64_t timeout, uint64_t speed);
};

class GLSoundMixer : public CObject {
private:
	GLRender* context;
	VectorMap<CString&, GLSound*> list;
	VectorMap<CString&, GLSound*> listFx;
	
public:
	GLfloat mixerVolume = 1.0;
	GLfloat mixerMaxVolume = 1.0;
	
	GLSoundMixer(GLRender* context) throw(const char*);
	~GLSoundMixer();
	
	GLSound* get(const CString& key) throw(const char*);
	GLSound* createSound(const CString& key, void* audio, uint32_t size) throw(const char*);
	
	void addFx(GLSound* sound);
	void removeFx(GLSound* sound);
	
	void mixMaxVolume(GLfloat volume, GLfloat maxvolume);
	bool mixPlaying();
	void update();
	void stop();
};

#endif //JAPPSY_UGLSOUND_H
