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

#include "uAudioConverter.h"

AudioConverter::AudioConverter() {

}

AudioConverter::~AudioConverter() {
	if (formatBuffer != NULL) {
		memFree(formatBuffer);
		formatBuffer = NULL;
		formatBufferInLength = formatBufferOutLength = 0;
	}
}

bool AudioConverter::build(uint16_t outFormat, uint8_t outChannels, uint32_t outFreq) {
	convert = false;

	inSpec.sample = (uint32_t)(inSpec.channels * ((inSpec.format&0xFF) / 8));

	outSpec.frequency = outFreq;
	outSpec.format = outFormat;
	outSpec.channels = outChannels;
	outSpec.sample = (uint32_t)(outChannels * ((outFormat&0xFF) / 8));

	if (outFreq == inSpec.frequency && outFormat == inSpec.format && outChannels == inSpec.channels) {
		return true;
	}

	if (formatBuffer == NULL) {
		formatBuffer = memAlloc(uint8_t, formatBuffer, AUDIO_FORMAT_BUFFER_LENGTH);
		if (formatBuffer == NULL) {
			return false;
		}
	}

	formatBufferOutLength = AUDIO_FORMAT_BUFFER_LENGTH;

	int formatConverterIndex = 0;

	// Первый фильтр: порядок байт
	if ( (inSpec.format & 0x1000) != (outFormat & 0x1000)
		 && ((inSpec.format & 0xff) == 16) && ((outFormat & 0xff) == 16)) {
		formatConverter[formatConverterIndex++] = convertEndian;
	}

	// Второй фильтр: со знаком/без знака
	if ( (inSpec.format & 0x8000) != (outFormat & 0x8000) ) {
		formatConverter[formatConverterIndex++] = convertSign;
	}

	// Третий фильтр: 16 bit <--> 8 bit
	if ( (inSpec.format & 0xFF) != (outFormat & 0xFF) ) {
		switch (outFormat&0x10FF) {
			case AUDIO_FORMAT_U16LSB:
				formatConverter[formatConverterIndex++] = convert16LSB;
				break;
			case AUDIO_FORMAT_U16MSB:
				formatConverter[formatConverterIndex++] = convert16MSB;
				break;
			case AUDIO_FORMAT_U8:
			default:
				formatConverter[formatConverterIndex++] = convert8; formatBufferOutLength >>= 1;
				break;
		}
	}

	// Последний фильтр: Моно/Стерео
	if ( inSpec.channels != outChannels ) {
		uint8_t srcChannels = inSpec.channels;
		if ( (srcChannels == 1) && (outChannels > 1) ) {
			formatConverter[formatConverterIndex++] = convertStereo;
			srcChannels = 2;
		}
		if ( (srcChannels == 2) && (outChannels == 6) ) {
			formatConverter[formatConverterIndex++] = convertSurround51;
			srcChannels = 6;
		}
		if ( (srcChannels == 2) && (outChannels == 4) ) {
			formatConverter[formatConverterIndex++] = convertSurround40;
			srcChannels = 4;
		}
		while ( (srcChannels*2) <= outChannels ) {
			formatConverter[formatConverterIndex++] = convertStereo;
			srcChannels *= 2;
		}
		if ( (srcChannels == 6) && (outChannels <= 2) ) {
			formatConverter[formatConverterIndex++] = convertStrip;
			formatBufferOutLength /= 3;
			srcChannels = 2;
		}
		if ( (srcChannels == 6) && (outChannels == 4) ) {
			formatConverter[formatConverterIndex++] = convertStrip2;
			formatBufferOutLength = (formatBufferOutLength / 6) * 4;
			srcChannels = 4;
		}
		// Предположим что 4 канальные данные хранятся в формате
		// Левый {перед/зад} + Правый {перед/зад}
		while ( ((srcChannels%2) == 0) && ((srcChannels/2) >= outChannels) ) {
			formatConverter[formatConverterIndex++] = convertMono;
			srcChannels /= 2;
			formatBufferOutLength >>= 1;
		}
		// if ( src_channels != dst_channels ) // такого не должно быть со стандартным звуком
	}

	// Усекаем размер выходного буфера по размеру семпла
	formatBufferOutLength -= (formatBufferOutLength % outSpec.sample);

	// Вычисляем размер входного буфера по числу выходных семплов
	formatBufferInLength = (formatBufferOutLength / outSpec.sample) * inSpec.sample;

	if ( formatConverterIndex != 0 ) {
		convert = true;
	}
	formatConverter[formatConverterIndex] = NULL;

	return true;
}

