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

#ifndef JAPPSY_UWAVSOUND_H
#define JAPPSY_UWAVSOUND_H

#include <platform.h>
#include <sound/uSound.h>
#include <sound/wav/uWAVDecoder.h>

#ifdef __cplusplus
extern "C" {
#endif

bool is_wav(uint32_t sig);

#ifdef __cplusplus
}
#endif

class WAVSound : public Sound {
protected:
	struct tWAVDecoder* wav;

private:
	static bool wav_openBuffer(WAVSound* sound, Stream* stream);
	static size_t wav_fillBuffer(WAVSound* sound, void* stream, size_t len);
	static void wav_resetBuffer(WAVSound* sound);
	static void wav_closeBuffer(WAVSound* sound);

public:
	WAVSound();
	~WAVSound();
};


#endif //JAPPSY_UWAVSOUND_H
