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

#include "uMP3Decoder.h"
#include "uMP3Tables.h"
#include "uMP3Synth.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tMPADecoder* mpadec_init() {
	struct tMPADecoder* mpa = memAlloc(struct tMPADecoder, mpa, sizeof(struct tMPADecoder));
	uint8_t temp[2];
	if (!mpa) return 0;
	memset(mpa, 0, sizeof(struct tMPADecoder));
	*((int16_t *)temp) = 1;
	mpa->config.quality = MPADEC_CONFIG_FULL_QUALITY;
	mpa->config.mode = MPADEC_CONFIG_AUTO;
	mpa->config.format = MPADEC_CONFIG_16BIT;
	mpa->config.endian = (uint8_t)((uint8_t)temp[0] ? MPADEC_CONFIG_LITTLE_ENDIAN : MPADEC_CONFIG_BIG_ENDIAN);
	mpa->config.replaygain = MPADEC_CONFIG_REPLAYGAIN_NONE;
	mpa->config.skip = 1;
	mpa->config.crc = 1;
	mpa->config.dblsync = 1;
	mpa->config.gain = 0.0;
	mpa->replay_gain = 1.0;
	mpadec_init_tables(mpa, 0x8000, MPADEC_SBLIMIT);
	mpa->synth_bufoffs = 1;
	mpa->state = MPADEC_STATE_START;
	return mpa;
}

void mpadec_uninit(struct tMPADecoder* mpadec) {
	memFree(mpadec);
}

struct tMP3Decoder* mp3dec_init() {
	struct tMP3Decoder* mp3 = memAlloc(struct tMP3Decoder, mp3, sizeof(struct tMP3Decoder));
	if (!mp3) return 0;
	memset(mp3, 0, sizeof(struct tMP3Decoder));
	if (!(mp3->mpadec = mpadec_init())) {
		memFree(mp3);
		return 0;
	}
	return mp3;
}

void mp3dec_uninit(struct tMP3Decoder* mp3dec) {
	mpadec_uninit(mp3dec->mpadec);
	memFree(mp3dec);
}

void tMPADecoder::reset() {
	struct tMPADecoder* mpa = this;
	memset(mpa->hybrid_in, 0, sizeof(mpa->hybrid_in));
	memset(mpa->hybrid_out, 0, sizeof(mpa->hybrid_out));
	memset(mpa->hybrid_buffers, 0, sizeof(mpa->hybrid_buffers));
	memset(mpa->synth_buffers, 0, sizeof(mpa->synth_buffers));
	mpa->hybrid_block[0] = mpa->hybrid_block[1] = 0;
	mpa->synth_bufoffs = 1;
	memset(&mpa->tag_info, 0, sizeof(mpa->tag_info));
	if (mpa->config.replaygain != MPADEC_CONFIG_REPLAYGAIN_CUSTOM) mpa->config.gain = 0.0;
	mpa->prev_frame_size = 0;
	mpa->free_format = 0;
	mpa->error = 0;
	mpa->reservoir_size = 0;
	mpa->replay_gain = 1.0;
	mpa->skip_samples = 0;
	mpa->padding_samples = 0;
	mpa->decoded_frames = 0;
	mpa->decoded_samples = 0;
	mpa->state = MPADEC_STATE_START;
}

void tMP3Decoder::reset() {
	struct tMP3Decoder* mp3 = this;
	mp3->init_done = 0;
	mp3->stream = 0;
	mp3->mpadec->reset();
	mp3->stream_size = 0;
	mp3->in_buffer_offset = mp3->in_buffer_used = 0;
	mp3->out_buffer_offset = mp3->out_buffer_used = 0;
	memset(&mp3->mpainfo, 0, sizeof(mp3->mpainfo));
	memset(&mp3->taginfo, 0, sizeof(mp3->taginfo));
}

#define MPADEC_RETCODE_ERROR             -1
#define MPADEC_RETCODE_OK                 0
#define MPADEC_RETCODE_NEED_MORE_DATA     1
#define MPADEC_RETCODE_BUFFER_TOO_SMALL   2
#define MPADEC_RETCODE_NO_SYNC            3

static const int16_t mpadec_bitrate_table[2][3][16] = {
	{
		{ 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 560 },
		{ 0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 448 },
		{ 0, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384 }
	},
	{
		{ 0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 320 },
		{ 0,  8, 16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160, 192 },
		{ 0,  8, 16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160, 192 }
	}
};

static const int32_t mpadec_frequency_table[9] = { 44100, 48000, 32000, 22050, 24000, 16000, 11025, 12000, 8000 };

