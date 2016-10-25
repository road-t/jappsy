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

#ifndef JAPPSY_UMP3SOUND_H
#define JAPPSY_UMP3SOUND_H

#include <platform.h>
#include <sound/uSound.h>
#include <sound/mp3/uMP3Decoder.h>

#ifdef __cplusplus
extern "C" {
#endif

bool is_mp3(uint32_t sig);

#ifdef __cplusplus
}
#endif

class MP3Sound : public Sound {
protected:
	struct tMP3Decoder* mp3 = NULL;
	struct tMPADecoderConfig config;
	struct tMPADecoderInfo mpainfo;

private:
	static bool mp3_openBuffer(MP3Sound* sound, Stream* stream);
	static size_t mp3_fillBuffer(MP3Sound* sound, void* stream, size_t len);
	static void mp3_closeBuffer(MP3Sound* sound);

public:
	MP3Sound();
	~MP3Sound();
};

#endif //JAPPSY_UMP3SOUND_H
