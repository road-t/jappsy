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

static int mpadec_decode_layer3_sideinfo(struct tMPADecoder* mpadec) {
	int ch, gr, ms_stereo, powdiff, databits = 0;
	static const uint8_t tabs[2][5] = { { 2, 9, 5, 3, 4 }, { 1, 8, 1, 2, 9 } };
	const uint8_t *tab = tabs[mpadec->frame.LSF];

	ms_stereo = ((mpadec->frame.mode == MPG_MD_JOINT_STEREO) && (mpadec->frame.mode_ext & 2));
	powdiff = ((mpadec->frame.channels > 1) && (mpadec->config.mode == MPADEC_CONFIG_MONO)) ? 4 : 0;
	mpadec->sideinfo.main_data_begin = MPADEC_GETBITS(tab[1]);
	if (mpadec->frame.channels == 1) mpadec->sideinfo.private_bits = MPADEC_GETBITS(tab[2]);
	else mpadec->sideinfo.private_bits = MPADEC_GETBITS(tab[3]);
	if (!mpadec->frame.LSF) {
		for (ch = 0; ch < mpadec->frame.channels; ch++) {
			mpadec->sideinfo.ch[ch].gr[0].scfsi = -1;
			mpadec->sideinfo.ch[ch].gr[1].scfsi = MPADEC_GETBITS(4);
		}
	}
	for (gr = 0; gr < tab[0]; gr++) {
		for (ch = 0; ch < mpadec->frame.channels; ch++) {
			struct tMPAGRInfo *grinfo = &mpadec->sideinfo.ch[ch].gr[gr];
			grinfo->part2_3_length = MPADEC_GETBITS(12);
			grinfo->big_values = MPADEC_GETBITS(9);
			databits += grinfo->part2_3_length;
			if (grinfo->big_values > 288) grinfo->big_values = 288;
			grinfo->pow2gain = mpadec->tables.gainpow2 + 256 - MPADEC_GETBITS(8) + powdiff;
			if (ms_stereo) grinfo->pow2gain += 2;
			grinfo->scalefac_compress = MPADEC_GETBITS(tab[4]);
			if (MPADEC_GETBITS(1)) {
				grinfo->block_type = (uint8_t)MPADEC_GETBITS(2);
				grinfo->mixed_block_flag = (uint8_t)MPADEC_GETBITS(1);
				grinfo->table_select[0] = MPADEC_GETBITS(5);
				grinfo->table_select[1] = MPADEC_GETBITS(5);
				grinfo->table_select[2] = 0;
				grinfo->full_gain[0] = grinfo->pow2gain + (MPADEC_GETBITS(3) << 3);
				grinfo->full_gain[1] = grinfo->pow2gain + (MPADEC_GETBITS(3) << 3);
				grinfo->full_gain[2] = grinfo->pow2gain + (MPADEC_GETBITS(3) << 3);
				if (!grinfo->block_type) {
					mpadec->error = 1;
					return 0;
				} else mpadec->error = 0;
				if (mpadec->frame.LSF) {
					if (grinfo->block_type == 2) {
						if (grinfo->mixed_block_flag) {
							if (mpadec->frame.frequency_index == 8) grinfo->region1start = 48;
							else grinfo->region1start = 48 >> 1;
						} else {
							if (mpadec->frame.frequency_index == 8) grinfo->region1start = 36;
							else grinfo->region1start = 36 >> 1;
						}
					} else {
						if (mpadec->frame.frequency_index == 8) grinfo->region1start = 54;
						else grinfo->region1start = 54 >> 1;
					}
				} else grinfo->region1start = 36 >> 1;
				grinfo->region2start = 576 >> 1;
			} else {
				grinfo->block_type = 0;
				grinfo->mixed_block_flag = 0;
				grinfo->table_select[0] = MPADEC_GETBITS(5);
				grinfo->table_select[1] = MPADEC_GETBITS(5);
				grinfo->table_select[2] = MPADEC_GETBITS(5);
				{
					int tmp = MPADEC_GETBITS(4);
					grinfo->region1start = (uint32_t)mpadec_band_info[mpadec->frame.frequency_index].long_idx[tmp + 1] >> 1;
					tmp += MPADEC_GETBITS(3);
					grinfo->region2start = (uint32_t)mpadec_band_info[mpadec->frame.frequency_index].long_idx[tmp + 2] >> 1;
				}
			}
			if (!mpadec->frame.LSF) grinfo->preflag = (uint8_t)MPADEC_GETBITS(1);
			grinfo->scalefac_scale = (uint8_t)MPADEC_GETBITS(1);
			grinfo->count1table_select = (uint8_t)MPADEC_GETBITS(1);
		}
	}
	databits -= 8*mpadec->sideinfo.main_data_begin;
	return databits;
}