uint32_t tMPADecoder::detect_frame_size() {
	struct tMPADecoder* mpa = this;
	uint8_t *buf = mpa->next_byte;
	uint32_t i = mpa->bytes_left, hdr = 0;

	if (i < (mpa->frame.frame_size + 4)) return 0;
	buf += mpa->frame.frame_size;
	i -= mpa->frame.frame_size;
	while (i >= 4) {
		uint32_t tmp = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
		if (((tmp & 0xFFE00000) == 0xFFE00000) && (tmp & (3 << 17)) && ((tmp & (3 << 10)) != (3 << 10))) {
			if ((mpa->frame.layer == (uint8_t)(4 - ((tmp >> 17) & 3))) &&
				(mpa->frame.frequency_index == (((tmp >> 10) & 3) + 3*(mpa->frame.LSF + mpa->frame.MPEG25))) &&
				(mpa->frame.channels == ((((tmp >> 6) & 3) == MPG_MD_MONO) ? 1 : 2))) {
				if (mpa->config.dblsync) {
					uint32_t fs = mpa->bytes_left - i - mpa->frame.padding + ((tmp >> 9) & 1);
					if (i >= (fs + 4)) {
						buf += fs;
						tmp = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
						buf -= fs;
						if (((tmp & 0xFFE00000) == 0xFFE00000) && (tmp & (3 << 17)) && ((tmp & (3 << 10)) != (3 << 10))) {
							if ((mpa->frame.layer == (uint8_t)(4 - ((tmp >> 17) & 3))) &&
								(mpa->frame.frequency_index == (((tmp >> 10) & 3) + 3*(mpa->frame.LSF + mpa->frame.MPEG25))) &&
								(mpa->frame.channels == ((((tmp >> 6) & 3) == MPG_MD_MONO) ? 1 : 2))) {
								hdr = tmp;
								break;
							}
						}
					} else return 0;
				} else {
					hdr = tmp;
					break;
				}
			}
		}
		buf++; i--;
	}
	return (hdr ? (mpa->bytes_left - i) : 0);
}

int tMPADecoder::decode_header(uint32_t header) {
	struct tMPADecoder* mpa = this;
	unsigned int layer, bridx, fridx;

	layer = 4 - ((header >> 17) & 3);
	bridx = ((header >> 12) & 0x0F);
	fridx = ((header >> 10) & 3);
	if ((fridx == 3) || (layer == 4) || ((layer != 3) && !bridx)) return 0;
	if (header & (1 << 20)) {
		mpa->frame.LSF = (uint8_t)((header & (1 << 19)) ? 0 : 1);
		mpa->frame.MPEG25 = 0;
	} else mpa->frame.LSF = mpa->frame.MPEG25 = 1;
	mpa->frame.layer = (uint8_t)layer;
	mpa->frame.bitrate_index = bridx;
	mpa->frame.bitrate = (uint32_t)mpadec_bitrate_table[mpa->frame.LSF][layer - 1][bridx];
	mpa->frame.frequency_index = (fridx += 3*(mpa->frame.LSF + mpa->frame.MPEG25));
	mpa->frame.frequency = (uint32_t)mpadec_frequency_table[fridx];
	mpa->frame.decoded_frequency = mpa->frame.frequency >> mpa->config.quality;
	mpa->frame.CRC = (uint8_t)(((header >> 16) & 1) ^ 1);
	mpa->frame.padding = ((header >> 9) & 1);
	mpa->frame.extension = (uint8_t)((header >> 8) & 1);
	mpa->frame.mode = (uint8_t)((header >> 6) & 3);
	mpa->frame.mode_ext = (uint8_t)((header >> 4) & 3);
	mpa->frame.copyright = (uint8_t)((header >> 3) & 1);
	mpa->frame.original = (uint8_t)((header >> 2) & 1);
	mpa->frame.emphasis = (uint8_t)(header & 3);
	mpa->frame.channels = (uint8_t)((mpa->frame.mode == MPG_MD_MONO) ? 1 : 2);
	switch (mpa->config.mode) {
		case MPADEC_CONFIG_CHANNEL1:
		case MPADEC_CONFIG_CHANNEL2:
		case MPADEC_CONFIG_MONO:     mpa->frame.decoded_channels = 1; break;
		case MPADEC_CONFIG_STEREO:   mpa->frame.decoded_channels = 2; break;
		default:                     mpa->frame.decoded_channels = mpa->frame.channels;
	}
	mpa->free_format = 0;
	switch (mpa->frame.layer) {
		case 1: mpa->frame.frame_samples = 384;
			mpa->frame.frame_size = (12000*mpa->frame.bitrate/mpa->frame.frequency + mpa->frame.padding) << 2;
			mpa->ssize = 0;
			break;
		case 2: mpa->frame.frame_samples = 1152;
			mpa->frame.frame_size = 144000*mpa->frame.bitrate/mpa->frame.frequency + mpa->frame.padding;
			mpa->ssize = 0;
			break;
		case 3: mpa->frame.frame_samples = (uint32_t)(1152 >> mpa->frame.LSF);
			if (mpa->frame.LSF) mpa->ssize = (mpa->frame.channels > 1) ? 17 : 9;
			else mpa->ssize = (mpa->frame.channels > 1) ? 32 : 17;
			if (mpa->frame.bitrate_index) {
				mpa->frame.frame_size = 144000*mpa->frame.bitrate/(mpa->frame.frequency << mpa->frame.LSF) + mpa->frame.padding;
				mpa->prev_frame_size = 0;
			} else {
				uint32_t div = 125*mpa->frame.frame_samples;
				if (!mpa->prev_frame_size) {
					mpa->frame.frame_size = (mpa->frame.CRC ? 6 : 4) + mpa->ssize + mpa->frame.padding;
					mpa->prev_frame_size = detect_frame_size();
					if (mpa->prev_frame_size) mpa->prev_frame_size -= mpa->frame.padding;
				}
				mpa->frame.frame_size = mpa->prev_frame_size + mpa->frame.padding;
				mpa->frame.bitrate = (mpa->frame.frame_size*mpa->frame.frequency + (div >> 1))/div;
				mpa->free_format = 1;
			}
			break;
		default:;
	}
	mpa->frame.decoded_samples = mpa->frame.frame_samples >> mpa->config.quality;
	mpa->frame.downsample = 0;
	mpa->frame.downsample_sblimit = MPADEC_SBLIMIT;
	mpa->frame.decoded_size = mpa->frame.decoded_samples*mpa->frame.decoded_channels;
	switch (mpa->config.format) {
		case MPADEC_CONFIG_24BIT: mpa->frame.decoded_size *= 3; break;
		case MPADEC_CONFIG_32BIT:
		case MPADEC_CONFIG_FLOAT: mpa->frame.decoded_size <<= 2; break;
		default:                  mpa->frame.decoded_size <<= 1; break;
	}
	mpa->hsize = mpa->frame.CRC ? 6 : 4;
	if (mpa->frame.frame_size < (mpa->hsize + mpa->ssize)) mpa->frame.frame_size = mpa->hsize + mpa->ssize;
	mpa->dsize = mpa->frame.frame_size - (mpa->hsize + mpa->ssize);
	return 1;
}

