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

#ifndef JAPPSY_UOPENSLSOUND_H
#define JAPPSY_UOPENSLSOUND_H

#include <platform.h>
#include <data/uObject.h>
#include <data/uString.h>
#include <sound/uOpenSLContext.h>

class OpenSLSound : public CObject {
public:
	OpenSLSound(struct OpenSLContext* context);
	virtual ~OpenSLSound();

	CString path;
	virtual void load(const CString& path) = 0;

	virtual void setVolume(float volume);
	virtual float getVolume();

	virtual void play();
	virtual void pause();
	virtual void stop();
	SLuint32 state();

	bool isActive();
	virtual bool isPlaying();
	virtual bool isPaused();
	virtual bool isStopped();
	void setLoop(bool loop);
	bool isLooping();
	virtual long getSize() { return 0; }
	virtual char* getBuffer() { return 0; }

protected:
	bool loop = false;
	float volume = 1.0f;
	OpenSLPlayer* player = NULL;
	OpenSLContext* context = NULL;
};

#endif //JAPPSY_UOPENSLSOUND_H
