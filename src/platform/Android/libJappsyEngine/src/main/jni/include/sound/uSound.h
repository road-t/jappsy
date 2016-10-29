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

#ifndef JAPPSY_USOUND_H
#define JAPPSY_USOUND_H

#include <platform.h>
#include <data/uObject.h>
#include <data/uString.h>
#include <data/uStream.h>
#include <sound/uAudioConverter.h>

class Sound : public CObject {
protected:
	Stream* stream = NULL;				// входной поток данных

public:
	// Название аудио формата
	CString name;
	// Описание аудио формата
	CString desc;

	// Функции аудио буффера
	int (*available)(void* sound) = NULL;
	bool (*createBuffer)(void* sound) = NULL; // создать буффер
	void (*deleteBuffer)(void* sound) = NULL; // удалить буффер
	bool (*openBuffer)(void* sound, Stream* stream) = NULL; // открыть буффер из файла или памяти
	size_t (*fillBuffer)(void* sound, void* stream, size_t len) = NULL; // заполнить буффер данными
	void (*resetBuffer)(void* sound); // сбросить буффер в начало
	void (*closeBuffer)(void* sound) = NULL; // закрыть буффер

	uint8_t *buffer = NULL;			// буффер содержащий уже считанные данные из (файла/памяти)
	size_t bufferSize = 0;			// размер буффера

	// Длительность в милисекундах
	uint32_t duration = 0;
	uint32_t totalSamples = 0;

	// Фильтры преобразования входных данных при считывании в выходные данные в буффере
	AudioConverter converter;
};

#endif //JAPPSY_USOUND_H
