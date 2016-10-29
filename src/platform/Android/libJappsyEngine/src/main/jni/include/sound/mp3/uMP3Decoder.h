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

#ifndef JAPPSY_UMP3DECODER_H
#define JAPPSY_UMP3DECODER_H

#include <platform.h>
#include <data/uStream.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MP3DEC_SEEK_BYTES   0
#define MP3DEC_SEEK_SAMPLES 1
#define MP3DEC_SEEK_SECONDS 2
#define MP3DEC_SEEK_START	3

#define MPADEC_SBLIMIT     32
#define MPADEC_SSLIMIT     18
#define MPADEC_SCALE_BLOCK 12

#define MPG_MD_STEREO       0
#define MPG_MD_JOINT_STEREO 1
#define MPG_MD_DUAL_CHANNEL 2
#define MPG_MD_MONO         3

#define MPADEC_STATE_UNDEFINED 0
#define MPADEC_STATE_START     1
#define MPADEC_STATE_DECODE    2

#define MPADEC_CONFIG_FULL_QUALITY 0
#define MPADEC_CONFIG_HALF_QUALITY 1

#define MPADEC_CONFIG_AUTO     0
#define MPADEC_CONFIG_MONO     1
#define MPADEC_CONFIG_STEREO   2
#define MPADEC_CONFIG_CHANNEL1 3
#define MPADEC_CONFIG_CHANNEL2 4

#define MPADEC_CONFIG_16BIT 0
#define MPADEC_CONFIG_24BIT 1
#define MPADEC_CONFIG_32BIT 2
#define MPADEC_CONFIG_FLOAT 3

#define MPADEC_CONFIG_LITTLE_ENDIAN 0
#define MPADEC_CONFIG_BIG_ENDIAN    1

#define MPADEC_CONFIG_REPLAYGAIN_NONE       0
#define MPADEC_CONFIG_REPLAYGAIN_RADIO      1
#define MPADEC_CONFIG_REPLAYGAIN_AUDIOPHILE 2
#define MPADEC_CONFIG_REPLAYGAIN_CUSTOM     3

#define MPADEC_INFO_STREAM 0
#define MPADEC_INFO_TAG    1
#define MPADEC_INFO_CONFIG 2

struct tMPATagInfo {
	uint32_t flags;
	uint32_t frames;
	uint32_t bytes;
	uint8_t toc[100];
	int32_t replay_gain[2];
	int32_t enc_delay;
	int32_t enc_padding;
};

struct tMPAAllocTable {
	uint16_t bits;
	int16_t d;
};

struct tMPANewHuff {
	uint32_t linbits;
	int16_t *table;
};

struct tMPABandInfo {
	int16_t long_idx[23];
	int16_t long_diff[22];
	int16_t short_idx[14];
	int16_t short_diff[13];
};

struct tMPAFrameInfo {
	uint8_t layer;
	uint8_t mode;
	uint8_t channels;
	uint8_t decoded_channels;
	uint8_t LSF;
	uint8_t MPEG25;
	uint8_t CRC;
	uint8_t extension;
	uint8_t mode_ext;
	uint8_t copyright;
	uint8_t original;
	uint8_t emphasis;
	uint8_t jsbound;
	uint8_t sblimit;
	uint8_t downsample;
	uint8_t downsample_sblimit;
	uint16_t crc;
	uint16_t rsvd;
	uint32_t padding;
	uint32_t bitrate_index;
	uint32_t frequency_index;
	uint32_t bitrate;
	uint32_t frequency;
	uint32_t frame_size;
	uint32_t frame_samples;
	uint32_t decoded_frequency;
	uint32_t decoded_samples;
	uint32_t decoded_size;
	struct tMPAAllocTable *alloc_table;
};

struct tMPADecoderConfig {
	uint8_t quality;
	uint8_t mode;
	uint8_t format;
	uint8_t endian;
	uint8_t replaygain;
	uint8_t skip;
	uint8_t crc;
	uint8_t dblsync;
	double gain;
};

struct tMPADecoderTables {
	double decwin[512 + 32];
	double muls[27][64];
	double gainpow2[256 + 122];
	double ispow[8207];
	double win[2][4][36];
	double *istabs[3][2][2];
	double tan1_1[16];
	double tan2_1[16];
	double tan1_2[16];
	double tan2_2[16];
	double pow1_1[2][16];
	double pow2_1[2][16];
	double pow1_2[2][16];
	double pow2_2[2][16];
	int32_t long_limit[9][23];
	int32_t short_limit[9][14];
	int32_t n_slen2[512];
	int32_t i_slen2[256];
	int32_t mapbuf0[9][152];
	int32_t mapbuf1[9][156];
	int32_t mapbuf2[9][44];
	int32_t *map[9][3];
	int32_t *mapend[9][3];
	uint8_t *mp2tables[10];
	uint8_t grp3tab[32*3];
	uint8_t grp5tab[128*3];
	uint8_t grp9tab[1024*3];
};