static int mpadec_III_get_scale_factors(struct tMPADecoder* mpadec, tMPAGRInfo* gr_info, int32_t *scf) {
	struct tMPAGRInfo *grinfo = gr_info;
	int numbits = 0;
	static uint8_t slen[2][16] = { {0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4},
								   {0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3} };
	static uint8_t stab[3][6][4] = { { { 6, 5, 5,5 } , { 6, 5, 7,3 } , { 11,10,0,0} ,
											 { 7, 7, 7,0 } , { 6, 6, 6,3 } , { 8, 8,5,0} } ,
									 { { 9, 9, 9,9 } , { 9, 9,12,6 } , { 18,18,0,0} ,
											 {12,12,12,0 } , {12, 9, 9,6 } , { 15,12,9,0} } ,
									 { { 6, 9, 9,9 } , { 6, 9,12,6 } , { 15,18,0,0} ,
											 { 6,15,12,0 } , { 6,12, 9,6 } , { 6,18,9,0} } };

	if (!mpadec->frame.LSF) {
		int i, num0 = slen[0][grinfo->scalefac_compress], num1 = slen[1][grinfo->scalefac_compress];
		if (grinfo->block_type == 2) {
			i = 18; numbits = 18*(num0 + num1);
			if (grinfo->mixed_block_flag) {
				i--;
				numbits -= num0;
			}
			for (; i; i--) *scf++ = MPADEC_GETBITS((unsigned)num0);
			for (i = 18; i; i--) *scf++ = MPADEC_GETBITS((unsigned)num1);
			scf[0] = scf[1] = scf[2] = 0;
		} else {
			if (grinfo->scfsi < 0) {
				for (i = 11; i; i--) *scf++ = MPADEC_GETBITS((unsigned)num0);
				for (i = 10; i; i--) *scf++ = MPADEC_GETBITS((unsigned)num1);
				numbits = 10*(num0 + num1) + num0;
				*scf = 0;
			} else {
				numbits = 0;
				if (!(grinfo->scfsi & 8)) {
					for (i = 6; i; i--) *scf++ = MPADEC_GETBITS((unsigned)num0);
					numbits += 6*num0;
				} else scf += 6;
				if (!(grinfo->scfsi & 4)) {
					for (i = 5; i; i--) *scf++ = MPADEC_GETBITS((unsigned)num0);
					numbits += 5*num0;
				} else scf += 5;
				if (!(grinfo->scfsi & 2)) {
					for (i = 5; i; i--) *scf++ = MPADEC_GETBITS((unsigned)num1);
					numbits += 5*num1;
				} else scf += 5;
				if (!(grinfo->scfsi & 1)) {
					for (i = 5; i; i--) *scf++ = MPADEC_GETBITS((unsigned)num1);
					numbits += 5*num1;
				} else scf += 5;
				*scf = 0;
			}
		}
	} else {
		int i, j, n = 0;
		unsigned s_len; uint8_t *pnt;
		if ((mpadec->frame.mode == MPG_MD_JOINT_STEREO) && (mpadec->frame.mode_ext & 1)) {
			s_len = (unsigned)mpadec->tables.i_slen2[grinfo->scalefac_compress >> 1];
		} else s_len = (unsigned)mpadec->tables.n_slen2[grinfo->scalefac_compress];
		grinfo->preflag = (uint8_t)((s_len >> 15) & 1);
		if (grinfo->block_type == 2) n = grinfo->mixed_block_flag ? 2 : 1;
		pnt = stab[n][(s_len >> 12) & 7];
		for (i = 0; i < 4; i++) {
			int num = s_len & 7;
			s_len >>= 3;
			if (num) {
				for (j = 0; j < (int)pnt[i]; j++) *scf++ = MPADEC_GETBITS((unsigned)num);
				numbits += pnt[i]*num;
			} else for (j = 0; j < (int)pnt[i]; j++) *scf++ = 0;
		}
		for (i = (n << 1) + 1; i; i--) *scf++ = 0;
	}
	return numbits;
}

