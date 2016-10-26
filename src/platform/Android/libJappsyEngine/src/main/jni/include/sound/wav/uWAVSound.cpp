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

#include "uWAVSound.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

int wav_available(WAVSound* sound) {
	return 1;
}

bool wav_createBuffer(WAVSound* sound) {
	return true;
}

void wav_deleteBuffer(WAVSound* sound) {
}

const char formatDesc[] = "WAVE/RIFF Reader";

bool WAVSound::wav_openBuffer(WAVSound* sound, Stream* stream) {
	// Инициализация струтур
	sound->wav = memAlloc(struct tWAVDecoder, sound->wav, sizeof(struct tWAVDecoder));
	if (sound->wav == NULL) {
		return -1;
	}
	memset(sound->wav,0,sizeof(struct tWAVDecoder));
	memset(&(sound->spec),0,sizeof(struct tAudioSpec));
	try {
		sound->stream = stream->duplicate();
	} catch (...) {
		memFree(sound->wav); sound->wav = NULL;
		return -1;
	}
	if (sound->wav->init(sound->stream,&(sound->spec)) != 0) {
		memFree(sound->wav); sound->wav = NULL;
		delete sound->stream; sound->stream = NULL;
		return -1;
	}

	// генерация названия формата из полученных данных
	CString name;
	switch (sound->wav->encoding) {
		case WAVDEC_MS_ADPCM_CODE:
			name.concat(L"MS-ADPCM");
			break;
		case WAVDEC_IMA_ADPCM_CODE:
			name.concat(L"IMA-ADPCM");
			break;
		default:
			name.concat(L"PCM");
			break;
	}
	name.concat(L", ");
	if (sound->spec.channels > 1) name.concat(L"Stereo"); else name.concat(L"Mono");
	name.concat(CString::format(L", %d kbps, %d Hz, %d:%02d", sound->wav->bitpersecond, sound->spec.freq, sound->wav->duration/60, sound->wav->duration%60));
	sound->duration = sound->wav->duration * 1000;
	sound->name = name;
	sound->desc = formatDesc;

	// Обновить размер фрагмета в байтах
	calculateAudioSpec(&(sound->spec));

	return 0;
}

size_t WAVSound::wav_fillBuffer(WAVSound* sound, void* stream, size_t len) {
	uint32_t bufused = 0;
	int r = sound->wav->decode((uint8_t*)stream, len, &bufused);
	if (r != 0) bufused = (uint32_t)-1;
	return bufused;
}

void WAVSound::wav_resetBuffer(WAVSound* sound) {
	sound->wav->reset(&(sound->spec));
	calculateAudioSpec(&(sound->spec));
}

void WAVSound::wav_closeBuffer(WAVSound* sound) {
	if (sound->wav != NULL) {
		sound->wav->free();
		memFree(sound->wav);
		sound->wav = NULL;

		if (sound->stream != NULL) {
			delete sound->stream;
			sound->stream = NULL;
		}
	}
}

WAVSound::WAVSound() {
	available = (int (*)(void*))wav_available;
	createBuffer = (bool (*)(void*))wav_createBuffer;
	deleteBuffer = (void (*)(void*))wav_deleteBuffer;
	openBuffer = (bool (*)(void*, Stream*))wav_openBuffer;
	fillBuffer = (size_t (*)(void*, void*, size_t))wav_fillBuffer;
	resetBuffer = (void (*)(void*))wav_resetBuffer;
	closeBuffer = (void (*)(void*))wav_closeBuffer;
}

WAVSound::~WAVSound() {
}