struct tMPAGRInfo {
	int32_t scfsi;
	uint32_t part2_3_length;
	uint32_t big_values;
	uint32_t scalefac_compress;
	uint8_t block_type;
	uint8_t mixed_block_flag;
	uint8_t preflag;
	uint8_t scalefac_scale;
	uint32_t table_select[3];
	uint32_t subblock_gain[3];
	uint32_t maxband[3];
	uint32_t maxbandl;
	uint32_t maxb;
	uint32_t region1start;
	uint32_t region2start;
	uint32_t count1table_select;
	double *full_gain[3];
	double *pow2gain;
};

struct tMPASideInfo {
	uint32_t main_data_begin;
	uint32_t private_bits;
	struct {
		struct tMPAGRInfo gr[2];
	} ch[2];
};

struct tMPADecoder {
	uint32_t state;
	uint8_t *next_byte;
	uint32_t bytes_left;
	uint32_t bit_buffer;
	uint8_t bits_left;
	uint8_t error;
	uint8_t free_format;
	uint8_t pad1;
	uint32_t sample_size;
	uint32_t prev_frame_size;
	uint32_t header;
	uint32_t hsize;
	uint32_t ssize;
	uint32_t dsize;
	uint16_t crc;
	uint16_t pad2;
	uint32_t skip_samples;
	uint32_t padding_samples;
	uint32_t padding_start;
	uint32_t decoded_frames;
	uint32_t decoded_samples;
	struct tMPATagInfo tag_info;
	uint32_t synth_size;
	double replay_gain;
	void (*synth_func)(struct tMPADecoder* mpadec, double block[MPADEC_SBLIMIT], int channel, uint8_t *buffer);
	uint32_t reservoir_size;
	uint8_t reservoir[2048];
	struct tMPAFrameInfo frame;
	struct tMPASideInfo sideinfo;
	struct tMPADecoderConfig config;
	struct tMPADecoderTables tables;
	uint32_t synth_bufoffs;
	uint8_t hybrid_block[4];
	double hybrid_in[2][MPADEC_SBLIMIT][MPADEC_SSLIMIT];
	double hybrid_out[2][MPADEC_SSLIMIT][MPADEC_SBLIMIT];
	double hybrid_buffers[2][2][MPADEC_SBLIMIT*MPADEC_SSLIMIT];
	double synth_buffers[2][2][0x110];

	void reset();
	uint32_t detect_frame_size();
	int decode_header(uint32_t header);
	uint32_t sync_buffer();
	int first_frame();
	int decode(uint8_t *srcbuf, uint32_t srcsize, uint8_t *dstbuf, uint32_t dstsize, uint32_t *srcused, uint32_t *dstused);
	int get_info(void *info, int info_type);
	int configure(struct tMPADecoderConfig *cfg);
};

struct tMPADecoderInfo {
	int32_t layer;
	int32_t channels;
	int32_t frequency;
	int32_t bitrate;
	uint8_t mode;
	uint8_t copyright;
	uint8_t original;
	uint8_t emphasis;
	int32_t frames;
	int32_t frame_size;
	int32_t frame_samples;
	int32_t decoded_channels;
	int32_t decoded_frequency;
	int32_t decoded_sample_size;
	int32_t decoded_frame_samples;
	int32_t duration;
};

struct tMP3Decoder {
	uint8_t init_done;
	struct tMPADecoder* mpadec;
	Stream* stream;
	uint32_t stream_size;
	uint32_t stream_start;
	uint32_t total_samples;
	struct tMPADecoderInfo mpainfo;
	struct tMPATagInfo taginfo;
	uint32_t in_buffer_offset;
	uint32_t in_buffer_used;
	uint32_t out_buffer_offset;
	uint32_t out_buffer_used;
	uint8_t in_buffer[0x10000];
	uint8_t out_buffer[8*1152];

	int init(Stream* stream, int nogap);
	void reset();
	int configure(struct tMPADecoderConfig *cfg);
	int get_info(void *info, int info_type);
	int decode(uint8_t *buf, uint32_t bufsize, uint32_t *used);
	int seek(int64_t pos, int units);
};

#define MPADEC_GETBITS(n) ((mpadec->bits_left >= (uint8_t)(n)) ? ((mpadec->bit_buffer >> (mpadec->bits_left -= (uint8_t)(n))) & mpadec_bitmask[n]) : mpadec_getbits(mpadec, n))

const uint32_t mpadec_bitmask[17] = {
	0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F,
	0x00FF, 0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF
};

struct tMP3Decoder* mp3dec_init();
uint32_t mpadec_getbits(struct tMPADecoder* mpadec, unsigned n);
uint16_t mpadec_update_crc(uint16_t init, uint8_t *buf, int length);
void mp3dec_uninit(struct tMP3Decoder* mp3dec);

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_UMP3DECODER_H