static int mpadec_III_decode_samples(struct tMPADecoder* mpadec, tMPAGRInfo* gr_info, double xr[MPADEC_SBLIMIT][MPADEC_SSLIMIT], int32_t *scf, int part2bits) {
	struct tMPAGRInfo* grinfo = gr_info;
	int shift = 1 + grinfo->scalefac_scale, l[3], l3;
	int part2remain = grinfo->part2_3_length - part2bits;
	double* xrptr = (double*)xr; int32_t *me;
	static uint8_t pretab1[22] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 3, 3, 2, 0 };
	static uint8_t pretab2[22] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	l3 = ((576 >> 1) - grinfo->big_values) >> 1;
	if (grinfo->big_values <= grinfo->region1start) {
		l[0] = grinfo->big_values;
		l[1] = l[2] = 0;
	} else {
		l[0] = grinfo->region1start;
		if (grinfo->big_values <= grinfo->region2start) {
			l[1] = grinfo->big_values - l[0]; l[2] = 0;
		} else {
			l[1] = grinfo->region2start - l[0];
			l[2] = grinfo->big_values - grinfo->region2start;
		}
	}
	if (grinfo->block_type == 2) {
		int32_t i, max[4], step = 0, lwin = 0, cb = 0;
		double v = 0.0;
		int32_t *m, mc;

		if (grinfo->mixed_block_flag) {
			max[3] = -1;
			max[0] = max[1] = max[2] = 2;
			m = mpadec->tables.map[mpadec->frame.frequency_index][0];
			me = mpadec->tables.mapend[mpadec->frame.frequency_index][0];
		} else {
			max[0] = max[1] = max[2] = max[3] = -1;
			m = mpadec->tables.map[mpadec->frame.frequency_index][1];
			me = mpadec->tables.mapend[mpadec->frame.frequency_index][1];
		}
		mc = 0;
		for (i = 0; i < 2; i++) {
			int lp = l[i];
			struct tMPANewHuff *h = mpadec_hufft + grinfo->table_select[i];
			for (; lp; lp--, mc--) {
				int x, y;
				if (!mc) {
					mc = *m++;
					xrptr = ((double *)xr) + (*m++);
					lwin = *m++;
					cb = *m++;
					if (lwin == 3) {
						v = grinfo->pow2gain[(*scf++) << shift];
						step = 1;
					} else {
						v = grinfo->full_gain[lwin][(*scf++) << shift];
						step = 3;
					}
				}
				{
					int16_t *val = h->table;
					while ((y = *val++) < 0) {
						if (MPADEC_GETBITS(1)) val -= y;
						part2remain--;
					}
					x = y >> 4;
					y &= 0x0F;
				}
				if ((x == 15) && h->linbits) {
					max[lwin] = cb;
					part2remain -= h->linbits + 1;
					x += MPADEC_GETBITS(h->linbits);
					if (MPADEC_GETBITS(1)) *xrptr = -mpadec->tables.ispow[x]*v;
					else *xrptr = mpadec->tables.ispow[x]*v;
				} else if (x) {
					max[lwin] = cb;
					if (MPADEC_GETBITS(1)) *xrptr = -mpadec->tables.ispow[x]*v;
					else *xrptr = mpadec->tables.ispow[x]*v;
					part2remain--;
				} else *xrptr = 0.0;
				xrptr += step;
				if ((y == 15) && h->linbits) {
					max[lwin] = cb;
					part2remain -= h->linbits + 1;
					y += MPADEC_GETBITS(h->linbits);
					if (MPADEC_GETBITS(1)) *xrptr = -mpadec->tables.ispow[y]*v;
					else *xrptr = mpadec->tables.ispow[y]*v;
				} else if (y) {
					max[lwin] = cb;
					if (MPADEC_GETBITS(1)) *xrptr = -mpadec->tables.ispow[y]*v;
					else *xrptr = mpadec->tables.ispow[y]*v;
					part2remain--;
				} else *xrptr = 0.0;
				xrptr += step;
			}
		}
		for (; l3 && (part2remain > 0); l3--) {
			struct tMPANewHuff *h = mpadec_hufftc + grinfo->count1table_select;
			int16_t *val = h->table, a;
			while ((a = *val++) < 0) {
				part2remain--;
				if (part2remain < 0) {
					part2remain++;
					a = 0;
					break;
				}
				if (MPADEC_GETBITS(1)) val -= a;
			}
			for (i = 0; i < 4; i++) {
				if (!(i & 1)) {
					if (!mc) {
						mc = *m++;
						xrptr = ((double *)xr) + (*m++);
						lwin = *m++;
						cb = *m++;
						if (lwin == 3) {
							v = grinfo->pow2gain[(*scf++) << shift];
							step = 1;
						} else {
							v = grinfo->full_gain[lwin][(*scf++) << shift];
							step = 3;
						}
					}
					mc--;
				}
				if (a & (8 >> i)) {
					max[lwin] = cb;
					part2remain--;
					if (part2remain < 0) {
						part2remain++;
						break;
					}
					if (MPADEC_GETBITS(1)) *xrptr = -v;
					else *xrptr = v;
				} else *xrptr = 0.0;
				xrptr += step;
			}
		}
		if (lwin < 3) {
			while (1) {
				for (; mc > 0; mc--) {
					xrptr[0] = xrptr[3] = 0.0;
					xrptr += 6;
				}
				if (m >= me) break;
				mc = *m++;
				xrptr = ((double *)xr) + (*m++);
				if ((*m++) == 0) break;
				m++;
			}
		}
		grinfo->maxband[0] = (uint32_t)(max[0] + 1);
		grinfo->maxband[1] = (uint32_t)(max[1] + 1);
		grinfo->maxband[2] = (uint32_t)(max[2] + 1);
		grinfo->maxbandl = (uint32_t)(max[3] + 1);
		{
			int rmax = max[0] > max[1] ? max[0] : max[1];
			rmax = (rmax > max[2] ? rmax : max[2]) + 1;
			grinfo->maxb = rmax ? (uint32_t)mpadec->tables.short_limit[mpadec->frame.frequency_index][rmax] : (uint32_t)mpadec->tables.long_limit[mpadec->frame.frequency_index][max[3] + 1];
		}
	} else {
		uint8_t *pretab = grinfo->preflag ? (uint8_t*)pretab1 : (uint8_t*)pretab2;
		int32_t i, max = -1, cb = 0, mc = 0;
		int32_t *m = mpadec->tables.map[mpadec->frame.frequency_index][2];
		double v = 0.0;

		for (i = 0; i < 3; i++) {
			int lp = l[i];
			struct tMPANewHuff *h = mpadec_hufft + grinfo->table_select[i];
			for (; lp; lp--, mc--) {
				int x, y;
				if (!mc) {
					mc = *m++;
					cb = *m++;
					if (cb == 21) v = 0.0;
					else v = grinfo->pow2gain[((*scf++) + (*pretab++)) << shift];
				}
				{
					int16_t *val = h->table;
					while ((y = *val++) < 0) {
						if (MPADEC_GETBITS(1)) val -= y;
						part2remain--;
					}
					x = y >> 4;
					y &= 0x0F;
				}
				if ((x == 15) && h->linbits) {
					max = cb;
					part2remain -= h->linbits + 1;
					x += MPADEC_GETBITS(h->linbits);
					if (MPADEC_GETBITS(1)) *xrptr++ = -mpadec->tables.ispow[x]*v;
					else *xrptr++ = mpadec->tables.ispow[x]*v;
				} else if (x) {
					max = cb;
					if (MPADEC_GETBITS(1)) *xrptr++ = -mpadec->tables.ispow[x]*v;
					else *xrptr++ = mpadec->tables.ispow[x]*v;
					part2remain--;
				} else *xrptr++ = 0.0;
				if ((y == 15) && h->linbits) {
					max = cb;
					part2remain -= h->linbits + 1;
					y += MPADEC_GETBITS(h->linbits);
					if (MPADEC_GETBITS(1)) *xrptr++ = -mpadec->tables.ispow[y]*v;
					else *xrptr++ = mpadec->tables.ispow[y]*v;
				} else if (y) {
					max = cb;
					if (MPADEC_GETBITS(1)) *xrptr++ = -mpadec->tables.ispow[y]*v;
					else *xrptr++ = mpadec->tables.ispow[y]*v;
					part2remain--;
				} else *xrptr++ = 0.0;
			}
		}
		for (; l3 && (part2remain > 0); l3--) {
			struct tMPANewHuff *h = mpadec_hufftc + grinfo->count1table_select;
			int16_t *val = h->table, a;
			while ((a = *val++) < 0) {
				part2remain--;
				if (part2remain < 0) {
					part2remain++;
					a = 0;
					break;
				}
				if (MPADEC_GETBITS(1)) val -= a;
			}
			for (i = 0; i < 4; i++) {
				if (!(i & 1)) {
					if (!mc) {
						mc = *m++;
						cb = *m++;
						v = grinfo->pow2gain[((*scf++) + (*pretab++)) << shift];
					}
					mc--;
				}
				if (a & (8 >> i)) {
					max = cb;
					part2remain--;
					if (part2remain < 0) {
						part2remain++;
						break;
					}
					if (MPADEC_GETBITS(1)) *xrptr++ = -v;
					else *xrptr++ = v;
				} else *xrptr++ = 0.0;
			}
		}
		grinfo->maxbandl = (uint32_t)(max + 1);
		grinfo->maxb = (uint32_t)mpadec->tables.long_limit[mpadec->frame.frequency_index][max + 1];
	}
	while (xrptr < &xr[MPADEC_SBLIMIT][0]) *xrptr++ = 0.0;
	while (part2remain > 0) {
		unsigned /*tmp, */i = (part2remain > 16) ? 16 : (unsigned)part2remain;
		/*tmp = */MPADEC_GETBITS(i);
		part2remain -= i;
		//i = tmp;
	}
	mpadec->error = (uint8_t)((part2remain < 0) ? 1 : 0);
	return mpadec->error;
}