uint32_t tMPADecoder::sync_buffer() {
	struct tMPADecoder* mpa = this;
	uint8_t *buf = mpa->next_byte;
	uint32_t retval = 0, i = mpa->bytes_left;

	if (mpa->state == MPADEC_STATE_START) {
		while (i >= 4) {
			uint32_t tmp = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
			if (((tmp & 0xFFE00000) == 0xFFE00000) && (tmp & (3 << 17)) && ((tmp & (3 << 10)) != (3 << 10))) {
				if (mpa->config.dblsync) {
					if (decode_header(tmp)) {
						if ((i < (mpa->frame.frame_size + 4)) || (mpa->free_format && !mpa->prev_frame_size)) break;
						else {
							uint32_t tmp2 = (buf[mpa->frame.frame_size] << 24) | (buf[mpa->frame.frame_size + 1] << 16) | (buf[mpa->frame.frame_size + 2] << 8) | buf[mpa->frame.frame_size + 3];
							if (((tmp2 & 0xFFE00000) == 0xFFE00000) && (tmp2 & (3 << 17)) && ((tmp2 & (3 << 10)) != (3 << 10))) {
								if ((mpa->frame.layer == (uint8_t)(4 - ((tmp2 >> 17) & 3))) &&
									(mpa->frame.frequency_index == (((tmp2 >> 10) & 3) + 3*(mpa->frame.LSF + mpa->frame.MPEG25))) &&
									(mpa->frame.channels == ((((tmp2 >> 6) & 3) == MPG_MD_MONO) ? 1 : 2))) {
									retval = tmp;
									break;
								}
							}
						}
					}
				} else {
					retval = tmp;
					break;
				}
			}
			buf++; i--;
		}
	} else {
		while (i >= 4) {
			uint32_t tmp = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
			if (((tmp & 0xFFE00000) == 0xFFE00000) && (tmp & (3 << 17)) && ((tmp & (3 << 10)) != (3 << 10))) {
				if ((mpa->frame.layer == (uint8_t)(4 - ((tmp >> 17) & 3))) &&
					(mpa->frame.frequency_index == (((tmp >> 10) & 3) + 3*(mpa->frame.LSF + mpa->frame.MPEG25))) &&
					(mpa->frame.channels == ((((tmp >> 6) & 3) == MPG_MD_MONO) ? 1 : 2))) {
					retval = tmp;
					break;
				}
			}
			buf++; i--;
		}
	}
	if (i < mpa->bytes_left) {
		i = mpa->bytes_left - i;
		mpa->next_byte = buf;;
		mpa->bytes_left -= i;
		if (i >= 512) {
			mpa->reservoir_size = 0;
			i = 512;
		}
		memcpy(mpa->reservoir + mpa->reservoir_size, mpa->next_byte - i, i);
		mpa->reservoir_size += i;
	}
	return retval;
}