size_t AudioConverter::process(size_t filled, size_t& used) {
	if (convert && filled > 0) {
		if (formatConverter[0] != NULL) {
			used = filled - (filled % inSpec.sample);
			formatConverter[0](this, 0, inSpec.format, inSpec.channels, used);
			return (used / inSpec.sample) * outSpec.sample;
		}
	}

	return 0;
}

void AudioConverter::convertEndian(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength) {
	uint16_t* data = (uint16_t*)context->formatBuffer;
	for ( size_t i = currentLength / 2 - 1; i >= 0; i-- ) {
		data[i] = __builtin_bswap16(data[i]);
	}
	currentFormat = (uint16_t)(currentFormat ^ 0x1000);
	if ( context->formatConverter[++converterIndex] ) {
		context->formatConverter[converterIndex](context, converterIndex, currentFormat, currentChannels, currentLength);
	}
}

void AudioConverter::convertSign(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength) {
	uint8_t* data = context->formatBuffer;
	if ( (currentFormat & 0xFF) == 16 ) {
		if ( (currentFormat & 0x1000) != 0x1000 ) ++data;
		for ( size_t i = currentLength / 2 - 1; i >= 0; i-- ) {
			data[i*2] ^= 0x80;
		}
	} else {
		for ( size_t i = currentLength - 1; i >= 0; i-- ) {
			data[i] ^= 0x80;
		}
	}
	currentFormat = (uint16_t)(currentFormat ^ 0x8000);
	if ( context->formatConverter[++converterIndex] ) {
		context->formatConverter[converterIndex](context, converterIndex, currentFormat, currentChannels, currentLength);
	}
}

// Конвертировать 8-bit в 16-bit - LSB
void AudioConverter::convert16LSB(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength) {
	uint8_t* data = context->formatBuffer;
	for ( size_t i = currentLength / 2 - 1; i >= 0; i-- ) {
		data[i*2] = 0;
		data[i*2+1] = data[i];
	}
	currentFormat = (uint16_t)((currentFormat & ~0x0008) | AUDIO_FORMAT_U16LSB);
	if ( context->formatConverter[++converterIndex] ) {
		context->formatConverter[converterIndex](context, converterIndex, currentFormat, currentChannels, currentLength << 1);
	}
}

// Конвертировать 8-bit в 16-bit - MSB
void AudioConverter::convert16MSB(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength) {
	uint8_t* data = context->formatBuffer;
	for ( size_t i = currentLength / 2 - 1; i >= 0; i-- ) {
		data[i*2] = data[i];
		data[i*2+1] = 0;
	}
	currentFormat = (uint16_t)((currentFormat & ~0x0008) | AUDIO_FORMAT_U16MSB);
	if ( context->formatConverter[++converterIndex] ) {
		context->formatConverter[converterIndex](context, converterIndex, currentFormat, currentChannels, currentLength << 1);
	}
}

// Конвертировать 16-bit и 8-bit
void AudioConverter::convert8(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength) {
	uint8_t* data = context->formatBuffer;
	if ( (currentFormat & 0x1000) != 0x1000 ) {
		for ( size_t i = 0; i < (currentLength / 2 - 1); i++ ) {
			data[i] = data[i*2+1];
		}
	} else {
		for ( size_t i = 0; i < (currentLength / 2 - 1); i++ ) {
			data[i] = data[i*2];
		}
	}
	currentFormat = (uint16_t)((currentFormat & ~0x9010) | AUDIO_FORMAT_U8);
	if ( context->formatConverter[++converterIndex] ) {
		context->formatConverter[converterIndex](context, converterIndex, currentFormat, currentChannels, currentLength >> 1);
	}
}

// Дублировать моно канал на левый и правый
void AudioConverter::convertStereo(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength) {
	if ( (currentFormat & 0xFF) == 16 ) {
		uint16_t* data = (uint16_t*)context->formatBuffer;
		for ( size_t i = currentLength / 2 - 1; i >= 0; i-- ) {
			data[i*2+1] = data[i*2] = data[i];
		}
	} else {
		uint8_t* data = context->formatBuffer;
		for ( size_t i = currentLength - 1; i >= 0; i-- ) {
			data[i*2+1] = data[i*2] = data[i];
		}
	}

	if ( context->formatConverter[++converterIndex] ) {
		context->formatConverter[converterIndex](context, converterIndex, currentFormat, 2, currentLength << 1);
	}
}