static void mpadec_III_i_stereo(struct tMPADecoder* mpadec, tMPAGRInfo* gr_info, double xrbuf[2][MPADEC_SBLIMIT][MPADEC_SSLIMIT], int32_t *scalefac) {
	struct tMPAGRInfo* grinfo = gr_info;
	double (*xr)[MPADEC_SBLIMIT*MPADEC_SSLIMIT] = (double (*)[MPADEC_SBLIMIT*MPADEC_SSLIMIT])xrbuf;
	struct tMPABandInfo *bi = &mpadec_band_info[mpadec->frame.frequency_index];
	int tab = mpadec->frame.LSF + (grinfo->scalefac_compress & mpadec->frame.LSF);
	int ms_stereo = ((mpadec->frame.mode == MPG_MD_JOINT_STEREO) &&
					 (mpadec->frame.mode_ext & 2)) ? 1 : 0;
	const double *tab1, *tab2;

	tab1 = mpadec->tables.istabs[tab][ms_stereo][0];
	tab2 = mpadec->tables.istabs[tab][ms_stereo][1];
	if (grinfo->block_type == 2) {
		int lwin, do_l = grinfo->mixed_block_flag;
		for (lwin = 0; lwin < 3; lwin++) {
			int32_t is_p, sb, idx, sfb = grinfo->maxband[lwin];
			if (sfb > 3) do_l = 0;
			for (; sfb < 12; sfb++) {
				is_p = scalefac[3*sfb + lwin - grinfo->mixed_block_flag];
				if (is_p != 7) {
					double t1 = tab1[is_p], t2 = tab2[is_p];
					sb = bi->short_diff[sfb];
					idx = bi->short_idx[sfb] + lwin;
					for (; sb; sb--, idx += 3) {
						double v = xr[0][idx];
						xr[0][idx] = v*t1;
						xr[1][idx] = v*t2;
					}
				}
			}
			is_p = scalefac[3*11 + lwin - grinfo->mixed_block_flag];
			sb = bi->short_diff[12];
			idx = bi->short_idx[12] + lwin;
			if (is_p != 7) {
				double t1 = tab1[is_p], t2 = tab2[is_p];
				for (; sb; sb--, idx += 3) {
					double v = xr[0][idx];
					xr[0][idx] = v*t1;
					xr[1][idx] = v*t2;
				}
			}
		}
		if (do_l) {
			int sfb = grinfo->maxbandl;
			int idx = bi->long_idx[sfb];
			for (; sfb < 8; sfb++) {
				int sb = bi->long_diff[sfb];
				int is_p = scalefac[sfb];
				if (is_p != 7) {
					double t1 = tab1[is_p], t2 = tab2[is_p];
					for (; sb; sb--, idx++) {
						double v = xr[0][idx];
						xr[0][idx] = v*t1;
						xr[1][idx] = v*t2;
					}
				} else idx += sb;
			}
		}
	} else {
		int sfb = grinfo->maxbandl;
		int is_p, idx = bi->long_idx[sfb];
		for (; sfb < 21; sfb++) {
			int sb = bi->long_diff[sfb];
			is_p = scalefac[sfb];
			if (is_p != 7) {
				double t1 = tab1[is_p], t2 = tab2[is_p];
				for (; sb; sb--, idx++) {
					double v = xr[0][idx];
					xr[0][idx] = v*t1;
					xr[1][idx] = v*t2;
				}
			} else idx += sb;
		}
		is_p = scalefac[20];
		if (is_p != 7) {
			int sb = bi->long_diff[21];
			double t1 = tab1[is_p], t2 = tab2[is_p];
			for (; sb; sb--, idx++) {
				double v = xr[0][idx];
				xr[0][idx] = v*t1;
				xr[1][idx] = v*t2;
			}
		}
	}
}

static void mpadec_III_antialias(tMPAGRInfo *gr_info, double xr[MPADEC_SBLIMIT][MPADEC_SSLIMIT]) {
	struct tMPAGRInfo *grinfo = gr_info;
	int sblim;

	if (grinfo->block_type == 2) {
		if (!grinfo->mixed_block_flag) return;
		sblim = 1;
	} else sblim = grinfo->maxb - 1;
	{
		int sb;
		double *xr1 = (double *)xr[1];
		for (sb = sblim; sb; sb--, xr1 += 10) {
			int ss;
			double *xr2 = xr1;
			for (ss = 0; ss < 8; ss++) {
				double bu = *--xr2, bd = *xr1;
				*xr2 = bu*mpadec_cs[ss] - bd*mpadec_ca[ss];
				*xr1++ = bd*mpadec_cs[ss] + bu*mpadec_ca[ss];
			}
		}
	}
}