int tMPADecoder::first_frame() {
	struct tMPADecoder* mpa = this;
	int i, skip = 0;
	uint32_t framesize; double scale;

	if (mpa->frame.channels > 1) i = ((mpa->config.mode == MPADEC_CONFIG_STEREO) || (mpa->config.mode == MPADEC_CONFIG_AUTO)) ? 3 : 2;
	else i = (mpa->config.mode == MPADEC_CONFIG_STEREO) ? 1 : 0;
	mpa->synth_func = mpadec_synth_table[mpa->config.quality][mpa->config.endian][mpa->config.format][i];
	mpa->sample_size = mpa->frame.decoded_channels;
	switch (mpa->config.format) {
		case MPADEC_CONFIG_24BIT: mpa->sample_size *= 3; scale = 0x800000; break;
		case MPADEC_CONFIG_32BIT: mpa->sample_size <<= 2; scale = 0x80000000; break;
		case MPADEC_CONFIG_FLOAT: mpa->sample_size <<= 2; scale = 1; break;
		default:                  mpa->sample_size <<= 1; scale = 0x8000; break;
	}
	mpa->synth_size = (mpa->sample_size << 5) >> mpa->config.quality;
	if (mpa->config.replaygain != MPADEC_CONFIG_REPLAYGAIN_CUSTOM) {
		mpa->config.gain = 0.0;
		mpa->replay_gain = 1.0;
	}
	mpa->skip_samples = 0;
	mpa->padding_samples = 0;
	mpa->decoded_frames = 0;
	mpa->decoded_samples = 0;
	memset(&mpa->tag_info, 0, sizeof(mpa->tag_info));
	framesize = (mpa->frame.frame_size < mpa->bytes_left) ? mpa->frame.frame_size : mpa->bytes_left;
	if ((mpa->frame.layer == 3) && (framesize >= (mpa->ssize + 124))) {
		uint8_t *buf = mpa->next_byte + 4 + mpa->ssize;
		if (((buf[0] == 'X') && (buf[1] == 'i') && (buf[2] == 'n') && (buf[3] == 'g')) ||
			((buf[0] == 'I') && (buf[1] == 'n') && (buf[2] == 'f') && (buf[3] == 'o'))) {
			skip = 1;
			mpa->next_byte += framesize;
			mpa->bytes_left -= framesize;
			buf += 4;
			mpa->tag_info.flags = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
			buf += 4;
			if (mpa->tag_info.flags & 1) {
				mpa->tag_info.frames = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
				buf += 4;
			};
			if (mpa->tag_info.flags & 2) {
				mpa->tag_info.bytes = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
				buf += 4;
			};
			if (mpa->tag_info.flags & 4) {
				memcpy(mpa->tag_info.toc, buf, 100);
				buf += 100;
			};
			if (mpa->tag_info.flags & 8) buf += 4;
			mpa->tag_info.flags &= 7;
			if (framesize >= mpa->ssize + 160) {
				buf += 15;
				mpa->tag_info.replay_gain[0] = ((buf[0] << 8) | buf[1]) & 0x1FF;
				if (buf[0] & 2) mpa->tag_info.replay_gain[0] = -mpa->tag_info.replay_gain[0];
				buf += 2;
				mpa->tag_info.replay_gain[1] = ((buf[0] << 8) | buf[1]) & 0x1FF;
				if (buf[0] & 2) mpa->tag_info.replay_gain[1] = -mpa->tag_info.replay_gain[1];
				buf += 4;
				mpa->tag_info.enc_delay = (buf[0] << 4) | ((buf[1] >> 4) & 0x0F);
				mpa->tag_info.enc_padding = ((buf[1] & 0x0F) << 8) | buf[2];
				if (((mpa->tag_info.enc_delay < 0) || (mpa->tag_info.enc_delay > 3000)) ||
					((mpa->tag_info.enc_padding < 0) || (mpa->tag_info.enc_padding > 3000))) {
					mpa->tag_info.replay_gain[0] = mpa->tag_info.replay_gain[1] = 0;
					mpa->tag_info.enc_delay = 0;
					mpa->tag_info.enc_padding = 0;
				} else {
					if (mpa->config.replaygain == MPADEC_CONFIG_REPLAYGAIN_RADIO) {
						mpa->config.gain = ((double)mpa->tag_info.replay_gain[0])/10.0;
					} else if (mpa->config.replaygain == MPADEC_CONFIG_REPLAYGAIN_AUDIOPHILE) {
						mpa->config.gain = ((double)mpa->tag_info.replay_gain[1])/10.0;
					}
				}
				mpa->skip_samples = (uint32_t)mpa->tag_info.enc_delay;
				mpa->padding_samples = (uint32_t)mpa->tag_info.enc_padding;
			}
		}
	}
	mpa->replay_gain = pow(10.0, mpa->config.gain/20.0);
	if (mpa->config.replaygain != MPADEC_CONFIG_REPLAYGAIN_NONE) scale *= mpa->replay_gain;
	mpadec_init_tables(mpa, scale, MPADEC_SBLIMIT >> mpa->config.quality);
	if (mpa->frame.layer == 3) {
		mpa->skip_samples += 529;
		if ((mpa->tag_info.flags & 1) && (mpa->padding_samples > 529)) mpa->padding_samples -= 529;
		else mpa->padding_samples = 0;
	} else {
		mpa->skip_samples += 241;
		mpa->padding_samples = 0;
	}
	mpa->padding_start = mpa->tag_info.frames*mpa->frame.frame_samples;
	if (mpa->padding_start > mpa->padding_samples) mpa->padding_start -= mpa->padding_samples;
	else mpa->padding_start = 0;
	mpa->state = MPADEC_STATE_DECODE;
	return skip;
}

