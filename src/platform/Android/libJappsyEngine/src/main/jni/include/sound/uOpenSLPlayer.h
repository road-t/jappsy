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

#ifndef JAPPSY_UOPENSLPLAYER_H
#define JAPPSY_UOPENSLPLAYER_H

#include <platform.h>
#include <data/uObject.h>
#include <sound/uOpenSLContext.h>
#include <sound/uOpenSLSound.h>

class OpenSLPlayer : public CObject {
public:
	OpenSLPlayer(struct OpenSLContext* context);
	virtual ~OpenSLPlayer();

	void setSound(OpenSLSound* sound);

	float getVolume();
	void setVolume(float volume);

	void pause();
	void play(bool loop, bool reset);
	void stop();
	void restore();

	bool isPlaying();
	bool isPaused();
	bool isStopped();

	OpenSLSound* sound = NULL;
	uint64_t accessTime = 0;
	jbool resume = false;
	jbool reset = false;

protected:
	virtual void clear();

	int state = SL_PLAYSTATE_STOPPED;
	jbool loop = false;

	OpenSLContext* context;

	SLmillibel gain_to_attenuation(float gain);
	static float gain_from_attenuation(float attenuation);

	SLDataLocator_AndroidFD locatorFD = {0};
	SLDataFormat_MIME formatMIME = {0};

	SLDataSource audioSrc = {0};
	SLDataLocator_OutputMix locatorOutMix = {0};
	SLDataSink audioSnk = {0};

	SLObjectItf playerObj = NULL;
	SLPlayItf player = NULL;
	SLVolumeItf playerVolume = NULL;
	SLSeekItf playerSeek = NULL;
	SLMuteSoloItf playerMuteSolo = NULL;

	static void EventCallback(SLPlayItf caller, void* userData, SLuint32 event);
};

#endif //JAPPSY_UOPENSLPLAYER_H