static void mpadec_dct36(double *in, double *out1, double *out2, double *w, double *ts) {
	double tmp[18];
	{
		in[17] += in[16]; in[16] += in[15]; in[15] += in[14];
		in[14] += in[13]; in[13] += in[12]; in[12] += in[11];
		in[11] += in[10]; in[10] += in[9];	in[9]	+= in[8];
		in[8]	+= in[7];	in[7]	+= in[6];	in[6]	+= in[5];
		in[5]	+= in[4];	in[4]	+= in[3];	in[3]	+= in[2];
		in[2]	+= in[1];	in[1]	+= in[0];

		in[17] += in[15]; in[15] += in[13]; in[13] += in[11]; in[11] += in[9];
		in[9]	+= in[7];	in[7]	+= in[5];	in[5]	+= in[3];	in[3]	+= in[1];
		{
			double t3;
			{
				double t0, t1, t2;

				t0 = mpadec_newcos[7]*(in[8] + in[16] - in[4]);
				t1 = mpadec_newcos[7]*in[12];
				t3 = in[0];
				t2 = t3 - t1 - t1;
				tmp[1] = tmp[7] = t2 - t0;
				tmp[4] = t2 + t0 + t0;
				t3 += t1;
				t2 = mpadec_newcos[6]*(in[10] + in[14] - in[2]);
				tmp[1] -= t2;
				tmp[7] += t2;
			}
			{
				double t0, t1, t2;

				t0 = mpadec_newcos[0]*(in[4] + in[8]);
				t1 = mpadec_newcos[1]*(in[8] - in[16]);
				t2 = mpadec_newcos[2]*(in[4] + in[16]);
				tmp[2] = tmp[6] = t3 - t0 - t2;
				tmp[0] = tmp[8] = t3 + t0 + t1;
				tmp[3] = tmp[5] = t3 - t1 + t2;
			}
		}
		{
			double t1, t2, t3;

			t1 = mpadec_newcos[3]*(in[2] + in[10]);
			t2 = mpadec_newcos[4]*(in[10] - in[14]);
			t3 = mpadec_newcos[6]*in[6];
			{
				double t0 = t1 + t2 + t3;
				tmp[0] += t0;
				tmp[8] -= t0;
			}
			t2 -= t3;
			t1 -= t3;
			t3 = mpadec_newcos[5]*(in[2] + in[14]);
			t1 += t3;
			tmp[3] += t1;
			tmp[5] -= t1;
			t2 -= t3;
			tmp[2] += t2;
			tmp[6] -= t2;
		}
		{
			double t0, t1, t2, t3, t4, t5, t6, t7;

			t1 = mpadec_newcos[7]*in[13];
			t2 = mpadec_newcos[7]*(in[9] + in[17] - in[5]);
			t3 = in[1] + t1;
			t4 = in[1] - t1 - t1;
			t5 = t4 - t2;
			t0 = mpadec_newcos[0]*(in[5] + in[9]);
			t1 = mpadec_newcos[1]*(in[9] - in[17]);
			tmp[13] = (t4 + t2 + t2)*mpadec_tfcos36[17 - 13];
			t2 = mpadec_newcos[2]*(in[5] + in[17]);
			t6 = t3 - t0 - t2;
			t0 += t3 + t1;
			t3 += t2 - t1;
			t2 = mpadec_newcos[3]*(in[3] + in[11]);
			t4 = mpadec_newcos[4]*(in[11] - in[15]);
			t7 = mpadec_newcos[6]*in[7];
			t1 = t2 + t4 + t7;
			tmp[17] = (t0 + t1)*mpadec_tfcos36[17 - 17];
			tmp[9] = (t0 - t1)*mpadec_tfcos36[17 - 9];
			t1 = mpadec_newcos[5]*(in[3] + in[15]);
			t2 += t1 - t7;
			tmp[14] = (t3 + t2)*mpadec_tfcos36[17 - 14];
			t0 = mpadec_newcos[6]*(in[11] + in[15] - in[3]);
			tmp[12] = (t3 - t2)*mpadec_tfcos36[17 - 12];
			t4 -= t1 + t7;
			tmp[16] = (t5 - t0)*mpadec_tfcos36[17 - 16];
			tmp[10] = (t5 + t0)*mpadec_tfcos36[17 - 10];
			tmp[15] = (t6 + t4)*mpadec_tfcos36[17 - 15];
			tmp[11] = (t6 - t4)*mpadec_tfcos36[17 - 11];
		}
	}
#define MPADEC_DCT36_MACRO(v) { \
		double tmpval = tmp[(v)] + tmp[17 - (v)]; \
		out2[9 + (v)] = tmpval*w[27 + (v)]; \
		out2[8 - (v)] = tmpval*w[26 - (v)]; \
		tmpval = tmp[(v)] - tmp[17 - (v)]; \
		ts[MPADEC_SBLIMIT*(8 - (v))] = out1[8 - (v)] + tmpval*w[8 - (v)]; \
		ts[MPADEC_SBLIMIT*(9 + (v))] = out1[9 + (v)] + tmpval*w[9 + (v)]; \
	}
	{
		MPADEC_DCT36_MACRO(0);
		MPADEC_DCT36_MACRO(1);
		MPADEC_DCT36_MACRO(2);
		MPADEC_DCT36_MACRO(3);
		MPADEC_DCT36_MACRO(4);
		MPADEC_DCT36_MACRO(5);
		MPADEC_DCT36_MACRO(6);
		MPADEC_DCT36_MACRO(7);
		MPADEC_DCT36_MACRO(8);
	}
#undef MPADEC_DCT36_MACRO
}

