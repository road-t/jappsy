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

#ifndef uMixerH
#define uMixerH

#include <platform.h>
#include <data/uString.h>

#ifdef __cplusplus
extern "C" {
#endif
	
	void initAudioPlayer();
	bool shutdownAudioPlayer();
	
	void pauseAudioPlayer();
	void resumeAudioPlayer();
	
	typedef void (*AudioReadyCallback)(void* audioHandle, uint32_t bufferSize, uint64_t audioSize, void* userData);
	typedef void (*AudioErrorCallback)(void* userData);
	
	bool prepareAudio(const CString& filePath, void* userData, bool threaded, AudioReadyCallback onAudioReady, AudioErrorCallback onAudioError);
	void destroyAudio(void* audioHandle);
	void playAudio(void* audioHandle, bool loop);
	bool playingAudio(void* audioHandle);
	void volumeAudio(void* audioHandle, float volume);
	bool pauseAudio(void* audioHandle);
	void stopAudio(void* audioHandle);
	
	int32_t freeChannels();
	int32_t activeChannels();
	
#ifdef __cplusplus
}
#endif

#endif //uMixerH
