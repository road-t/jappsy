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

#ifndef JAPPSY_UOPENSLCONTEXT_H
#define JAPPSY_UOPENSLCONTEXT_H

#include <platform.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <data/uObject.h>

class OpenSLPlayer;
class OpenSLSound;

#define OPENSL_CHANNELS		16

class OpenSLContext : public CObject {
private:
	jbool paused = false;

public:
	SLObjectItf engineObj = NULL;
	SLEngineItf engine = NULL;
	SLObjectItf outputMixObj = NULL;

	OpenSLPlayer* players[OPENSL_CHANNELS] = {0};

	bool init();
	void shutdown();

	bool isPaused();

	void pause();
	void resume();

	OpenSLPlayer* getFreePlayer(OpenSLSound* sound);
};

#endif //JAPPSY_UOPENSLCONTEXT_H
