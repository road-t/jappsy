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
#include <data/uStream.h>
#include <sound/uSound.h>

class OpenSLSound : public CObject {
private:
	Sound* sound = NULL;

public:
	OpenSLSound();
	~OpenSLSound();

	CString path;
	//bool load(const CString& path);
	bool load(Stream* stream);

	void setVolume(float volume);
	float getVolume();

	void play(OpenSLContext* context);
	void pause();
	void stop();

	bool isPlaying();
	bool isPaused();
	void setLoop(bool loop);
	bool isLooping();

	size_t memoryUsed();
	size_t audioSize();
	size_t fillBuffer(uint8_t* buffer, size_t bufferSize);
	void reset();

protected:
	jbool loop = false;
	jbool paused = false;
	jbool playing = false;
	jbool rewind = false;
	float volume = 1.0f;
	OpenSLPlayer* player = NULL;
	OpenSLContext* context = NULL;

	off_t lastConvertOfs = 0;
	size_t lastConvertSize = 0;
};

#endif //JAPPSY_UOPENSLSOUND_H