// Дублировать стерео в псевдостерео-4.0
void AudioConverter::convertSurround40(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength) {
	switch (currentFormat&0x8018) {
		case AUDIO_FORMAT_U8: {
			uint8_t* data = context->formatBuffer;
			for ( size_t i = currentLength / 2 - 1; i >= 0; i-- ) {
				uint8_t lf = data[i*2];
				uint8_t rf = data[i*2+1];
				uint8_t ce = (uint8_t)((lf/2) + (rf/2));
				data[i*4] = lf;
				data[i*4+1] = rf;
				data[i*4+2] = lf - ce;
				data[i*4+3] = rf - ce;
			}
			break;
		}

		case AUDIO_FORMAT_S8: {
			int8_t* data = (int8_t*)context->formatBuffer;
			for ( size_t i = currentLength / 2 - 1; i >= 0; i-- ) {
				int8_t lf = data[i*2];
				int8_t rf = data[i*2+1];
				int8_t ce = (int8_t)((lf/2) + (rf/2));
				data[i*4] = lf;
				data[i*4+1] = rf;
				data[i*4+2] = lf - ce;
				data[i*4+3] = rf - ce;
			}
			break;
		}

		case AUDIO_FORMAT_U16: {
			uint16_t* data = (uint16_t*)context->formatBuffer;
			if ( (currentFormat & 0x1000) == 0x1000 ) {
				for ( size_t i = currentLength / 4 - 1; i >= 0; i-- ) {
					uint16_t lf = __builtin_bswap16(data[i*2]);
					uint16_t rf = __builtin_bswap16(data[i*2+1]);
					uint16_t ce = (uint16_t)((lf/2) + (rf/2));
					data[i*4] = __builtin_bswap16(lf);
					data[i*4+1] = __builtin_bswap16(rf);
					data[i*4+2] = __builtin_bswap16(lf - ce);
					data[i*4+3] = __builtin_bswap16(rf - ce);
				}
			} else {
				for ( size_t i = currentLength / 4 - 1; i >= 0; i-- ) {
					uint16_t lf = data[i*2];
					uint16_t rf = data[i*2+1];
					uint16_t ce = (uint16_t)((lf/2) + (rf/2));
					data[i*4] = lf;
					data[i*4+1] = rf;
					data[i*4+2] = lf - ce;
					data[i*4+3] = rf - ce;
				}
			}
			break;
		}

		case AUDIO_FORMAT_S16: {
			int16_t* data = (int16_t*)context->formatBuffer;
			if ( (currentFormat & 0x1000) == 0x1000 ) {
				for ( size_t i = currentLength / 4 - 1; i >= 0; i-- ) {
					int16_t lf = __builtin_bswap16(data[i*2]);
					int16_t rf = __builtin_bswap16(data[i*2+1]);
					int16_t ce = (int16_t)((lf/2) + (rf/2));
					data[i*4] = __builtin_bswap16(lf);
					data[i*4+1] = __builtin_bswap16(rf);
					data[i*4+2] = __builtin_bswap16(lf - ce);
					data[i*4+3] = __builtin_bswap16(rf - ce);
				}
			} else {
				for ( size_t i = currentLength / 4 - 1; i >= 0; i-- ) {
					int16_t lf = data[i*2];
					int16_t rf = data[i*2+1];
					int16_t ce = (int16_t)((lf/2) + (rf/2));
					data[i*4] = lf;
					data[i*4+1] = rf;
					data[i*4+2] = lf - ce;
					data[i*4+3] = rf - ce;
				}
			}
			break;
		}

		default:;
	}

	if ( context->formatConverter[++converterIndex] ) {
		context->formatConverter[converterIndex](context, converterIndex, currentFormat, 4, currentLength << 1);
	}
}

