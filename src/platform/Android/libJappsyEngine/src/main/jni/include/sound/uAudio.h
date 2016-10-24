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

#ifndef JAPPSY_UAUDIO_H
#define JAPPSY_UAUDIO_H

#include <platform.h>
#include <data/uString.h>
#include <data/uStream.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*AudioSpecCallback)(void* userData, uint8_t* stream, uint32_t len);

struct tAudioSpec {
	int32_t freq;			// Частота дискретизации DSP -- колличество семплов в секунду
	uint16_t format;		// Формат аудио данных
	uint8_t channels;		// Колличество каналов: 1 mono, 2 stereo
	uint8_t silence;		// Величина тишины
	uint32_t size;			// Размер аудио буфера в байтах
	uint16_t samples;		// Размер аудио буфера в семплах
//	uint16_t padding;		// Необходимо для внутренних вычислений
	// Функция вызываемая, когда устройство требует больше данных
	AudioSpecCallback callback;
	void *userdata;
};

#define AUDIO_U8		0x0008	/* Unsigned 8-bit samples */
#define AUDIO_S8		0x8008	/* Signed 8-bit samples */
#define AUDIO_U16LSB	0x0010	/* Unsigned 16-bit samples */
#define AUDIO_S16LSB	0x8010	/* Signed 16-bit samples */
#define AUDIO_U16MSB	0x1010	/* As above, but big-endian byte order */
#define AUDIO_S16MSB	0x9010	/* As above, but big-endian byte order */
#define AUDIO_U16		AUDIO_U16LSB
#define AUDIO_S16		AUDIO_S16LSB

struct tAudioConvert;

typedef void (*AudioConvertFilterCallback)(struct tAudioConvert* cvt, uint16_t format, uint8_t channels);

struct tAudioConvert {
	int32_t needed;			// Установлено в 1 если возможно преобразование
	uint16_t src_format;	// Исходный формат аудио
	uint16_t dst_format;	// Конечный формат аудио
	double rate_incr;		// Rate conversion increment
	uint8_t* buf;			// Буффер для хранения всех аудио данных
	uint32_t len;			// Длинна оригинального буфера
	uint32_t len_cvt;		// Длинна преобразованного буфера
	uint32_t len_mult;		// буффер должен быть размером len*len_mult
	double len_ratio; 		// Получаемый размер, конечный размер len*len_ratio
	AudioConvertFilterCallback filters[10];
	int32_t filter_index;	/* Current audio conversion function */
};

class AudioDevice : CObject {
public:
	// Название аудио устройства
	CString name;
	// Описание аудио устройства
	CString desc;

	// Функции устройства
	int (*available)(void* audio);
	bool (*createDevice)(void* audio);
	void (*deleteDevice)(void* audio);
	int (*deviceCount)(void* audio);
	char* (*getDeviceName)(void* audio, int devindex);
	int (*openAudio)(void* audio, struct tAudioSpec* spec, int devindex);
//	void (*threadInit)(void* audio);
//	void (*waitAudio)(void* audio);
//	void (*playAudio)(void* audio);
//	unsigned char* (*getAudioBuf)(void* audio);
//	void (*waitDone)(void* audio);
	void (*closeAudio)(void* audio);

	void* (*createSound)(void* audio, Stream* sound);
	void (*freeSound)(void* audio, void* sound);
	void (*playSound)(void* audio, void* sound, bool loop, bool reset);
	void (*volSound)(void* audio, void* sound, int volume);
	void (*pauseSound)(void* audio, void* sound, bool pause);
	void (*stopSound)(void* audio, void* sound);
	bool (*isPlaying)(void* audio, void* sound);

	// Формат аудио данных
	struct tAudioSpec spec;
	// Фильтры преобразования
	struct tAudioConvert convert;

	// Семафор для блокировки буферов микшера
	jlock mixer_lock;

	// Флаги текущего состояния
	int enabled;
	int opened;
	int paused;
};

#define AUDIO_STOPPED	0
#define AUDIO_PLAYING	1

class Audio : CObject {
public:
	AudioDevice* audioDevice;
	int init();
	int open(struct tAudioSpec *desired, struct tAudioSpec *obtained, int devindex);
	int status();
	void close();
	void quit();
	void lock();
	void unlock();

	unsigned long fakeLoad(Stream* sound);
	void* load(Stream* sound);
	void free(void* sound);
	void play(void* sound, bool loop = false, int volume = -1, bool unpause = false);
	void volume(void* sound, int volume = -1);
	void pause(void* sound, bool pause);
	void stop(void* sound);
	bool isPlaying(void* sound);

	Audio();
	~Audio();
};

void calculateAudioSpec(struct tAudioSpec *spec);

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_UAUDIO_H
