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
 
#include "uMP3Tables.h"

#ifdef __cplusplus
extern "C" {
#endif

static void mpadec_II_decode_bitalloc(struct tMPADecoder* mpadec, uint8_t *bit_alloc, uint8_t *scalefac) {
	struct tMPAAllocTable *alloc = mpadec->frame.alloc_table;
	uint8_t *scfsi, *ba = bit_alloc, *scf = scalefac;
	unsigned i, step, sblimit2, sblimit = mpadec->frame.sblimit;
	uint32_t crclen = mpadec->bytes_left;
	uint8_t scfsi_buf[2*MPADEC_SBLIMIT];

	if (mpadec->frame.channels > 1) {
		unsigned jsbound = mpadec->frame.jsbound;
		sblimit2 = sblimit << 1;
		for (i = jsbound; i; i--, alloc += (1 << step)) {
			step = alloc->bits;
			*ba++ = (uint8_t)MPADEC_GETBITS(step);
			*ba++ = (uint8_t)MPADEC_GETBITS(step);
		}
		for (i = sblimit - jsbound; i; i--, alloc += (1 << step)) {
			step = alloc->bits;
			ba[0] = (uint8_t)MPADEC_GETBITS(step);
			ba[1] = ba[0];
			ba += 2;
		}
		ba = bit_alloc;
		scfsi = scfsi_buf;
		for (i = sblimit2; i; i--) if (*ba++) *scfsi++ = (uint8_t)MPADEC_GETBITS(2);
	} else {
		sblimit2 = sblimit;
		for (i = sblimit; i; i--, alloc += (1 << step)) {
			step = alloc->bits;
			*ba++ = (uint8_t)MPADEC_GETBITS(step);
		}
		ba = bit_alloc;
		scfsi = scfsi_buf;
		for (i = sblimit; i; i--) if (*ba++) *scfsi++ = (uint8_t)MPADEC_GETBITS(2);
	}
	if (mpadec->config.crc && mpadec->frame.CRC) {
		crclen -= mpadec->bytes_left;
		mpadec->crc = mpadec_update_crc(mpadec->crc, mpadec->next_byte - crclen, ((crclen << 3) - mpadec->bits_left));
		if (mpadec->crc != mpadec->frame.crc) mpadec->error = 1;
	}
	ba = bit_alloc;
	scfsi = scfsi_buf;
	for (i = sblimit2; i; i--) {
		if (*ba++) {
			switch (*scfsi++) {
				case 0: scf[0] = (uint8_t)MPADEC_GETBITS(6);
					scf[1] = (uint8_t)MPADEC_GETBITS(6);
					scf[2] = (uint8_t)MPADEC_GETBITS(6);
					break;
				case 1: scf[0] = (uint8_t)MPADEC_GETBITS(6);
					scf[1] = scf[0];
					scf[2] = (uint8_t)MPADEC_GETBITS(6);
					break;
				case 2: scf[0] = (uint8_t)MPADEC_GETBITS(6);
					scf[1] = scf[2] = scf[0];
					break;
				default: scf[0] = (uint8_t)MPADEC_GETBITS(6);
					scf[1] = (uint8_t)MPADEC_GETBITS(6);
					scf[2] = scf[1];
					break;
			}
			scf += 3;
		}
	}
}

static void mpadec_II_decode_samples(struct tMPADecoder* mpadec, uint8_t *bit_alloc, uint8_t *scalefac, double fraction[2][4][MPADEC_SBLIMIT], int x1) {
	struct tMPAAllocTable *alloc = mpadec->frame.alloc_table, *alloc2;
	uint8_t *ba = bit_alloc, *scf = scalefac;
	unsigned i, j, k, step, sblimit = mpadec->frame.sblimit, jsbound = mpadec->frame.jsbound;

	for (i = 0; i < jsbound; i++, alloc += (1 << step)) {
		step = alloc->bits;
		for (j = 0; j < (unsigned)mpadec->frame.channels; j++) {
			unsigned b = *ba++; int d;
			if (b) {
				alloc2 = alloc + b;
				k = alloc2->bits;
				d = alloc2->d;
				if (d < 0) {
					double cm = mpadec->tables.muls[k][scf[x1]];
					fraction[j][0][i] = ((double)((int)MPADEC_GETBITS(k) + d))*cm;
					fraction[j][1][i] = ((double)((int)MPADEC_GETBITS(k) + d))*cm;
					fraction[j][2][i] = ((double)((int)MPADEC_GETBITS(k) + d))*cm;
				} else {
					unsigned idx = MPADEC_GETBITS(k), m = scf[x1];
					uint8_t *tab = (mpadec->tables.mp2tables[d] + 3*idx);
					fraction[j][0][i] = mpadec->tables.muls[*tab++][m];
					fraction[j][1][i] = mpadec->tables.muls[*tab++][m];
					fraction[j][2][i] = mpadec->tables.muls[*tab][m];
				}
				scf += 3;
			} else fraction[j][0][i] = fraction[j][1][i] = fraction[j][2][i] = 0.0;
		}
	}
	for (i = jsbound; i < sblimit; i++, alloc += (1 << step)) {
		unsigned b = ba[1]; int d;
		step = alloc->bits;
		ba += 2;
		if (b) {
			alloc2 = alloc + b;
			k = alloc2->bits;
			d = alloc2->d;
			if (d < 0) {
				double cm = mpadec->tables.muls[k][scf[x1 + 3]];
				fraction[1][0][i] = fraction[0][0][i] = ((double)((int)MPADEC_GETBITS(k) + d))*cm;
				fraction[1][1][i] = fraction[0][1][i] = ((double)((int)MPADEC_GETBITS(k) + d))*cm;
				fraction[1][2][i] = fraction[0][2][i] = ((double)((int)MPADEC_GETBITS(k) + d))*cm;
				cm = mpadec->tables.muls[k][scf[x1]];
				fraction[0][0][i] *= cm;
				fraction[0][1][i] *= cm;
				fraction[0][2][i] *= cm;
			} else {
				unsigned idx = MPADEC_GETBITS(k), m1 = scf[x1], m2 = scf[x1 + 3];
				uint8_t *tab = (mpadec->tables.mp2tables[d] + 3*idx);
				fraction[0][0][i] = mpadec->tables.muls[*tab][m1];
				fraction[1][0][i] = mpadec->tables.muls[*tab++][m2];
				fraction[0][1][i] = mpadec->tables.muls[*tab][m1];
				fraction[1][1][i] = mpadec->tables.muls[*tab++][m2];
				fraction[0][2][i] = mpadec->tables.muls[*tab][m1];
				fraction[1][2][i] = mpadec->tables.muls[*tab][m2];
			}
			scf += 6;
		} else fraction[0][0][i] = fraction[0][1][i] = fraction[0][2][i] =
			   fraction[1][0][i] = fraction[1][1][i] = fraction[1][2][i] = 0.0;
	}
	if (sblimit > (unsigned)mpadec->frame.downsample_sblimit) sblimit = mpadec->frame.downsample_sblimit;
	for (i = sblimit; i < MPADEC_SBLIMIT; i++)
		for (j = 0; j < (unsigned)mpadec->frame.channels; j++) fraction[j][0][i] = fraction[j][1][i] = fraction[j][2][i] = 0.0;
}

void mpadec_decode_layer2(struct tMPADecoder* mpadec, uint8_t *buffer) {
	int i, j, table, single;
	double fraction[2][4][MPADEC_SBLIMIT];
	uint8_t bit_alloc[2*MPADEC_SBLIMIT];
	uint8_t scalefac[3*2*MPADEC_SBLIMIT];
	static uint8_t sblimits[5] = { 27 , 30 , 8, 12 , 30 };
	static uint8_t translate[3][2][16] = { { { 0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 1, 1, 1, 1, 1, 0 },
												   { 0, 2, 2, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 } },
										   { { 0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
												   { 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
										   { { 0, 3, 3, 3, 3, 3, 3, 0, 0, 0, 1, 1, 1, 1, 1, 0 },
												   { 0, 3, 3, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 } } };

	mpadec->error = 0;
	mpadec->bits_left = 0;
	if (mpadec->frame.LSF) table = 4;
	else table = translate[mpadec->frame.frequency_index][2 - mpadec->frame.channels][mpadec->frame.bitrate_index];
	mpadec->frame.alloc_table = mpadec_alloc_tables[table];
	mpadec->frame.sblimit = sblimits[table];
	mpadec->frame.jsbound = (uint8_t)((mpadec->frame.mode == MPG_MD_JOINT_STEREO) ? ((mpadec->frame.mode_ext + 1) << 2) : mpadec->frame.sblimit);
	if (mpadec->frame.channels > 1) switch (mpadec->config.mode) {
			case MPADEC_CONFIG_MONO:     single = 0; break;
			case MPADEC_CONFIG_CHANNEL1: single = 1; break;
			case MPADEC_CONFIG_CHANNEL2: single = 2; break;
			default:                     single = -1; break;
		} else single = 1;
	mpadec_II_decode_bitalloc(mpadec, bit_alloc, scalefac);
	for (i = 0; i < MPADEC_SCALE_BLOCK; i++) {
		mpadec_II_decode_samples(mpadec, bit_alloc, scalefac, fraction, i >> 2);
		if (!single) for (j = 0; j < 3; j++) {
				int k;
				for (k = 0; k < MPADEC_SBLIMIT; k++) fraction[0][j][k] = 0.5*(fraction[0][j][k] + fraction[1][j][k]);
			}
		if (single < 0) {
			for (j = 0; j < 3; j++, buffer += mpadec->synth_size) {
				mpadec->synth_func(mpadec, fraction[0][j], 0, buffer);
				mpadec->synth_func(mpadec, fraction[1][j], 1, buffer);
			}
		} else if (!single) {
			for (j = 0; j < 3; j++, buffer += mpadec->synth_size) {
				mpadec->synth_func(mpadec, fraction[0][j], 0, buffer);
			}
		} else {
			for (j = 0; j < 3; j++, buffer += mpadec->synth_size) {
				mpadec->synth_func(mpadec, fraction[single - 1][j], 0, buffer);
			}
		}
	}
	{
		unsigned n = mpadec->bits_left >> 3;
		mpadec->next_byte -= n;
		mpadec->bytes_left += n;
		mpadec->reservoir_size = 0;
	}
}

#ifdef __cplusplus
}
#endif