// Дублировать стерео в псевдостерео-5.1
void AudioConverter::convertSurround51(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength) {
	switch (currentFormat&0x8018) {
		case AUDIO_FORMAT_U8: {
			uint8_t* data = context->formatBuffer;
			for ( size_t i = currentLength / 2 - 1; i >= 0; i-- ) {
				uint8_t lf = data[i*2];
				uint8_t rf = data[i*2+1];
				uint8_t ce = (uint8_t)((lf/2) + (rf/2));
				data[i*6] = lf;
				data[i*6+1] = rf;
				data[i*6+2] = lf - ce;
				data[i*6+3] = rf - ce;
				data[i*6+4] = data[i*6+5] = ce;
			}
			break;
		}

		case AUDIO_FORMAT_S8: {
			int8_t* data = (int8_t*)context->formatBuffer;
			for ( size_t i = currentLength / 2 - 1; i >= 0; i-- ) {
				int8_t lf = data[i*2];
				int8_t rf = data[i*2+1];
				int8_t ce = (int8_t)((lf/2) + (rf/2));
				data[i*6] = lf;
				data[i*6+1] = rf;
				data[i*6+2] = lf - ce;
				data[i*6+3] = rf - ce;
				data[i*6+4] = data[i*6+5] = ce;
			}
			break;
		}

		case AUDIO_FORMAT_U16: {
			uint16_t* data = (uint16_t*)context->formatBuffer;
			if ( (currentFormat & 0x1000) == 0x1000 ) {
				for ( size_t i = currentLength / 4 - 1; i >= 0; i-- ) {
					uint16_t lf = __builtin_bswap16(data[i*2]);
					uint16_t rf = __builtin_bswap16(data[i*2+1]);
					uint16_t ce = (uint16_t)((lf/2) + (rf/2));
					data[i*6] = __builtin_bswap16(lf);
					data[i*6+1] = __builtin_bswap16(rf);
					data[i*6+2] = __builtin_bswap16(lf - ce);
					data[i*6+3] = __builtin_bswap16(rf - ce);
					data[i*6+4] = data[i*6+5] = __builtin_bswap16(ce);
				}
			} else {
				for ( size_t i = currentLength / 4 - 1; i >= 0; i-- ) {
					uint16_t lf = data[i*2];
					uint16_t rf = data[i*2+1];
					uint16_t ce = (uint16_t)((lf/2) + (rf/2));
					data[i*6] = lf;
					data[i*6+1] = rf;
					data[i*6+2] = lf - ce;
					data[i*6+3] = rf - ce;
					data[i*6+4] = data[i*6+5] = ce;
				}
			}
			break;
		}

		case AUDIO_FORMAT_S16: {
			int16_t* data = (int16_t*)context->formatBuffer;
			if ( (currentFormat & 0x1000) == 0x1000 ) {
				for ( size_t i = currentLength / 4 - 1; i >= 0; i-- ) {
					int16_t lf = __builtin_bswap16(data[i*2]);
					int16_t rf = __builtin_bswap16(data[i*2+1]);
					int16_t ce = (int16_t)((lf/2) + (rf/2));
					data[i*6] = __builtin_bswap16(lf);
					data[i*6+1] = __builtin_bswap16(rf);
					data[i*6+2] = __builtin_bswap16(lf - ce);
					data[i*6+3] = __builtin_bswap16(rf - ce);
					data[i*6+4] = data[i*6+5] = __builtin_bswap16(ce);
				}
			} else {
				for ( size_t i = currentLength / 4 - 1; i >= 0; i-- ) {
					int16_t lf = data[i*2];
					int16_t rf = data[i*2+1];
					int16_t ce = (int16_t)((lf/2) + (rf/2));
					data[i*6] = lf;
					data[i*6+1] = rf;
					data[i*6+2] = lf - ce;
					data[i*6+3] = rf - ce;
					data[i*6+4] = data[i*6+5] = ce;
				}
			}
			break;
		}

		default:;
	}

	if ( context->formatConverter[++converterIndex] ) {
		context->formatConverter[converterIndex](context, converterIndex, currentFormat, 6, currentLength * 3);
	}
}

