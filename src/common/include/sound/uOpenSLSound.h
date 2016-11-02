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

class OpenSLSound : public CObject {
public:
	OpenSLSound();
	~OpenSLSound();

	bool load(const CString& path);

	void setVolume(float volume);
	float getVolume();

	void play(OpenSLContext* context, bool loop);
	void pause();
	void stop();

	bool isPlaying();
	bool isPaused();

	void reset();

	int fd = -1;
	off_t fileSize = 0;

protected:
	jbool paused = false;
	jbool playing = false;
	jbool rewind = false;
	float volume = 1.0f;

	OpenSLPlayer* player = NULL;
	OpenSLContext* context = NULL;

private:
	void clear();
};

#endif //JAPPSY_UOPENSLSOUND_H
