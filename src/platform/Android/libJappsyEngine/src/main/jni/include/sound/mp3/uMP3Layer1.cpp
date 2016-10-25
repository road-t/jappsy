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

static void mpadec_I_decode_bitalloc(struct tMPADecoder* mpadec, uint8_t *bit_alloc, uint8_t *scalefac) {
	uint8_t *ba = bit_alloc, *scf = scalefac;
	uint32_t crclen = mpadec->bytes_left;

	if (mpadec->frame.channels > 1) {
		unsigned i, jsbound = mpadec->frame.jsbound;
		for (i = jsbound; i; i--) {
			*ba++ = (uint8_t)MPADEC_GETBITS(4);
			*ba++ = (uint8_t)MPADEC_GETBITS(4);
		}
		for (i = (MPADEC_SBLIMIT - jsbound); i; i--) *ba++ = (uint8_t)MPADEC_GETBITS(4);
		if (mpadec->config.crc && mpadec->frame.CRC) {
			crclen -= mpadec->bytes_left;
			mpadec->crc = mpadec_update_crc(mpadec->crc, mpadec->next_byte - crclen, ((crclen << 3) - mpadec->bits_left));
			if (mpadec->crc != mpadec->frame.crc) mpadec->error = 1;
		}
		ba = bit_alloc;
		for (i = jsbound; i; i--) {
			if (*ba++) *scf++ = (uint8_t)MPADEC_GETBITS(6);
			if (*ba++) *scf++ = (uint8_t)MPADEC_GETBITS(6);
		}
		for (i = (MPADEC_SBLIMIT - jsbound); i; i--) {
			if (*ba++) {
				*scf++ = (uint8_t)MPADEC_GETBITS(6);
				*scf++ = (uint8_t)MPADEC_GETBITS(6);
			}
		}
	} else {
		unsigned i;
		for (i = MPADEC_SBLIMIT; i; i--) *ba++ = (uint8_t)MPADEC_GETBITS(4);
		if (mpadec->config.crc && mpadec->frame.CRC) {
			crclen -= mpadec->bytes_left;
			mpadec->crc = mpadec_update_crc(mpadec->crc, mpadec->next_byte - crclen, ((crclen << 3) - mpadec->bits_left));
			if (mpadec->crc != mpadec->frame.crc) mpadec->error = 1;
		}
		ba = bit_alloc;
		for (i = MPADEC_SBLIMIT; i; i--) if (*ba++) *scf++ = (uint8_t)MPADEC_GETBITS(6);
	}
}

static void mpadec_I_decode_samples(struct tMPADecoder* mpadec, uint8_t *bit_alloc, uint8_t *scalefac, double fraction[2][MPADEC_SBLIMIT]) {
	uint8_t *ba = bit_alloc, *scf = scalefac;
	unsigned i, n;

	if (mpadec->frame.channels > 1) {
		unsigned jsbound = mpadec->frame.jsbound;
		double *f0 = fraction[0], *f1 = fraction[1];
		for (i = jsbound; i; i--) {
			if ((n = *ba++) != 0) *f0++ = (((-1) << n) + MPADEC_GETBITS(n + 1) + 1)*mpadec->tables.muls[n + 1][*scf++]; else *f0++ = 0.0;
			if ((n = *ba++) != 0) *f1++ = (((-1) << n) + MPADEC_GETBITS(n + 1) + 1)*mpadec->tables.muls[n + 1][*scf++]; else *f1++ = 0.0;
		}
		for (i = (MPADEC_SBLIMIT - jsbound); i; i--) {
			if ((n = *ba++) != 0) {
				double tmp = (((-1) << n) + MPADEC_GETBITS(n + 1) + 1);
				*f0++ = tmp*mpadec->tables.muls[n + 1][*scf++];
				*f1++ = tmp*mpadec->tables.muls[n + 1][*scf++];
			} else *f0++ = *f1++ = 0.0;
		}
		for (i = (unsigned)(MPADEC_SBLIMIT - mpadec->frame.downsample_sblimit); i; i--) *--f0 = *--f1 = 0.0;
	} else {
		double *f0 = fraction[0];
		for (i = MPADEC_SBLIMIT; i; i--) {
			if ((n = *ba++) != 0) *f0++ = (((-1) << n) + MPADEC_GETBITS(n + 1) + 1)*mpadec->tables.muls[n + 1][*scf++]; else *f0++ = 0.0;
		}
		for (i = (unsigned)(MPADEC_SBLIMIT - mpadec->frame.downsample_sblimit); i; i--) *--f0 = 0.0;
	}
}

void mpadec_decode_layer1(struct tMPADecoder* mpadec, uint8_t *buffer) {
	int i, j, single;
	double fraction[2][MPADEC_SBLIMIT];
	uint8_t bit_alloc[2*MPADEC_SBLIMIT];
	uint8_t scalefac[2*MPADEC_SBLIMIT];

	mpadec->error = 0;
	mpadec->bits_left = 0;
	mpadec->frame.jsbound = (uint8_t)((mpadec->frame.mode == MPG_MD_JOINT_STEREO) ? ((mpadec->frame.mode_ext + 1) << 2) : MPADEC_SBLIMIT);
	if (mpadec->frame.channels > 1) switch (mpadec->config.mode) {
			case MPADEC_CONFIG_MONO:     single = 0; break;
			case MPADEC_CONFIG_CHANNEL1: single = 1; break;
			case MPADEC_CONFIG_CHANNEL2: single = 2; break;
			default:                     single = -1; break;
		} else single = 1;
	mpadec_I_decode_bitalloc(mpadec, bit_alloc, scalefac);
	for (i = 0; i < MPADEC_SCALE_BLOCK; i++) {
		mpadec_I_decode_samples(mpadec, bit_alloc, scalefac, fraction);
		if (!single) for (j = 0; j < MPADEC_SBLIMIT; j++) fraction[0][j] = 0.5*(fraction[0][j] + fraction[1][j]);
		if (single < 0) {
			mpadec->synth_func(mpadec, fraction[0], 0, buffer);
			mpadec->synth_func(mpadec, fraction[1], 1, buffer);
		} else if (!single) {
			mpadec->synth_func(mpadec, fraction[0], 0, buffer);
		} else {
			mpadec->synth_func(mpadec, fraction[single - 1], 0, buffer);
		}
		buffer += mpadec->synth_size;
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