static void mpadec_dct12(double *in, double *out1, double *out2, double *w, double *ts)
{
#define MPADEC_DCT12_PART1 in5 = in[5*3]; \
	in5 += (in4 = in[4*3]); \
	in4 += (in3 = in[3*3]); \
	in3 += (in2 = in[2*3]); \
	in2 += (in1 = in[1*3]); \
	in1 += (in0 = in[0*3]); \
	in5 += in3; in3 += in1; \
	in2 *= mpadec_newcos[6]; \
	in3 *= mpadec_newcos[6];

#define MPADEC_DCT12_PART2 in0 += in4*mpadec_newcos[7]; \
	in4 = in0 + in2; \
	in0 -= in2; \
	in1 += in5*mpadec_newcos[7]; \
	in5 = (in1 + in3)*mpadec_tfcos12[0]; \
	in1 = (in1 - in3)*mpadec_tfcos12[2]; \
	in3 = in4 + in5; \
	in4 -= in5; \
	in2 = in0 + in1; \
	in0 -= in1;

	{
		double in0, in1, in2, in3, in4, in5;

		ts[0*MPADEC_SBLIMIT] = out1[0]; ts[1*MPADEC_SBLIMIT] = out1[1]; ts[2*MPADEC_SBLIMIT] = out1[2];
		ts[3*MPADEC_SBLIMIT] = out1[3]; ts[4*MPADEC_SBLIMIT] = out1[4]; ts[5*MPADEC_SBLIMIT] = out1[5];

		MPADEC_DCT12_PART1

		{
			double tmp0, tmp1 = in0 - in4;
			{
				double tmp2 = (in1 - in5)*mpadec_tfcos12[1];
				tmp0 = tmp1 + tmp2;
				tmp1 -= tmp2;
			}
			ts[(17 - 1)*MPADEC_SBLIMIT] = out1[17 - 1] + tmp0*w[11 - 1];
			ts[(12 + 1)*MPADEC_SBLIMIT] = out1[12 + 1] + tmp0*w[6 + 1];
			ts[(6 + 1)*MPADEC_SBLIMIT] = out1[6 + 1] + tmp1*w[1];
			ts[(11 - 1)*MPADEC_SBLIMIT] = out1[11 - 1] + tmp1*w[5 - 1];
		}

		MPADEC_DCT12_PART2

		ts[(17 - 0)*MPADEC_SBLIMIT] = out1[17 - 0] + in2*w[11 - 0];
		ts[(12 + 0)*MPADEC_SBLIMIT] = out1[12 + 0] + in2*w[6 + 0];
		ts[(12 + 2)*MPADEC_SBLIMIT] = out1[12 + 2] + in3*w[6 + 2];
		ts[(17 - 2)*MPADEC_SBLIMIT] = out1[17 - 2] + in3*w[11 - 2];
		ts[(6 + 0)*MPADEC_SBLIMIT]	= out1[6 + 0] + in0*w[0];
		ts[(11 - 0)*MPADEC_SBLIMIT] = out1[11 - 0] + in0*w[5 - 0];
		ts[(6 + 2)*MPADEC_SBLIMIT]	= out1[6 + 2] + in4*w[2];
		ts[(11 - 2)*MPADEC_SBLIMIT] = out1[11 - 2] + in4*w[5 - 2];
	}
	in++;
	{
		double in0, in1, in2, in3, in4, in5;

		MPADEC_DCT12_PART1

		{
			double tmp0, tmp1 = in0 - in4;
			{
				double tmp2 = (in1 - in5)*mpadec_tfcos12[1];
				tmp0 = tmp1 + tmp2;
				tmp1 -= tmp2;
			}
			out2[5 - 1] = tmp0*w[11 - 1];
			out2[0 + 1] = tmp0*w[6 + 1];
			ts[(12 + 1)*MPADEC_SBLIMIT] += tmp1*w[0 + 1];
			ts[(17 - 1)*MPADEC_SBLIMIT] += tmp1*w[5 - 1];
		}

		MPADEC_DCT12_PART2

		out2[5 - 0] = in2*w[11 - 0];
		out2[0 + 0] = in2*w[6 + 0];
		out2[0 + 2] = in3*w[6 + 2];
		out2[5 - 2] = in3*w[11 - 2];
		ts[(12 + 0)*MPADEC_SBLIMIT] += in0*w[0];
		ts[(17 - 0)*MPADEC_SBLIMIT] += in0*w[5 - 0];
		ts[(12 + 2)*MPADEC_SBLIMIT] += in4*w[2];
		ts[(17 - 2)*MPADEC_SBLIMIT] += in4*w[5 - 2];
	}
	in++;
	{
		double in0, in1, in2, in3, in4, in5;

		out2[12] = out2[13] = out2[14] = out2[15] = out2[16] = out2[17] = 0.0;

		MPADEC_DCT12_PART1

		{
			double tmp0, tmp1 = in0 - in4;
			{
				double tmp2 = (in1 - in5)*mpadec_tfcos12[1];
				tmp0 = tmp1 + tmp2;
				tmp1 -= tmp2;
			}
			out2[11 - 1] = tmp0*w[11 - 1];
			out2[6 + 1] = tmp0*w[6 + 1];
			out2[0 + 1] += tmp1*w[1];
			out2[5 - 1] += tmp1*w[5 - 1];
		}

		MPADEC_DCT12_PART2

		out2[11 - 0] = in2*w[11 - 0];
		out2[6 + 0] = in2*w[6 + 0];
		out2[6 + 2] = in3*w[6 + 2];
		out2[11 - 2] = in3*w[11 - 2];
		out2[0 + 0] += in0*w[0];
		out2[5 - 0] += in0*w[5 - 0];
		out2[0 + 2] += in4*w[2];
		out2[5 - 2] += in4*w[5 - 2];
	}
#undef MPADEC_DCT12_PART1
#undef MPADEC_DCT12_PART2
}

