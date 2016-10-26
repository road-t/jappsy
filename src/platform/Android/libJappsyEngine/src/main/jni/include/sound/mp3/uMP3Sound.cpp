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

#include "uMP3Sound.h"

#ifdef __cplusplus
extern "C" {
#endif

bool is_mp3(uint32_t sig) {
	uint32_t tmp = __builtin_bswap32(sig);
	if (((tmp & 0xFFE00000) == 0xFFE00000) && (tmp & (3 << 17)) && ((tmp & (3 << 10)) != (3 << 10))) {
		unsigned int layer, bridx, fridx;
		layer = 4 - ((tmp >> 17) & 3);
		bridx = ((tmp >> 12) & 0x0F);
		fridx = ((tmp >> 10) & 3);
		return !((fridx == 3) || (layer == 4) || ((layer != 3) && !bridx));
	}
	return false;
}

#ifdef __cplusplus
}
#endif

int mp3_available(MP3Sound* sound) {
	return 1;
}

bool mp3_createBuffer(MP3Sound* sound) {
	return true;
}

void mp3_deleteBuffer(MP3Sound* sound) {
}

const char formatDesc[] = "MP3 Reader";

static const tMPADecoderConfig defaultConfig = {
	MPADEC_CONFIG_FULL_QUALITY, MPADEC_CONFIG_AUTO,
	MPADEC_CONFIG_16BIT, MPADEC_CONFIG_LITTLE_ENDIAN,
	MPADEC_CONFIG_REPLAYGAIN_NONE, 1, 1, 1, 0.0
};

bool MP3Sound::mp3_openBuffer(MP3Sound* sound, Stream* stream) {
	// Инициализация струтур
	sound->mp3 = mp3dec_init();
	memcpy(&(sound->config),&defaultConfig,sizeof(struct tMPADecoderConfig));
	if (sound->mp3->configure(&(sound->config)) != 0) {
		mp3dec_uninit(sound->mp3); sound->mp3 = 0;
		return false;
	}
	try {
		sound->stream = stream->duplicate();
	} catch (...) {
		mp3dec_uninit(sound->mp3); sound->mp3 = 0;
		return false;
	}
	if (sound->mp3->init(sound->stream,0) != 0) {
		mp3dec_uninit(sound->mp3); sound->mp3 = 0;
		delete sound->stream; sound->stream = NULL;
		return false;
	}
	if (sound->mp3->get_info(&(sound->mpainfo), MPADEC_INFO_STREAM) != 0) {
		mp3dec_uninit(sound->mp3); sound->mp3 = 0;
		delete sound->stream; sound->stream = NULL;
		return false;
	}

	// генерация названия формата из полученных данных
	CString name;
	if (sound->mpainfo.frequency < 16000) name.concat(L"MPEG-2.5");
	else if (sound->mpainfo.frequency < 32000) name.concat(L"MPEG-2"); else name.concat(L"MPEG-1");
	name.concat(L" ");
	if (sound->mpainfo.layer == 1) name.concat(L"Layer I");
	else if (sound->mpainfo.layer == 2) name.concat(L"Layer II"); else name.concat(L"Layer III");
	name.concat(L", ");
	if (sound->mpainfo.channels > 1) name.concat(L"Stereo"); else name.concat(L"Mono");
	name.concat(CString::format(L", %d kbps, %d Hz, %d:%02d", sound->mpainfo.bitrate, sound->mpainfo.frequency, sound->mpainfo.duration/60, sound->mpainfo.duration%60));
	sound->duration = (uint32_t)sound->mpainfo.duration * 1000;
	sound->name = name;

	sound->desc = formatDesc;

#ifdef DEBUG
	CString::format(L"MP3: %ls", (wchar_t*)name).log();
#endif

	// Заполняем структуру формата данных
	memset(&(sound->spec),0,sizeof(tAudioSpec));
	sound->spec.freq = (uint32_t)sound->mpainfo.frequency;
	sound->spec.format = AUDIO_S16LSB;
	sound->spec.channels = (uint8_t)sound->mpainfo.channels;
	sound->spec.samples = (uint16_t)(((sound->spec.freq>>2)+3)&~3); // минимум 1/4 секунды (word aligned)

	// Обновить размер фрагмета в байтах
	calculateAudioSpec(&(sound->spec));

	return true;
}

size_t MP3Sound::mp3_fillBuffer(MP3Sound* sound, void* stream, size_t len) {
	uint32_t bufused = 0;
	int r = sound->mp3->decode((uint8_t*)stream, (uint32_t)len, &bufused);
	if (r != 0) bufused = (uint32_t)-1;
	return (size_t)bufused;
}

void MP3Sound::mp3_resetBuffer(MP3Sound* sound) {
	sound->mp3->seek(0, MP3DEC_SEEK_START);
}

void MP3Sound::mp3_closeBuffer(MP3Sound* sound) {
	if (sound->mp3 != NULL) {
		mp3dec_uninit(sound->mp3);
		sound->mp3 = NULL;

		if (sound->stream != NULL) {
			delete sound->stream;
			sound->stream = NULL;
		}
	}
}

MP3Sound::MP3Sound() {
	available = (int (*)(void*))mp3_available;
	createBuffer = (bool (*)(void*))mp3_createBuffer;
	deleteBuffer = (void (*)(void*))mp3_deleteBuffer;
	openBuffer = (bool (*)(void*, Stream*))mp3_openBuffer;
	fillBuffer = (size_t (*)(void*, void*, size_t))mp3_fillBuffer;
	resetBuffer = (void (*)(void*))mp3_resetBuffer;
	closeBuffer = (void (*)(void*))mp3_closeBuffer;
}

MP3Sound::~MP3Sound() {
}
