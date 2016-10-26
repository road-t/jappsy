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

#include "uWAVDecoder.h"
#include <sound/uAudioCVT.h>

#ifdef __cplusplus
extern "C" {
#endif

int tMSADPCMDecoder::init(struct tWAVFormat* format, uint32_t length, Stream* stream) {
	uint8_t *rogue_feel;
	uint16_t extra_info;
	int i;

	// Настройка структур
	wavefmt.encoding = format->encoding;
	wavefmt.channels = format->channels;
	wavefmt.frequency = format->frequency;
	wavefmt.byterate = format->byterate;
	wavefmt.blockalign = format->blockalign;
	wavefmt.bitspersample = format->bitspersample;
	rogue_feel = (uint8_t*)format+sizeof(*format);
	if ( length == 16 ) {
		extra_info = ((rogue_feel[1]<<8)|rogue_feel[0]);
		rogue_feel += sizeof(uint16_t);
	}
	wSamplesPerBlock = ((rogue_feel[1]<<8)|rogue_feel[0]);
	rogue_feel += sizeof(uint16_t);
	wNumCoef = ((rogue_feel[1]<<8)|rogue_feel[0]);
	rogue_feel += sizeof(uint16_t);
	if ( wNumCoef != 7 ) {
		// SetError("Unknown set of MS_ADPCM coefficients");
		return -1;
	}
	for ( i=0; i < wNumCoef; ++i ) {
		aCoeff[i][0] = ((rogue_feel[1]<<8)|rogue_feel[0]);
		rogue_feel += sizeof(uint16_t);
		aCoeff[i][1] = ((rogue_feel[1]<<8)|rogue_feel[0]);
		rogue_feel += sizeof(uint16_t);
	}

	// Подготовка структур для декодирования
	stereo = (wavefmt.channels == 2);
	state[0] = &states[0];
	state[1] = &states[stereo];

	// Выделение памяти для внутреннего буфера
	buffer_size = (wavefmt.channels == 2) ? 8 : 4;
	buffer_size += ((wSamplesPerBlock-2) * wavefmt.channels) * 2;
	buffer = memAlloc(uint8_t, buffer, buffer_size);
	if (buffer == 0) return -1;
	buffer_ofs = 0;

	this->stream = stream;

	return 0;
}

void tMSADPCMDecoder::free() {
	if (buffer) {
		memFree(buffer);
	}
}

int32_t tMSADPCMDecoder::nibble(tMSADPCMDecodeState *state, uint8_t nybble, int16_t *coeff) {
	static const int32_t max_audioval = ((1<<(16-1))-1);
	static const int32_t min_audioval = -(1<<(16-1));
	static const int32_t adaptive[] = {	230, 230, 230, 230, 307, 409, 512, 614,	768, 614, 512, 409, 307, 230, 230, 230 };
	int32_t new_sample, delta;

	new_sample = ((state->iSamp1 * coeff[0]) + (state->iSamp2 * coeff[1]))/256;
	if ( nybble & 0x08 )
		new_sample += state->iDelta * (nybble-0x10);
	else
		new_sample += state->iDelta * nybble;
	if ( new_sample < min_audioval )
		new_sample = min_audioval;
	else if ( new_sample > max_audioval )
		new_sample = max_audioval;
	delta = ((int32_t)state->iDelta * adaptive[nybble])/256;
	if ( delta < 16 ) delta = 16;
	state->iDelta = (uint16_t)delta;
	state->iSamp2 = state->iSamp1;
	state->iSamp1 = (int16_t)new_sample;
	return new_sample;
}

int tMSADPCMDecoder::decode(uint8_t *buf, uint32_t bufsize, uint32_t *used) {
	uint32_t bufused = 0;
	uint32_t buffersize = buffer_size;
	while (bufsize > 0) {
		if (buffer_ofs == 0) {
			if ( stream->getPosition() >= stream->getLimit() ) {
				if (used) *used = bufused;
				return 0;
			}
			uint8_t* decoded = buffer;
			int16_t *coeff[2];
			try {
				state[0]->hPredictor = stream->readU8();
				if (stereo) state[1]->hPredictor = stream->readU8();
				state[0]->iDelta = stream->readU16();
				if (stereo) state[1]->iDelta = stream->readU16();
				state[0]->iSamp1 = stream->readS16();
				if (stereo) state[1]->iSamp1 = stream->readS16();
				state[0]->iSamp2 = stream->readS16();
				if (stereo) state[1]->iSamp2 = stream->readS16();
			} catch (...) {
				return -1;
			}
			coeff[0] = aCoeff[state[0]->hPredictor];
			coeff[1] = aCoeff[state[1]->hPredictor];

			decoded[0] = (uint8_t)(state[0]->iSamp2&0xFF); decoded[1] = (uint8_t)(state[0]->iSamp2>>8); decoded += 2;
			if ( stereo ) {
				decoded[0] = (uint8_t)(state[1]->iSamp2&0xFF); decoded[1] = (uint8_t)(state[1]->iSamp2>>8); decoded += 2;
			}
			decoded[0] = (uint8_t)(state[0]->iSamp1&0xFF); decoded[1] = (uint8_t)(state[0]->iSamp1>>8); decoded += 2;
			if ( stereo ) {
				decoded[0] = (uint8_t)(state[1]->iSamp1&0xFF); decoded[1] = (uint8_t)(state[1]->iSamp1>>8); decoded += 2;
			}

			int32_t samplesleft = (wSamplesPerBlock-2) * wavefmt.channels;
			int8_t nybble;
			int32_t new_sample;
			while ( samplesleft > 0 ) {
				unsigned char in = 0;
				try {
					in = stream->readU8();
				} catch (...) {
					// исправление незаконченного конца
					buffersize -= (samplesleft * 2);
					samplesleft = 0;
					break;
				}

				nybble = in>>4;
				new_sample = nibble(state[0],(uint8_t)nybble,coeff[0]);
				decoded[0] = (uint8_t)(new_sample&0xFF);
				new_sample >>= 8;
				decoded[1] = (uint8_t)(new_sample&0xFF);
				decoded += 2;

				nybble = (int8_t)(in&0x0F);
				new_sample = nibble(state[1],(uint8_t)nybble,coeff[1]);
				decoded[0] = (uint8_t)(new_sample&0xFF);
				new_sample >>= 8;
				decoded[1] = (uint8_t)(new_sample&0xFF);
				decoded += 2;

				samplesleft -= 2;
			}
		}
		uint32_t size = bufsize;
		if ( ( buffer_ofs + size ) > buffersize ) {
			size = buffersize - buffer_ofs;
		}
		memcpy(buf,buffer,size);
		buffer_ofs += size; buf += size; bufsize -= size; bufused += size;
		if ( buffer_ofs >= buffersize ) {
			buffer_ofs -= buffersize;
		}
	}

	if (used) *used = bufused;
	return 0;
}

#define wavdec_sizeofarray(array)	(sizeof(array)/sizeof(array[0]))

int tIMAADPCMDecoder::init(struct tWAVFormat* format, uint32_t length, Stream* stream) {
	uint8_t *rogue_feel;
	uint16_t extra_info;

	// Настройка структур
	wavefmt.encoding = format->encoding;
	wavefmt.channels = format->channels;
	wavefmt.frequency = format->frequency;
	wavefmt.byterate = format->byterate;
	wavefmt.blockalign = format->blockalign;
	wavefmt.bitspersample = format->bitspersample;
	rogue_feel = (uint8_t*)format+sizeof(*format);
	if ( length == 16 ) {
		extra_info = ((rogue_feel[1]<<8)|rogue_feel[0]);
		rogue_feel += sizeof(uint16_t);
	}
	wSamplesPerBlock = ((rogue_feel[1]<<8)|rogue_feel[0]);

	// Подготовка структур для декодирования
	if ( wavefmt.channels > wavdec_sizeofarray(states) ) return -1;
	state = states;

	// Выделение памяти для внутреннего буфера
	buffer_size = (uint32_t)(wavefmt.channels) * 2;
	buffer_size += ((wSamplesPerBlock-1) * wavefmt.channels) * 2;
	buffer = memAlloc(uint8_t, buffer, buffer_size);
	if (buffer == 0) return -1;
	buffer_ofs = 0;

	this->stream = stream;

	return 0;
}

void tIMAADPCMDecoder::free() {
	if (buffer) {
		memFree(buffer);
	}
}

int32_t tIMAADPCMDecoder::nibble(tIMAADPCMDecodeState *state, uint8_t nybble) {
	static const int32_t max_audioval = ((1<<(16-1))-1);
	static const int32_t min_audioval = -(1<<(16-1));
	static const int index_table[16] = { -1, -1, -1, -1,  2,  4,  6,  8, -1, -1, -1, -1,  2,  4,  6,  8 };
	static const int32_t step_table[89] = {
			7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31,
			34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
			143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408,
			449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282,
			1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
			3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
			9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350,
			22385, 24623, 27086, 29794, 32767
	};
	int32_t delta, step;

	step = step_table[state->index];
	delta = step >> 3;
	if ( nybble & 0x04 ) delta += step;
	if ( nybble & 0x02 ) delta += (step >> 1);
	if ( nybble & 0x01 ) delta += (step >> 2);
	if ( nybble & 0x08 ) delta = -delta;
	state->sample += delta;

	state->index += index_table[nybble];
	if ( state->index > 88 )
		state->index = 88;
	else if ( state->index < 0 )
		state->index = 0;

	if ( state->sample > max_audioval )
		state->sample = max_audioval;
	else if ( state->sample < min_audioval )
		state->sample = min_audioval;

	return(state->sample);
}

int tIMAADPCMDecoder::fill_block(uint8_t *decoded, int channel, int numchannels, tIMAADPCMDecodeState *state) {
	int i;
	uint8_t nybble;
	uint32_t new_sample;

	decoded += (channel * 2);
	for ( i=0; i<4; ++i ) {
		unsigned char in = 0;
		try {
			in = stream->readU8();
		} catch (...) {
			return -1;
		}

		nybble = (uint8_t)(in&0x0F);
		new_sample = (uint32_t)nibble(state, nybble);
		decoded[0] = (uint8_t)(new_sample&0xFF);
		new_sample >>= 8;
		decoded[1] = (uint8_t)(new_sample&0xFF);
		decoded += 2 * numchannels;

		nybble = in>>4;
		new_sample = (uint32_t)nibble(state, nybble);
		decoded[0] = (uint8_t)(new_sample&0xFF);
		new_sample >>= 8;
		decoded[1] = (uint8_t)(new_sample&0xFF);
		decoded += 2 * numchannels;
	}
	return 0;
}

int tIMAADPCMDecoder::decode(uint8_t *buf, uint32_t bufsize, uint32_t *used) {
	uint32_t bufused = 0;
	uint32_t buffersize = buffer_size;
	while (bufsize > 0) {
		if (buffer_ofs == 0) {
			if ( stream->getPosition() >= stream->getLimit() ) {
				if (used) *used = bufused;
				return 0;
			}

			unsigned int c, channels = wavefmt.channels;
			uint8_t* decoded = buffer;
			for ( c = 0; c < channels; ++c ) {
				state[c].sample = 0;
				try {
					state[c].sample = stream->readS16();
					if ( state[c].sample & 0x8000 ) state[c].sample -= 0x10000;
					state[c].index = stream->readS8();
					uint8_t in = stream->readU8();
					if (in != 0) {
						// Поврежденные данные
					}
				} catch (...) {
					return -1;
				}

				decoded[0] = (uint8_t)(state[c].sample&0xFF);
				decoded[1] = (uint8_t)(state[c].sample>>8);
				decoded += 2;
			}

			int32_t samplesleft = (wSamplesPerBlock-1) * channels;
			while ( samplesleft > 0 ) {
				for ( c = 0; c < channels; ++c ) {
					if ( fill_block(decoded, c, channels, &state[c]) < 0 ) {
						// исправление незаконченного конца
						buffersize -= (samplesleft * channels * 2);
						samplesleft = 0;
						break;
					}
					samplesleft -= 8;
				}
				decoded += (channels * 8 * 2);
			}

		}
		register uint32_t size = bufsize;
		if ( ( buffer_ofs + size ) > buffersize ) {
			size = buffersize - buffer_ofs;
		}
		memcpy(buf,buffer,size);
		buffer_ofs += size; buf += size; bufsize -= size; bufused += size;
		if ( buffer_ofs >= buffersize ) {
			buffer_ofs -= buffersize;
		}
	}

	if (used) *used = bufused;
	return 0;
}

static int wav_readChunk(struct tWAVChunk *chunk, Stream* stream) {
	try {
		chunk->magic = stream->readU32();
		chunk->length = stream->readU32();
	} catch (...) {
		return -1;
	}
	if ( chunk->magic != WAVDEC_DATA ) {
		try {
			chunk->data = stream->readBytes(chunk->length);
		} catch (...) {
			return -1;
		}
	}
	return chunk->length;
}

int tWAVDecoder::init(Stream* stream, struct tAudioSpec* spec) {
	this->stream = stream;
	stream_start = stream->getPosition();

	// Заголовок WAV
	uint32_t RIFFchunk;
	uint32_t wavelen = 0;
	uint32_t WAVEmagic;

	// Проверяем заголовок
	try {
		RIFFchunk = stream->readU32();
		wavelen = stream->readU32();
	} catch (...) {
		delete this->stream; this->stream = NULL;
		return -1;
	}
	if ( wavelen == WAVDEC_WAVE ) {
		WAVEmagic = wavelen;
		wavelen   = RIFFchunk;
		RIFFchunk = WAVDEC_RIFF;
	} else {
		try {
			WAVEmagic = stream->readU32();
		} catch (...) {
			delete this->stream; this->stream = NULL;
			return -1;
		}
	}
	if ( (RIFFchunk != WAVDEC_RIFF) || (WAVEmagic != WAVDEC_WAVE) ) {
		delete this->stream; this->stream = NULL;
		return -1;
	}

	int is_err = 0;

	// Считываем блоки данных до тех пор пока не будет найден блок с форматом
	struct tWAVChunk chunk;
	int lenread;
	chunk.data = 0;
	do {
		if ( chunk.data != 0 ) {
			memFree(chunk.data);
		}
		lenread = wav_readChunk(&chunk, stream);
		if ( lenread < 0 ) {
			delete this->stream; this->stream = NULL;
			return -1;
		}
	} while ( (chunk.magic == WAVDEC_FACT) || (chunk.magic == WAVDEC_LIST) );

	// Декодируем формат аудио данных
	if ( chunk.magic != WAVDEC_FMT ) {
		// SetError("Complex WAVE files not supported");
		memFree(chunk.data);
		delete this->stream; this->stream = NULL;
		return -1;
	}

	int MS_ADPCM_encoded, IMA_ADPCM_encoded;
	struct tWAVFormat* format = (struct tWAVFormat*)chunk.data;
	MS_ADPCM_encoded = IMA_ADPCM_encoded = 0;

	{
		switch (format->encoding) {
			case WAVDEC_PCM_CODE:
				// full 8 16 bit support
			case WAVDEC_PCM_EXT_CODE:
				// fixme: 4 ch LCRS sample
				// fixme: 24 bit unsupported
				// 6 ch (5.1)

				encoding = format->encoding;
				break;
			case WAVDEC_MS_ADPCM_CODE:
				if (msadpcm.init(format, chunk.length, stream) < 0) {
					is_err = 1;
					goto tWAVDecoder_init_done;
				}
				encoding = format->encoding;
				MS_ADPCM_encoded = 1;
				break;
			case WAVDEC_IMA_ADPCM_CODE:
				if (imaadpcm.init(format, chunk.length, stream) < 0) {
					is_err = 1;
					goto tWAVDecoder_init_done;
				}
				encoding = format->encoding;
				IMA_ADPCM_encoded = 1;
				break;
			case WAVDEC_MP3_CODE:
				// SetError("MPEG Layer 3 data not supported",
				is_err = 1;
				goto tWAVDecoder_init_done;
			default:
				//SDL_SetError("Unknown WAVE data format: 0x%.4x",
				//SDL_SwapLE16(format->encoding));
				is_err = 1;
				goto tWAVDecoder_init_done;
		}

		memset(spec, 0, (sizeof(struct tAudioSpec)));
		spec->freq = format->frequency;
		bitpersecond = format->bitspersample * format->frequency * format->channels;
		switch (format->bitspersample) {
			case 4:
				if (MS_ADPCM_encoded || IMA_ADPCM_encoded) {
					spec->format = AUDIO_S16;
				} else {
					is_err = 1;
				}
				break;
			case 8:
				spec->format = AUDIO_U8;
				break;
			case 16:
				spec->format = AUDIO_S16;
				break;
			default:
				is_err = 1;
				break;
		}

		if (is_err) {
			// SetError("Unknown %d-bit PCM data format",
			goto tWAVDecoder_init_done;
		}
		spec->channels = (uint8_t) (format->channels);
		spec->samples = 4096;        // размер буффера по умолчанию

		// ищем начало аудио данных
		chunk.data = 0;
		do {
			if (chunk.data != 0) {
				memFree(chunk.data);
			}
			lenread = wav_readChunk(&chunk, stream);
			if (lenread < 0) return -1;
		} while (chunk.magic != WAVDEC_DATA);

		uint32_t samplesize = (uint32_t) (((spec->format & 0xFF) / 8) * spec->channels);
		datalen = chunk.length;
		datalen &= ~(samplesize - 1);

		this->stream->setLimit(this->stream->getPosition() + datalen);

		duration = datalen / format->byterate; // * 8 / bitpersecond;
	}

tWAVDecoder_init_done:
	memFree(format);

	if ( is_err ) {
		encoding = 0;
		delete this->stream; this->stream = NULL;
	}

	return is_err;
}

void tWAVDecoder::free() {
	switch (encoding) {
		case WAVDEC_MS_ADPCM_CODE:
			msadpcm.free();
			break;
		case WAVDEC_IMA_ADPCM_CODE:
			imaadpcm.free();
			break;
		default:;
	}
}

int tWAVDecoder::reset(struct tAudioSpec* spec) {
	stream->setPosition(stream_start);
	return init(stream, spec);
}

int tWAVDecoder::decode(uint8_t *buf, uint32_t bufsize, uint32_t *used) {
	int res = 0;
//	if (bufsize > datalen) bufsize = datalen;
	uint32_t bufused = 0;
	switch (encoding) {
		case WAVDEC_MS_ADPCM_CODE:
			res = msadpcm.decode(buf,bufsize,&bufused);
			break;
		case WAVDEC_IMA_ADPCM_CODE:
			res = imaadpcm.decode(buf,bufsize,&bufused);
			break;
		default: {
			res = stream->readBytes(buf,bufsize);
			if (res < static_cast<int32_t>(bufsize)) {
				if (stream->getPosition() >= stream->getLimit()) {
					bufused = (uint32_t)res;
					res = 0;
				} else {
					res = -1;
				}
			} else {
				res = 0;
				bufused = bufsize;
			}
			break;
		}
	}
//	datalen -= bufused;
	if (used) *used = bufused;
	return res;
}

#ifdef __cplusplus
}
#endif