static void mpadec_III_hybrid(struct tMPADecoder* mpadec, tMPAGRInfo *gr_info, double fs_in[MPADEC_SBLIMIT][MPADEC_SSLIMIT], double ts_out[MPADEC_SSLIMIT][MPADEC_SBLIMIT], int channel) {
	struct tMPAGRInfo* grinfo = gr_info;
	double *tsptr = (double *)ts_out;
	double *out1, *out2;
	unsigned bt = grinfo->block_type, sb = 0;

	{
		unsigned b = mpadec->hybrid_block[channel];
		out1 = mpadec->hybrid_buffers[b][channel];
		b ^= 1;
		out2 = mpadec->hybrid_buffers[b][channel];
		mpadec->hybrid_block[channel] = (uint8_t)b;
	}
	if (grinfo->mixed_block_flag) {
		sb = 2;
		mpadec_dct36(fs_in[0], out1, out2, mpadec->tables.win[0][0], tsptr);
		mpadec_dct36(fs_in[1], out1 + MPADEC_SSLIMIT, out2 + MPADEC_SSLIMIT, mpadec->tables.win[1][0], tsptr + 1);
		out1 += 36; out2 += 36; tsptr += 2;
	}
	if (bt == 2) {
		for (; sb < grinfo->maxb; sb += 2, out1 += 36, out2 += 36, tsptr += 2) {
			mpadec_dct12(fs_in[sb], out1, out2, mpadec->tables.win[0][2], tsptr);
			mpadec_dct12(fs_in[sb + 1], out1 + MPADEC_SSLIMIT, out2 + MPADEC_SSLIMIT, mpadec->tables.win[1][2], tsptr + 1);
		}
	} else {
		for (; sb < grinfo->maxb; sb += 2, out1 += 36, out2 += 36, tsptr += 2) {
			mpadec_dct36(fs_in[sb], out1, out2, mpadec->tables.win[0][bt], tsptr);
			mpadec_dct36(fs_in[sb + 1], out1 + MPADEC_SSLIMIT, out2 + MPADEC_SSLIMIT, mpadec->tables.win[1][bt], tsptr + 1);
		}
	}
	for (; sb < MPADEC_SBLIMIT; sb++, tsptr++) {
		int i;
		for (i = 0; i < MPADEC_SSLIMIT; i++) {
			tsptr[i*MPADEC_SBLIMIT] = *out1++;
			*out2++ = 0.0;
		}
	}
}