// Эффективное сложение правого и левого каналов в один
void AudioConverter::convertMono(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength) {
	switch (currentFormat&0x8018) {
		case AUDIO_FORMAT_U8: {
			uint8_t* data = context->formatBuffer;
			for ( size_t i = 0; i < (currentLength / 2); i++ ) {
				data[i] = (uint8_t)((int32_t)((uint32_t)data[i*2] + (uint32_t)data[i*2+1]) / 2);
			}
			break;
		}

		case AUDIO_FORMAT_S8: {
			int8_t* data = (int8_t*)context->formatBuffer;
			for ( size_t i = 0; i < (currentLength / 2); i++ ) {
				data[i] = (int8_t)((int32_t)((int32_t)data[i*2] + (int32_t)data[i*2+1]) / 2);
			}
			break;
		}

		case AUDIO_FORMAT_U16: {
			uint16_t* data = (uint16_t*)context->formatBuffer;
			if ( (currentFormat & 0x1000) == 0x1000 ) {
				for ( size_t i = 0; i < (currentLength / 4); i++ ) {
					data[i] = __builtin_bswap16((uint16_t)((int32_t)((uint32_t)__builtin_bswap16(data[i*2]) + (uint32_t)__builtin_bswap16(data[i*2+1])) / 2));
				}
			} else {
				for ( size_t i = 0; i < (currentLength / 4); i++ ) {
					data[i] = (uint16_t)((int32_t)((uint32_t)data[i*2] + (uint32_t)data[i*2+1]) / 2);
				}
			}
			break;
		}

		case AUDIO_FORMAT_S16: {
			int16_t* data = (int16_t*)context->formatBuffer;
			if ( (currentFormat & 0x1000) == 0x1000 ) {
				for ( size_t i = 0; i < (currentLength / 4); i++ ) {
					data[i] = __builtin_bswap16((int16_t)((int32_t)((int32_t)__builtin_bswap16(data[i*2]) + (int32_t)__builtin_bswap16(data[i*2+1])) / 2));
				}
			} else {
				for ( size_t i = 0; i < (currentLength / 4); i++ ) {
					data[i] = (int16_t)((int32_t)((int32_t)data[i*2] + (int32_t)data[i*2+1]) / 2);
				}
			}
			break;
		}

		default:;
	}

	if ( context->formatConverter[++converterIndex] ) {
		context->formatConverter[converterIndex](context, converterIndex, currentFormat, 1, currentLength >> 1);
	}
}

// Обрезать 4 канала из 6, кроме передних левого и правого
void AudioConverter::convertStrip(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength) {
	switch (currentFormat&0x8018) {
		case AUDIO_FORMAT_U8:
		case AUDIO_FORMAT_S8: {
			uint8_t* data = context->formatBuffer;
			for ( size_t i = 0; i < (currentLength / 6); i++) {
				data[i*2] = data[i*6];
				data[i*2+1] = data[i*6+1];
			}
			break;
		}

		case AUDIO_FORMAT_U16:
		case AUDIO_FORMAT_S16: {
			uint16_t* data = (uint16_t*)context->formatBuffer;
			for ( size_t i = 0; i < (currentLength / 12); i++) {
				data[i*2] = data[i*6];
				data[i*2+1] = data[i*6+1];
			}
			break;
		}

		default:;
	}

	if ( context->formatConverter[++converterIndex] ) {
		context->formatConverter[converterIndex](context, converterIndex, currentFormat, 2, currentLength / 3);
	}
}

//  Обрезать 2 канала из 6
void AudioConverter::convertStrip2(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength) {
	switch (currentFormat&0x8018) {
		case AUDIO_FORMAT_U8:
		case AUDIO_FORMAT_S8: {
			uint8_t* data = context->formatBuffer;
			for ( size_t i = 0; i < (currentLength / 6); i++) {
				data[i*4] = data[i*6];
				data[i*4+1] = data[i*6+1];
				data[i*4+2] = data[i*6+2];
				data[i*4+3] = data[i*6+3];
			}
			break;
		}

		case AUDIO_FORMAT_U16:
		case AUDIO_FORMAT_S16: {
			uint16_t* data = (uint16_t*)context->formatBuffer;
			for ( size_t i = 0; i < (currentLength / 12); i++) {
				data[i*4] = data[i*6];
				data[i*4+1] = data[i*6+1];
				data[i*4+2] = data[i*6+2];
				data[i*4+3] = data[i*6+3];
			}
			break;
		}

		default:;
	}

	if ( context->formatConverter[++converterIndex] ) {
		context->formatConverter[converterIndex](context, converterIndex, currentFormat, 4, (currentLength / 6) * 4);
	}
}