static const uint16_t mpadec_crc_table[256] = {
	0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
	0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
	0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
	0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
	0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
	0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
	0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
	0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
	0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
	0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
	0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
	0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
	0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
	0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
	0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
	0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
	0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
	0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
	0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
	0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
	0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
	0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
	0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
	0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
	0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
	0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
	0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
	0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
	0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
	0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
	0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
	0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

uint16_t mpadec_update_crc(uint16_t init, uint8_t *buf, int length) {
	uint32_t crc = (uint32_t)init, tmp;
	int l = length;
	uint8_t *b = buf;

	for (; l >= 8; l -= 8) crc = (crc << 8) ^ mpadec_crc_table[((crc >> 8) ^ (*b++)) & 0xFF];
	tmp = (uint32_t)(*b) << 8;
	while (l--) {
		tmp <<= 1;
		crc <<= 1;
		if ((crc ^ tmp) & 0x10000) crc ^= 0x8005;
	}
	return (uint16_t)crc;
}

uint32_t mpadec_getbits(struct tMPADecoder* mpadec, unsigned n) {
	while ((mpadec->bits_left <= 24) && mpadec->bytes_left) {
		mpadec->bit_buffer = (mpadec->bit_buffer << 8) | *mpadec->next_byte++;
		mpadec->bits_left += (uint8_t)8;
		mpadec->bytes_left--;
	}
	while (mpadec->bits_left <= 24) {
		mpadec->bit_buffer = (mpadec->bit_buffer << 8);
		mpadec->bits_left += (uint8_t)8;
	}
	mpadec->bits_left -= (uint8_t)n;
	return (mpadec->bit_buffer >> mpadec->bits_left) & mpadec_bitmask[n];
}

extern void mpadec_decode_layer1(struct tMPADecoder* mpadec, uint8_t *buffer);
extern void mpadec_decode_layer2(struct tMPADecoder* mpadec, uint8_t *buffer);
extern void mpadec_decode_layer3(struct tMPADecoder* mpadec, uint8_t *buffer);

int tMPADecoder::decode(uint8_t *srcbuf, uint32_t srcsize, uint8_t *dstbuf, uint32_t dstsize, uint32_t *srcused, uint32_t *dstused) {
	struct tMPADecoder* mpa = this;
	int retcode = MPADEC_RETCODE_OK;
	uint32_t decoded_size = 0;

	if (srcused) *srcused = 0;
	if (dstused) *dstused = 0;
	if (!dstbuf) dstsize = 0;
	if (mpa->state < MPADEC_STATE_START) return -1;
	if (!srcbuf || ((mpa->state > MPADEC_STATE_START) && !dstbuf )) return -1;
	mpa->next_byte = srcbuf;
	mpa->bytes_left = srcsize;
	while (mpa->bytes_left >= 4) {
		mpa->error = 0;
		mpa->bits_left = 0;
		mpa->header = sync_buffer();
		if (!mpa->header) {
			if (!decoded_size) retcode = MPADEC_RETCODE_NO_SYNC;
			break;
		}
		decode_header(mpa->header);
		if ((mpa->bytes_left < mpa->frame.frame_size) || (mpa->free_format && !mpa->prev_frame_size)) {
			retcode = MPADEC_RETCODE_NEED_MORE_DATA;
			break;
		}
		if (mpa->state == MPADEC_STATE_START) {
			if (first_frame()) continue;
		} else if ((mpa->frame.layer == 3) && (mpa->frame.frame_size >= (mpa->ssize + 12))) {
			uint8_t *buf = mpa->next_byte + 4 + mpa->ssize;
			if (((buf[0] == 'X') && (buf[1] == 'i') && (buf[2] == 'n') && (buf[3] == 'g')) ||
				((buf[0] == 'I') && (buf[1] == 'n') && (buf[2] == 'f') && (buf[3] == 'o'))) {
				mpa->next_byte += mpa->frame.frame_size;
				mpa->bytes_left -= mpa->frame.frame_size;
				continue;
			}
		}
		if (dstsize < mpa->frame.decoded_size) {
			retcode = MPADEC_RETCODE_BUFFER_TOO_SMALL;
			break;
		}
		if (mpa->config.crc && mpa->frame.CRC) {
			mpa->frame.crc = (uint16_t)((mpa->next_byte[4] << 8) | mpa->next_byte[5]);
			mpa->crc = mpadec_update_crc(0xFFFF, mpa->next_byte + 2, 16);
		}
		mpa->next_byte += mpa->hsize;
		mpa->bytes_left -= mpa->hsize;
		switch (mpa->frame.layer) {
			case 1: mpadec_decode_layer1(mpa, dstbuf); break;
			case 2: mpadec_decode_layer2(mpa, dstbuf); break;
			case 3: mpadec_decode_layer3(mpa, dstbuf); break;
			default:;
		}
		if (mpa->config.crc && mpa->error) memset(dstbuf, 0, mpa->frame.decoded_size);
		dstbuf += mpa->frame.decoded_size;
		dstsize -= mpa->frame.decoded_size;
		decoded_size += mpa->frame.decoded_size;
		mpa->decoded_samples += mpa->frame.frame_samples;
		mpa->decoded_frames++;
		if (mpa->config.skip) {
			if (mpa->skip_samples) {
				if (mpa->skip_samples >= mpa->frame.frame_samples) {
					dstbuf -= mpa->frame.decoded_size;
					dstsize += mpa->frame.decoded_size;
					decoded_size -= mpa->frame.decoded_size;
					mpa->skip_samples -= mpa->frame.frame_samples;
				} else {
					uint32_t tmp = mpa->frame.frame_samples - mpa->skip_samples;
					tmp = mpa->sample_size*(tmp >> mpa->config.quality);
					memmove(dstbuf - mpa->frame.decoded_size, dstbuf - tmp, tmp);
					dstbuf -= mpa->frame.decoded_size - tmp;
					dstsize += mpa->frame.decoded_size - tmp;
					decoded_size -= mpa->frame.decoded_size - tmp;
					mpa->skip_samples = 0;
				}
			} else if ((mpa->padding_samples) && (mpa->decoded_samples > mpa->padding_start)) {
				uint32_t tmp = mpa->decoded_samples - mpa->padding_start;
				if (tmp > mpa->padding_samples) tmp = mpa->padding_samples;
				mpa->padding_start += tmp;
				mpa->padding_samples -= tmp;
				tmp = mpa->sample_size*(tmp >> mpa->config.quality);
				dstbuf -= tmp;
				dstsize += tmp;
				decoded_size -= tmp;
			}
		}
	}
	if (srcused) *srcused = srcsize - mpa->bytes_left;
	if (dstused) *dstused = decoded_size;
	if ((retcode == MPADEC_RETCODE_OK) && mpa->bytes_left) retcode = MPADEC_RETCODE_NEED_MORE_DATA;
	if (!dstbuf && (retcode == MPADEC_RETCODE_BUFFER_TOO_SMALL)) retcode = MPADEC_RETCODE_OK;
	return retcode;
}

int tMPADecoder::get_info(void *info, int info_type) {
	struct tMPADecoder* mpa = this;
	if (!info) return -1;
	if (info_type == MPADEC_INFO_CONFIG) {
		struct tMPADecoderConfig *cfg = (struct tMPADecoderConfig*)info;
		cfg->quality = mpa->config.quality;
		cfg->mode = mpa->config.mode;
		cfg->format = mpa->config.format;
		cfg->endian = mpa->config.endian;
		cfg->replaygain = mpa->config.replaygain;
		cfg->skip = mpa->config.skip;
		cfg->crc = mpa->config.crc;
		cfg->dblsync = mpa->config.dblsync;
		cfg->gain = mpa->config.gain;
	} else if (info_type == MPADEC_INFO_TAG) {
		tMPATagInfo *tag = (tMPATagInfo*)info;
		if (mpa->state < MPADEC_STATE_DECODE) {
			memset(tag, 0, sizeof(tMPATagInfo));
			return -1;
		} else memcpy(tag, &mpa->tag_info, sizeof(mpa->tag_info));
	} else if (info_type == MPADEC_INFO_STREAM) {
		struct tMPADecoderInfo *inf = (struct tMPADecoderInfo*)info;
		if (mpa->state < MPADEC_STATE_DECODE) {
			memset(inf, 0, sizeof(struct tMPADecoderInfo));
			return -1;
		} else {
			inf->layer = mpa->frame.layer;
			inf->channels = mpa->frame.channels;
			inf->frequency = mpa->frame.frequency;
			inf->bitrate = mpa->frame.bitrate;
			inf->mode = mpa->frame.mode;
			inf->copyright = mpa->frame.copyright;
			inf->original = mpa->frame.original;
			inf->emphasis = mpa->frame.emphasis;
			inf->decoded_channels = mpa->frame.decoded_channels;
			inf->decoded_frequency = mpa->frame.decoded_frequency;
			inf->decoded_sample_size = mpa->sample_size;
			inf->frame_size = mpa->frame.frame_size;
			inf->frame_samples = mpa->frame.frame_samples;
			inf->decoded_frame_samples = mpa->frame.decoded_samples;
			if (mpa->tag_info.flags & 1) {
				inf->frames = mpa->tag_info.frames;
				inf->duration = (mpa->tag_info.frames*mpa->frame.frame_samples + (mpa->frame.frequency >> 1))/mpa->frame.frequency;
			} else {
				inf->frames = 0;
				inf->duration = 0;
			}
		}
	} else return -1;
	return 0;
}

int tMP3Decoder::init(Stream* stream, int nogap) {
	struct tMP3Decoder* mp3 = this;
	//int64_t tmp;
	int r;
	mp3->init_done = 0;
	mp3->stream = stream;
	mp3->stream_size = stream->getSize();
	mp3->stream_start = stream->getPosition();
	mp3->in_buffer_offset = mp3->in_buffer_used = 0;
	mp3->out_buffer_offset = mp3->out_buffer_used = 0;

	int32_t n = sizeof(mp3->in_buffer);
	if (static_cast<uint32_t>(n) > mp3->stream_size) n = (int32_t)mp3->stream_size;
	n = stream->readBytes(mp3->in_buffer, (uint32_t)n);
	mp3->in_buffer_used = (uint32_t)n;
	if (n < 4) {
		reset();
		return -1;
	}
	if (nogap) {
		mp3->mpadec->decode(mp3->in_buffer, mp3->in_buffer_used, mp3->out_buffer, sizeof(mp3->out_buffer), &mp3->in_buffer_offset, &mp3->out_buffer_used);
		mp3->in_buffer_used -= mp3->in_buffer_offset;
		if (!mp3->out_buffer_used) {
			mp3->mpadec->reset();
			mp3->in_buffer_used += mp3->in_buffer_offset;
			mp3->in_buffer_offset = 0;
		}
	} else
		mp3->mpadec->reset();
	if (!mp3->out_buffer_used) {
		r = mp3->mpadec->decode(mp3->in_buffer, mp3->in_buffer_used, 0, 0, &mp3->in_buffer_offset, 0);
		mp3->in_buffer_used -= mp3->in_buffer_offset;
		if (r != 0) {
			reset();
			return -1;
		}
	}
	if ((mp3->mpadec->get_info(&mp3->mpainfo, MPADEC_INFO_STREAM) != 0) ||
		(mp3->mpadec->get_info(&mp3->taginfo, MPADEC_INFO_TAG) != 0)) {
		reset();
		return -1;
	}
	if (mp3->taginfo.flags & 2) if (!mp3->stream_size || (mp3->stream_size > mp3->taginfo.bytes)) mp3->stream_size = mp3->taginfo.bytes;
	if (mp3->taginfo.flags & 1) {
		mp3->mpainfo.frames = mp3->taginfo.frames;
		if (mp3->mpainfo.frames && mp3->mpainfo.frame_samples) {
			mp3->mpainfo.bitrate = (int32_t)((((double)mp3->stream_size*(double)mp3->mpainfo.frequency + 0.5)/((double)125.0*mp3->mpainfo.frame_samples*mp3->mpainfo.frames)));
		}
	} else if (mp3->mpainfo.bitrate && mp3->mpainfo.frame_samples) {
		mp3->mpainfo.frames = (int32_t)((((double)mp3->stream_size*(double)mp3->mpainfo.frequency + 0.5)/((double)125.0*mp3->mpainfo.frame_samples*mp3->mpainfo.bitrate)));
	}
	mp3->mpainfo.duration = (mp3->mpainfo.frames*mp3->mpainfo.frame_samples + (mp3->mpainfo.frequency >> 1))/mp3->mpainfo.frequency;
	mp3->init_done = 1;
	mp3->stream_start += mp3->in_buffer_offset;
	return 0;
}

int tMPADecoder::configure(struct tMPADecoderConfig *cfg) {
	struct tMPADecoder* mpa = this;
	int32_t i, sblimit; double scale;

	if (!cfg) return -1;
	if ((cfg->quality > MPADEC_CONFIG_HALF_QUALITY) || (cfg->mode > MPADEC_CONFIG_CHANNEL2) ||
		(cfg->format > MPADEC_CONFIG_FLOAT) || (cfg->endian > MPADEC_CONFIG_BIG_ENDIAN) ||
		(cfg->replaygain > MPADEC_CONFIG_REPLAYGAIN_CUSTOM)) return -1;
	mpa->config.quality = cfg->quality;
	mpa->config.mode = cfg->mode;
	mpa->config.format = cfg->format;
	mpa->config.endian = cfg->endian;
	mpa->config.replaygain = cfg->replaygain;
	mpa->config.skip = (uint8_t)(cfg->skip ? 1 : 0);
	mpa->config.crc = (uint8_t)(cfg->crc ? 1 : 0);
	mpa->config.dblsync = (uint8_t)(cfg->dblsync ? 1 : 0);
	if (mpa->config.replaygain == MPADEC_CONFIG_REPLAYGAIN_CUSTOM) {
		mpa->config.gain = cfg->gain;
	} else {
		mpa->config.gain = 0.0;
		if (mpa->tag_info.flags) {
			if (mpa->config.replaygain == MPADEC_CONFIG_REPLAYGAIN_RADIO) {
				mpa->config.gain = ((double)mpa->tag_info.replay_gain[0])/10.0;
			} else if (mpa->config.replaygain == MPADEC_CONFIG_REPLAYGAIN_AUDIOPHILE) {
				mpa->config.gain = ((double)mpa->tag_info.replay_gain[1])/10.0;
			}
		}
	}
	mpa->replay_gain = pow(10.0, mpa->config.gain/20.0);
	switch (mpa->config.format) {
		case MPADEC_CONFIG_24BIT: scale = 0x800000; break;
		case MPADEC_CONFIG_32BIT: scale = 0x80000000; break;
		case MPADEC_CONFIG_FLOAT: scale = 1; break;
		default:                  scale = 0x8000; break;
	}
	sblimit = MPADEC_SBLIMIT >> mpa->config.quality;
	if (mpa->config.replaygain != MPADEC_CONFIG_REPLAYGAIN_NONE) scale *= mpa->replay_gain;
	mpadec_init_tables(mpa, scale, sblimit);
	if ((mpa->state > MPADEC_STATE_START) && mpa->header) {
		decode_header(mpa->header);
		if (mpa->frame.channels < 2) i = (mpa->config.mode == MPADEC_CONFIG_STEREO) ? 1 : 0;
		else i = ((mpa->config.mode == MPADEC_CONFIG_STEREO) || (mpa->config.mode == MPADEC_CONFIG_AUTO)) ? 3 : 2;
		mpa->synth_func = mpadec_synth_table[mpa->config.quality][mpa->config.endian][mpa->config.format][i];
		mpa->sample_size = mpa->frame.decoded_channels;
		switch (mpa->config.format) {
			case MPADEC_CONFIG_24BIT: mpa->sample_size *= 3; break;
			case MPADEC_CONFIG_32BIT:
			case MPADEC_CONFIG_FLOAT: mpa->sample_size <<= 2; break;
			default:                  mpa->sample_size <<= 1; break;
		}
		mpa->synth_size = (mpa->sample_size << 5) >> mpa->config.quality;
	} else mpa->state = MPADEC_STATE_START;
	return 0;
}

int tMP3Decoder::configure(struct tMPADecoderConfig *cfg) {
	struct tMP3Decoder* mp3 = this;
	mp3->out_buffer_offset = mp3->out_buffer_used = 0;
	if (mp3->mpadec->configure(cfg) != 0) return -1;
	return 0;
}

int tMP3Decoder::get_info(void *info, int info_type) {
	struct tMP3Decoder* mp3 = this;
	if (!info) return -1;
	if (mp3->init_done) {
		switch (info_type) {
			case MPADEC_INFO_STREAM: memcpy(info, &mp3->mpainfo, sizeof(mp3->mpainfo)); break;
			case MPADEC_INFO_TAG:    memcpy(info, &mp3->taginfo, sizeof(mp3->taginfo)); break;
			case MPADEC_INFO_CONFIG:
			default:
				if (mp3->mpadec->get_info(info, info_type) != 0) return -1;
		}
	} else return -1;
	return 0;
}

int tMP3Decoder::decode(uint8_t *buf, uint32_t bufsize, uint32_t *used) {
	struct tMP3Decoder* mp3 = this;
	uint32_t n, src_used, dst_used; int r;

	if (used) *used = 0;
	if (!(mp3->init_done)) return -1;
	if (!buf) return -1;
	while (bufsize) {
		if (mp3->out_buffer_used) {
			n = (bufsize < mp3->out_buffer_used) ? bufsize : mp3->out_buffer_used;
			memcpy(buf, mp3->out_buffer + mp3->out_buffer_offset, n);
			mp3->out_buffer_offset += n;
			mp3->out_buffer_used -= n;
			buf += n;
			bufsize -= n;
			if (used) *used += n;
		}
		if (!bufsize) break;
		if (mp3->in_buffer_used > 4) {
			r = mp3->mpadec->decode(mp3->in_buffer + mp3->in_buffer_offset, mp3->in_buffer_used, buf, bufsize, &src_used, &dst_used);
			mp3->in_buffer_offset += src_used;
			mp3->in_buffer_used -= src_used;
			buf += dst_used;
			bufsize -= dst_used;
			if (used) *used += dst_used;
			if (!bufsize) break;
			if (r == MPADEC_RETCODE_BUFFER_TOO_SMALL) {
				mp3->out_buffer_offset = mp3->out_buffer_used = 0;
				mp3->mpadec->decode(mp3->in_buffer + mp3->in_buffer_offset, mp3->in_buffer_used, mp3->out_buffer, sizeof(mp3->out_buffer), &src_used, &mp3->out_buffer_used);
				mp3->in_buffer_offset += src_used;
				mp3->in_buffer_used -= src_used;
				continue;
			}
		}
		if (mp3->in_buffer_used && mp3->in_buffer_offset) memmove(mp3->in_buffer, mp3->in_buffer + mp3->in_buffer_offset, mp3->in_buffer_used);
		mp3->in_buffer_offset = 0;
		n = sizeof(mp3->in_buffer) - mp3->in_buffer_used;
		if (mp3->stream_size && (n > (mp3->stream_size - mp3->stream->getPosition()))) n = (uint32_t)(mp3->stream_size - mp3->stream->getPosition());
		if (n) r = mp3->stream->readBytes(mp3->in_buffer + mp3->in_buffer_used, n); else r = 0;
		if (r < 0) r = 0;
		mp3->in_buffer_used += r;
		if (mp3->stream->getPosition() > mp3->stream_size) {
			try {
				mp3->stream->setPosition(mp3->stream_size);
			} catch (...) {
			}
		}
		if (!r) break;
	}
	return 0;
}

int tMP3Decoder::seek(int64_t pos, int units) {
	struct tMP3Decoder* mp3 = this;
	int64_t newpos;
	if (!(mp3->init_done)) return -1;
	if (units == MP3DEC_SEEK_START) {
		try {
			mp3->stream->setPosition(mp3->stream_start);
		} catch (...) {
			return -1;
		}
		mp3->mpadec->state = MPADEC_STATE_START;
		mp3->in_buffer_offset = mp3->in_buffer_used = 0;
		mp3->out_buffer_offset = mp3->out_buffer_used = 0;
	} else if (units == MP3DEC_SEEK_BYTES) {
		newpos = mp3->stream->getPosition() + pos;
		if (newpos > mp3->stream_size) newpos = mp3->stream_size;
		try {
			mp3->stream->setPosition((uint32_t)newpos);
		} catch (...) {
			return -1;
		}
		mp3->in_buffer_offset = mp3->in_buffer_used = 0;
		mp3->out_buffer_offset = mp3->out_buffer_used = 0;
	} else if (units == MP3DEC_SEEK_SAMPLES) {
		double fsize = (125.0*mp3->mpainfo.bitrate*mp3->mpainfo.decoded_frame_samples)/(double)mp3->mpainfo.decoded_frequency;
		newpos = mp3->stream->getPosition() + (int64_t)((double)pos*fsize/(double)mp3->mpainfo.decoded_frame_samples);
		if (newpos > mp3->stream_size) newpos = mp3->stream_size;
		try {
			mp3->stream->setPosition((uint32_t)newpos);
		} catch (...) {
			return -1;
		}
		pos = (pos % mp3->mpainfo.decoded_frame_samples)*mp3->mpainfo.decoded_sample_size;
		mp3->in_buffer_offset = mp3->in_buffer_used = 0;
		mp3->out_buffer_offset = mp3->out_buffer_used = 0;
		{
			uint8_t temp[8*1152];
			decode(temp, (uint32_t)pos, 0);
		}
	} else if (units == MP3DEC_SEEK_SECONDS) {
		if (pos > mp3->mpainfo.duration) pos = mp3->mpainfo.duration;
		if (mp3->taginfo.flags & 4) {
			int32_t n = (int32_t)((100*pos + (mp3->mpainfo.duration >> 1))/mp3->mpainfo.duration);
			if (n > 99) newpos = mp3->stream_size;
			else newpos = (mp3->taginfo.toc[n]*mp3->stream_size)/255;
		} else newpos = (pos*mp3->stream_size + (mp3->mpainfo.duration >> 1))/mp3->mpainfo.duration;
		newpos += mp3->stream->getPosition();
		if (newpos > mp3->stream_size) newpos = mp3->stream_size;
		try {
			mp3->stream->setPosition((uint32_t)newpos);
		} catch (...) {
			return -1;
		}
		mp3->in_buffer_offset = mp3->in_buffer_used = 0;
		mp3->out_buffer_offset = mp3->out_buffer_used = 0;
	} else return -1;
	return 0;
}

#ifdef __cplusplus
}
#endif