void mpadec_decode_layer3(struct tMPADecoder* mpadec, uint8_t *buffer) {
	uint8_t *saved_next_byte = mpadec->next_byte;
	uint32_t saved_bytes_left = mpadec->bytes_left;
	int32_t dbits, scalefacs[2][39];
	int ch, gr, ss, i_stereo, ms_stereo, single, channels, granules = mpadec->frame.LSF ? 1 : 2;

	mpadec->error = 0;
	mpadec->bits_left = 0;
	if (mpadec->config.crc && mpadec->frame.CRC) {
		mpadec->crc = mpadec_update_crc(mpadec->crc, mpadec->next_byte, mpadec->ssize << 3);
		if (mpadec->crc != mpadec->frame.crc) mpadec->error = 1;
	}
	dbits = mpadec_decode_layer3_sideinfo(mpadec);
	mpadec->dsize = (uint32_t)(((dbits < 0) ? 0 : dbits) + 7) >> 3;
	mpadec->next_byte = saved_next_byte + mpadec->ssize;
	mpadec->bytes_left = saved_bytes_left - mpadec->ssize;
	mpadec->bits_left = 0;
	saved_next_byte = 0;
	saved_bytes_left = mpadec->bytes_left;
	if (mpadec->error) mpadec->sideinfo.main_data_begin = mpadec->reservoir_size + 1;
	if (mpadec->sideinfo.main_data_begin) {
		if (mpadec->sideinfo.main_data_begin <= mpadec->reservoir_size) {
			uint8_t *ptr = mpadec->reservoir + mpadec->reservoir_size;
			uint32_t tmp = mpadec->frame.frame_size - mpadec->hsize - mpadec->ssize;
			if (tmp > (sizeof(mpadec->reservoir) - mpadec->reservoir_size)) tmp = sizeof(mpadec->reservoir) - mpadec->reservoir_size;
			saved_next_byte = mpadec->next_byte;
			memcpy(ptr, mpadec->next_byte, tmp);
			mpadec->next_byte = ptr - mpadec->sideinfo.main_data_begin;
			mpadec->bytes_left = mpadec->sideinfo.main_data_begin + tmp;
		} else {
			uint32_t tmp = mpadec->frame.frame_size - mpadec->hsize - mpadec->ssize;
			if (tmp > 512) {
				mpadec->next_byte += tmp - 512;
				mpadec->bytes_left -= tmp - 512;
				tmp = 512;
			}
			if ((mpadec->reservoir_size) && (mpadec->reservoir_size > 512)) {
				memmove(mpadec->reservoir, mpadec->reservoir + mpadec->reservoir_size - 512, 512);
				mpadec->reservoir_size = 512;
			}
			memcpy(mpadec->reservoir + mpadec->reservoir_size, mpadec->next_byte, tmp);
			mpadec->reservoir_size += tmp;
			mpadec->next_byte += tmp;
			mpadec->bytes_left -= tmp;
			memset(buffer, 0, mpadec->frame.decoded_size);
			mpadec->error = 1;
			return;
		}
	}
	if (mpadec->frame.mode == MPG_MD_JOINT_STEREO) {
		i_stereo = mpadec->frame.mode_ext & 1;
		ms_stereo = (mpadec->frame.mode_ext & 2) >> 1;
	} else i_stereo = ms_stereo = 0;
	if (mpadec->frame.channels > 1) switch (mpadec->config.mode) {
			case MPADEC_CONFIG_MONO:		 single = 0; break;
			case MPADEC_CONFIG_CHANNEL1: single = 1; break;
			case MPADEC_CONFIG_CHANNEL2: single = 2; break;
			default:										 single = -1; break;
		} else single = 1;
	channels = (single < 0) ? 2 : 1;
	for (gr = 0; gr < granules; gr++) {
		struct tMPAGRInfo *grinfo = &mpadec->sideinfo.ch[0].gr[gr];
		int32_t part2bits = mpadec_III_get_scale_factors(mpadec, grinfo, scalefacs[0]);
		if (mpadec_III_decode_samples(mpadec, grinfo, mpadec->hybrid_in[0], scalefacs[0], part2bits)) {
			unsigned size = mpadec->frame.decoded_size;
			if (!mpadec->frame.LSF && gr) size >>= 1;
			memset(buffer, 0, size);
			mpadec->error = 1;
			goto done;
		}
		if (mpadec->frame.channels > 1) {
			grinfo = &mpadec->sideinfo.ch[1].gr[gr];
			part2bits = mpadec_III_get_scale_factors(mpadec, grinfo, scalefacs[1]);
			if (mpadec_III_decode_samples(mpadec, grinfo, mpadec->hybrid_in[1], scalefacs[1], part2bits)) {
				unsigned size = mpadec->frame.decoded_size;
				if (!mpadec->frame.LSF && gr) size >>= 1;
				memset(buffer, 0, size);
				mpadec->error = 1;
				goto done;
			}
			if (ms_stereo) {
				double *in0 = (double *)(mpadec->hybrid_in[0]), *in1 = (double *)(mpadec->hybrid_in[1]);
				unsigned i, maxb = mpadec->sideinfo.ch[0].gr[gr].maxb;
				if (mpadec->sideinfo.ch[1].gr[gr].maxb > maxb) maxb = mpadec->sideinfo.ch[1].gr[gr].maxb;
				for (i = 0; i < MPADEC_SSLIMIT*maxb; i++) {
					double tmp0 = in0[i];
					double tmp1 = in1[i];
					in0[i] = tmp0 + tmp1;
					in1[i] = tmp0 - tmp1;
				}
			}
			if (i_stereo) mpadec_III_i_stereo(mpadec, grinfo, mpadec->hybrid_in, scalefacs[1]);
			if (i_stereo || ms_stereo || !single) {
				if (grinfo->maxb > mpadec->sideinfo.ch[0].gr[gr].maxb) mpadec->sideinfo.ch[0].gr[gr].maxb = grinfo->maxb;
				else grinfo->maxb = mpadec->sideinfo.ch[0].gr[gr].maxb;
			}
			if (!single) {
				unsigned i;
				double *in0 = (double *)(mpadec->hybrid_in[0]), *in1 = (double *)(mpadec->hybrid_in[1]);
				for (i = 0; i < MPADEC_SSLIMIT*grinfo->maxb; i++, in0++) *in0 = (*in0 + *in1++);
			} else if (single == 2) {
				unsigned i;
				double *in0 = (double *)(mpadec->hybrid_in[0]), *in1 = (double *)(mpadec->hybrid_in[1]);
				for (i = 0; i < MPADEC_SSLIMIT*grinfo->maxb; i++, in0++) *in0 = *in1++;
			}
		}
		for (ch = 0; ch < channels; ch++) {
			grinfo = &mpadec->sideinfo.ch[ch].gr[gr];
			mpadec_III_antialias(grinfo, mpadec->hybrid_in[ch]);
			mpadec_III_hybrid(mpadec, grinfo, mpadec->hybrid_in[ch], mpadec->hybrid_out[ch], ch);
		}
		if (single < 0) {
			for (ss = 0; ss < MPADEC_SSLIMIT; ss++, buffer += mpadec->synth_size) {
				mpadec->synth_func(mpadec, mpadec->hybrid_out[0][ss], 0, buffer);
				mpadec->synth_func(mpadec, mpadec->hybrid_out[1][ss], 1, buffer);
			}
		} else {
			for (ss = 0; ss < MPADEC_SSLIMIT; ss++, buffer += mpadec->synth_size) {
				mpadec->synth_func(mpadec, mpadec->hybrid_out[0][ss], 0, buffer);
			}
		}
	}
	done:
	{
		unsigned n = mpadec->bits_left >> 3;
		mpadec->next_byte -= n;
		mpadec->bytes_left += n;
		if (saved_next_byte) {
			uint32_t tmp = mpadec->frame.frame_size - mpadec->hsize - mpadec->ssize;
			if (mpadec->bytes_left) {
				if (mpadec->bytes_left > 512) {
					mpadec->next_byte += mpadec->bytes_left - 512;
					mpadec->bytes_left = 512;
				}
				memmove(mpadec->reservoir, mpadec->next_byte, mpadec->bytes_left);
				mpadec->reservoir_size = mpadec->bytes_left;
			} else mpadec->reservoir_size = 0;
			mpadec->next_byte = saved_next_byte + tmp;
			mpadec->bytes_left = saved_bytes_left - tmp;
		} else {
			uint32_t tmp = mpadec->frame.frame_size - mpadec->hsize - mpadec->ssize;
			mpadec->reservoir_size = 0;
			if (tmp > (saved_bytes_left - mpadec->bytes_left)) {
				tmp -= saved_bytes_left - mpadec->bytes_left;
				if (tmp > 512) {
					mpadec->next_byte += tmp - 512;
					mpadec->bytes_left -= tmp - 512;
					tmp = 512;
				}
				memcpy(mpadec->reservoir, mpadec->next_byte, tmp);
				mpadec->reservoir_size = tmp;
				mpadec->next_byte += tmp;
				mpadec->bytes_left -= tmp;
			}
		}
	}
}

#ifdef __cplusplus
}
#endif
