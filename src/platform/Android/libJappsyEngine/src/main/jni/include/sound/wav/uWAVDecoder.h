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

#ifndef JAPPSY_UWAVDECODER_H
#define JAPPSY_UWAVDECODER_H

#include <platform.h>
#include <data/uStream.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WAVDEC_RIFF				0x46464952		/* "RIFF" */
#define WAVDEC_WAVE				0x45564157		/* "WAVE" */
#define WAVDEC_FACT				0x74636166		/* "fact" */
#define WAVDEC_LIST				0x5453494c		/* "LIST" */
#define WAVDEC_FMT				0x20746D66		/* "fmt " */
#define WAVDEC_DATA				0x61746164		/* "data" */
#define WAVDEC_PCM_CODE			0x0001
#define WAVDEC_MS_ADPCM_CODE	0x0002
#define WAVDEC_IMA_ADPCM_CODE	0x0011
#define WAVDEC_MP3_CODE			0x0055
#define WAVDEC_PCM_EXT_CODE		0xFFFE
#define WAVDEC_WAVE_MONO		1
#define WAVDEC_WAVE_STEREO		2

struct tWAVFormat {
	uint16_t encoding;
	uint16_t channels;		// 1 = mono, 2 = stereo
	uint32_t frequency;		// одно из 11025, 22050, или 44100 Гц
	uint32_t byterate;		// Колличество байт в секунду
	uint16_t blockalign;	// Размер одного блока
	uint16_t bitspersample;	// одно из 8, 12, 16, или 4 для ADPCM
};

struct tWAVChunk {
	uint32_t magic;
	uint32_t length;
	uint8_t *data;
};

struct tMSADPCMDecodeState {
	uint8_t hPredictor;
	uint16_t iDelta;
	int16_t iSamp1;
	int16_t iSamp2;
};

struct tMSADPCMDecoder {
	struct tWAVFormat wavefmt;
	uint16_t wSamplesPerBlock;
	uint16_t wNumCoef;
	int16_t aCoeff[7][2];
	struct tMSADPCMDecodeState states[2];
	Stream* stream;
	int8_t stereo;
	uint8_t* buffer;
	uint32_t buffer_size;
	uint32_t buffer_ofs;
	struct tMSADPCMDecodeState* state[2];
	int init(struct tWAVFormat* format, uint32_t length, Stream* stream);
	int32_t nibble(struct tMSADPCMDecodeState *state, uint8_t nybble, int16_t *coeff);
	int decode(uint8_t *buf, uint32_t bufsize, uint32_t *used);
	void free();
};

struct tIMAADPCMDecodeState {
	int32_t sample;
	int8_t index;
};

struct tIMAADPCMDecoder {
	struct tWAVFormat wavefmt;
	uint16_t wSamplesPerBlock;
	struct tIMAADPCMDecodeState states[2];
	Stream* stream;
	int8_t stereo;
	uint8_t* buffer;
	uint32_t buffer_size;
	uint32_t buffer_ofs;
	struct tIMAADPCMDecodeState* state;
	int init(struct tWAVFormat* format, uint32_t length, Stream* stream);
	int32_t nibble(struct tIMAADPCMDecodeState *state, uint8_t nybble);
	int fill_block(uint8_t *decoded, int channel, int numchannels, struct tIMAADPCMDecodeState *state);
	int decode(uint8_t *buf, uint32_t bufsize, uint32_t *used);
	void free();
};

// PCM format (8 / 16 bit)
// Microsoft ADPCM format (4 bit)
// International Multimedia Association ADPCM format (4 bit)
struct tWAVDecoder {
	struct tMSADPCMDecoder msadpcm;
	struct tIMAADPCMDecoder imaadpcm;
	Stream* stream;
	uint32_t stream_start;
	uint16_t encoding;
	uint32_t bitpersecond;
	uint32_t datalen;
	uint32_t duration;
	int init(Stream* stream, struct tAudioSpec* spec);
	int decode(uint8_t *buf, uint32_t bufsize, uint32_t *used);
	int reset(struct tAudioSpec* spec);
	void free();
};

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_UWAVDECODER_H
