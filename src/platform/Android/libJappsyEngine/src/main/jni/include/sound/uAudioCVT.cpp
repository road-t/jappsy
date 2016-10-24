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

#include "uAudioCVT.h"

/*
// Вычисление максимальной громкости и последующая его нормализация
void normVol(struct tAudioConvert *cvt, uint16_t format) {
	int i; long sample; long maxvol;
	switch (format&0x8018) {
		case AUDIO_U8: {
				unsigned char *src, *dst;
				src = cvt->buf;
				maxvol = 0;
				for ( i=cvt->len_cvt; i; --i ) {
					sample = src[0]; sample -= 0x80;
					if (sample < 0) sample = -sample;
					if (maxvol < sample) maxvol = sample;
					src += 1;
				}
				src = cvt->buf; dst = cvt->buf;
				double volmul = (128.0f * cvt->volume) / (double(maxvol)+1.0f);
				for ( i=cvt->len_cvt; i; --i ) {
					sample = src[0];
					sample -= 0x80; sample = (long)(double(sample) * volmul); sample += 0x80;
					dst[0] = sample;
					src += 1; dst += 1;
				}
			}
			break;
		case AUDIO_S8: {
				char *src, *dst;
				src = (char*)cvt->buf;
				maxvol = 0;
				for ( i=cvt->len_cvt/2; i; --i ) {
					sample = src[0];
					if (sample < 0) sample = -sample;
					if (maxvol < sample) maxvol = sample;
					src += 1;
				}
				src = (char*)cvt->buf; dst = (char*)cvt->buf;
				double volmul = (128.0f * cvt->volume) / (double(maxvol)+1.0f);
				for ( i=cvt->len_cvt; i; --i ) {
					sample = src[0];
					sample = (long)(double(sample) * volmul);
					dst[0] = sample;
					src += 1; dst += 1;
				}
			}
			break;
		case AUDIO_U16: {
				unsigned char *src, *dst;
				src = cvt->buf;
				maxvol = 0;
				if ( (format & 0x1000) == 0x1000 ) {
					for ( i=cvt->len_cvt/2; i; --i ) {
						sample = (unsigned short int)((src[0]<<8)|src[1]); sample -= 0x8000;
						if (sample < 0) sample = -sample;
						if (maxvol < sample) maxvol = sample;
						src += 2;
					}
				} else {
					for ( i=cvt->len_cvt/2; i; --i ) {
						sample = (unsigned short int)((src[1]<<8)|src[0]); sample -= 0x8000;
						if (sample < 0) sample = -sample;
						if (maxvol < sample) maxvol = sample;
						src += 2;
					}
				}
				src = cvt->buf; dst = cvt->buf;
				double volmul = (32768.0f * cvt->volume) / (double(maxvol)+1.0f);
				if ( (format & 0x1000) == 0x1000 ) {
					for ( i=cvt->len_cvt/2; i; --i ) {
						sample = (unsigned short int)((src[0]<<8)|src[1]);
						sample -= 0x8000; sample = (long)(double(sample) * volmul); sample += 0x8000;
						dst[1] = (sample&0xFF);
						sample >>= 8; dst[0] = (sample&0xFF);
						src += 2; dst += 2;
					}
				} else {
					for ( i=cvt->len_cvt/2; i; --i ) {
						sample = (unsigned short int)((src[1]<<8)|src[0]);
						sample -= 0x8000; sample = (long)(double(sample) * volmul); sample += 0x8000;
						dst[1] = (sample&0xFF);
						sample >>= 8; dst[0] = (sample&0xFF);
						src += 2; dst += 2;
					}
				}
			}
			break;
		case AUDIO_S16: {
				unsigned char *src, *dst;
				src = cvt->buf;
				maxvol = 0;
				if ( (format & 0x1000) == 0x1000 ) {
					for ( i=cvt->len_cvt/2; i; --i ) {
						sample = (short int)((src[0]<<8)|src[1]);
						if (sample < 0) sample = -sample;
						if (maxvol < sample) maxvol = sample;
						src += 2;
					}
				} else {
					for ( i=cvt->len_cvt/2; i; --i ) {
						sample = (short int)((src[1]<<8)|src[0]);
						if (sample < 0) sample = -sample;
						if (maxvol < sample) maxvol = sample;
						src += 2;
					}
				}
				src = cvt->buf;	dst = cvt->buf;
				double volmul = (32768.0f * cvt->volume) / (double(maxvol)+1.0f);
				if ( (format & 0x1000) == 0x1000 ) {
					for ( i=cvt->len_cvt/2; i; --i ) {
						sample = (short int)((src[0]<<8)|src[1]);
						sample = (long)(double(sample) * volmul);
						dst[1] = (sample&0xFF);
						sample >>= 8; dst[0] = (sample&0xFF);
						src += 2; dst += 2;
					}
				} else {
					for ( i=cvt->len_cvt/2; i; --i ) {
						sample = (short int)((src[1]<<8)|src[0]);
						sample = (long)(double(sample) * volmul);
						dst[1] = (sample&0xFF);
						sample >>= 8; dst[0] = (sample&0xFF);
						src += 2; dst += 2;
					}
				}
			}
			break;
	}
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format);
	}
}
*/

// Эффективное сложение правого и левого каналов в один
void audioConvertMono(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; int32_t sample;

	switch (format&0x8018) {
		case AUDIO_U8: {
			uint8_t *src, *dst;
			src = cvt->buf; dst = cvt->buf;
			for ( i=cvt->len_cvt/2; i; --i ) {
				sample = (int32_t)((uint32_t)src[0] + (uint32_t)src[1]);
				*dst = (uint8_t)(sample / 2);
				src += 2; dst += 1;
			}
			break;
		}

		case AUDIO_S8: {
			int8_t *src, *dst;
			src = (int8_t*)cvt->buf; dst = (int8_t*)cvt->buf;
			for ( i=cvt->len_cvt/2; i; --i ) {
				sample = (int32_t)src[0] + (int32_t)src[1];
				*dst = (int8_t)(sample / 2);
				src += 2; dst += 1;
			}
			break;
		}

		case AUDIO_U16: {
			uint8_t *src, *dst;
			src = cvt->buf; dst = cvt->buf;
			if ( (format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len_cvt/4; i; --i ) {
					sample = (int32_t)(uint32_t)(((uint16_t)src[0]<<8)|(uint16_t)src[1])
							 + (int32_t)(uint32_t)(((uint16_t)src[2]<<8)|(uint16_t)src[3]);
					sample /= 2; dst[1] = (uint8_t)(sample&0xFF);
					sample >>= 8; dst[0] = (uint8_t)(sample&0xFF);
					src += 4; dst += 2;
				}
			} else {
				for ( i=cvt->len_cvt/4; i; --i ) {
					sample = (int32_t)(uint32_t)(((uint16_t)src[1]<<8)|(uint16_t)src[0])
							 + (int32_t)(uint32_t)(((uint16_t)src[3]<<8)|(uint16_t)src[2]);
					sample /= 2; dst[0] = (uint8_t)(sample&0xFF);
					sample >>= 8; dst[1] = (uint8_t)(sample&0xFF);
					src += 4; dst += 2;
				}
			}
			break;
		}

		case AUDIO_S16: {
			uint8_t *src, *dst;
			src = cvt->buf;	dst = cvt->buf;
			if ( (format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len_cvt/4; i; --i ) {
					sample = (int32_t)(int16_t)(((uint16_t)src[0]<<8)|(uint16_t)src[1])
							 + (int32_t)(int16_t)(((uint16_t)src[2]<<8)|(uint16_t)src[3]);
					sample /= 2; dst[1] = (uint8_t)(sample&0xFF);
					sample >>= 8; dst[0] = (uint8_t)(sample&0xFF);
					src += 4; dst += 2;
				}
			} else {
				for ( i=cvt->len_cvt/4; i; --i ) {
					sample = (int32_t)(int16_t)(((uint16_t)src[1]<<8)|(uint16_t)src[0])
							 + (int32_t)(int16_t)(((uint16_t)src[3]<<8)|(uint16_t)src[2]);
					sample /= 2; dst[0] = (uint8_t)(sample&0xFF);
					sample >>= 8; dst[1] = (uint8_t)(sample&0xFF);
					src += 4; dst += 2;
				}
			}
			break;
		}

		default:;
	}

	cvt->len_cvt /= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 1);
	}
}

// Обрезать 4 канала из 6, кроме передних левого и правого
void audioConvertStrip(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; int32_t lsample, rsample;

	switch (format&0x8018) {
		case AUDIO_U8: {
			uint8_t *src, *dst;
			src = cvt->buf; dst = cvt->buf;
			for ( i=cvt->len_cvt/6; i; --i ) {
				dst[0] = src[0]; dst[1] = src[1];
				src += 6; dst += 2;
			}
			break;
		}

		case AUDIO_S8: {
			int8_t *src, *dst;
			src = (int8_t*)cvt->buf; dst = (int8_t*)cvt->buf;
			for ( i=cvt->len_cvt/6; i; --i ) {
				dst[0] = src[0]; dst[1] = src[1];
				src += 6; dst += 2;
			}
			break;
		}

		case AUDIO_U16: {
			uint8_t *src, *dst;
			src = cvt->buf; dst = cvt->buf;
			if ( (format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len_cvt/12; i; --i ) {
					lsample = (int32_t)(uint32_t)(((uint16_t)src[0]<<8)|(uint16_t)src[1]);
					rsample = (int32_t)(uint32_t)(((uint16_t)src[2]<<8)|(uint16_t)src[3]);
					dst[1] = (uint8_t)(lsample&0xFF); lsample >>= 8;
					dst[0] = (uint8_t)(lsample&0xFF); dst[3] = (uint8_t)(rsample&0xFF);
					rsample >>= 8; dst[2] = (uint8_t)(rsample&0xFF);
					src += 12; dst += 4;
				}
			} else {
				for ( i=cvt->len_cvt/12; i; --i ) {
					lsample = (int32_t)(uint32_t)(((uint16_t)src[1]<<8)|(uint16_t)src[0]);
					rsample = (int32_t)(uint32_t)(((uint16_t)src[3]<<8)|(uint16_t)src[2]);
					dst[0] = (uint8_t)(lsample&0xFF); lsample >>= 8;
					dst[1] = (uint8_t)(lsample&0xFF); dst[2] = (uint8_t)(rsample&0xFF);
					rsample >>= 8; dst[3] = (uint8_t)(rsample&0xFF);
					src += 12; dst += 4;
				}
			}
			break;
		}

		case AUDIO_S16: {
			unsigned char *src, *dst;
			src = cvt->buf; dst = cvt->buf;
			if ( (format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len_cvt/12; i; --i ) {
					lsample = (int32_t)(int16_t)(((uint16_t)src[0]<<8)|(uint16_t)src[1]);
					rsample = (int32_t)(int16_t)(((uint16_t)src[2]<<8)|(uint16_t)src[3]);
					dst[1] = (uint8_t)(lsample&0xFF); lsample >>= 8;
					dst[0] = (uint8_t)(lsample&0xFF); dst[3] = (uint8_t)(rsample&0xFF);
					rsample >>= 8; dst[2] = (uint8_t)(rsample&0xFF);
					src += 12; dst += 4;
				}
			} else {
				for ( i=cvt->len_cvt/12; i; --i ) {
					lsample = (int32_t)(int16_t)(((uint16_t)src[1]<<8)|(uint16_t)src[0]);
					rsample = (int32_t)(int16_t)(((uint16_t)src[3]<<8)|(uint16_t)src[2]);
					dst[0] = (uint8_t)(lsample&0xFF); lsample >>= 8;
					dst[1] = (uint8_t)(lsample&0xFF); dst[2] = (uint8_t)(rsample&0xFF);
					rsample >>= 8; dst[3] = (uint8_t)(rsample&0xFF);
					src += 12; dst += 4;
				}
			}
			break;
		}

		default:;
	}

	cvt->len_cvt /= 3;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 2);
	}
}

//  Обрезать 2 канала из 6
void audioConvertStrip_2(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; int32_t lsample, rsample;

	switch (format&0x8018) {
		case AUDIO_U8: {
			uint8_t *src, *dst;
			src = cvt->buf; dst = cvt->buf;
			for ( i=cvt->len_cvt/4; i; --i ) {
				dst[0] = src[0]; dst[1] = src[1];
				src += 4; dst += 2;
			}
			break;
		}

		case AUDIO_S8: {
			int8_t *src, *dst;
			src = (int8_t*)cvt->buf; dst = (int8_t*)cvt->buf;
			for ( i=cvt->len_cvt/4; i; --i ) {
				dst[0] = src[0]; dst[1] = src[1];
				src += 4; dst += 2;
			}
			break;
		}

		case AUDIO_U16: {
			uint8_t *src, *dst;
			src = cvt->buf; dst = cvt->buf;
			if ( (format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len_cvt/8; i; --i ) {
					lsample = (int32_t)(uint32_t)(((uint16_t)src[0]<<8)|(uint16_t)src[1]);
					rsample = (int32_t)(uint32_t)(((uint16_t)src[2]<<8)|(uint16_t)src[3]);
					dst[1] = (uint8_t)(lsample&0xFF); lsample >>= 8;
					dst[0] = (uint8_t)(lsample&0xFF); dst[3] = (uint8_t)(rsample&0xFF);
					rsample >>= 8; dst[2] = (uint8_t)(rsample&0xFF);
					src += 8; dst += 4;
				}
			} else {
				for ( i=cvt->len_cvt/8; i; --i ) {
					lsample = (int32_t)(uint32_t)(((uint16_t)src[1]<<8)|(uint16_t)src[0]);
					rsample = (int32_t)(uint32_t)(((uint16_t)src[3]<<8)|(uint16_t)src[2]);
					dst[0] = (uint8_t)(lsample&0xFF); lsample >>= 8;
					dst[1] = (uint8_t)(lsample&0xFF); dst[2] = (uint8_t)(rsample&0xFF);
					rsample >>= 8; dst[3] = (uint8_t)(rsample&0xFF);
					src += 8; dst += 4;
				}
			}
			break;
		}

		case AUDIO_S16: {
			uint8_t *src, *dst;
			src = cvt->buf; dst = cvt->buf;
			if ( (format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len_cvt/8; i; --i ) {
					lsample = (int32_t)(int16_t)(((uint16_t)src[0]<<8)|(uint16_t)src[1]);
					rsample = (int32_t)(int16_t)(((uint16_t)src[2]<<8)|(uint16_t)src[3]);
					dst[1] = (uint8_t)(lsample&0xFF); lsample >>= 8;
					dst[0] = (uint8_t)(lsample&0xFF); dst[3] = (uint8_t)(rsample&0xFF);
					rsample >>= 8; dst[2] = (uint8_t)(rsample&0xFF);
					src += 8; dst += 4;
				}
			} else {
				for ( i=cvt->len_cvt/8; i; --i ) {
					lsample = (int32_t)(int16_t)(((uint16_t)src[1]<<8)|(uint16_t)src[0]);
					rsample = (int32_t)(int16_t)(((uint16_t)src[3]<<8)|(uint16_t)src[2]);
					dst[0] = (uint8_t)(lsample&0xFF); lsample >>= 8;
					dst[1] = (uint8_t)(lsample&0xFF); dst[2] = (uint8_t)(rsample&0xFF);
					rsample >>= 8; dst[3] = (uint8_t)(rsample&0xFF);
					src += 8; dst += 4;
				}
			}
			break;
		}

		default:;
	}

	cvt->len_cvt /= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 4);
	}
}

// дублировать моно канал на левый и правый
void audioConvertStereo(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i;

	if ( (format & 0xFF) == 16 ) {
		uint16_t *src, *dst;
		src = (uint16_t*)(cvt->buf+cvt->len_cvt);
		dst = (uint16_t*)(cvt->buf+cvt->len_cvt*2);
		for ( i=cvt->len_cvt/2; i; --i ) {
			dst -= 2; src -= 1;
			dst[0] = src[0]; dst[1] = src[0];
		}
	} else {
		uint8_t *src, *dst;
		src = cvt->buf+cvt->len_cvt;
		dst = cvt->buf+cvt->len_cvt*2;
		for ( i=cvt->len_cvt; i; --i ) {
			dst -= 2; src -= 1;
			dst[0] = src[0]; dst[1] = src[0];
		}
	}

	cvt->len_cvt *= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 2);
	}
}

// дублировать стерео в псевдостерео-5.1
void audioConvertSurround(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i;

	switch (format&0x8018) {
		case AUDIO_U8: {
			uint8_t *src, *dst, lf, rf, ce;
			src = (cvt->buf+cvt->len_cvt);
			dst = (cvt->buf+cvt->len_cvt*3);
			for ( i=cvt->len_cvt / 2; i; --i ) {
				dst -= 6; src -= 2;
				lf = src[0]; rf = src[1];
				ce = (uint8_t)((lf/2) + (rf/2));
				dst[0] = lf; dst[1] = rf;
				dst[2] = lf - ce; dst[3] = rf - ce;
				dst[4] = ce; dst[5] = ce;
			}
			break;
		}

		case AUDIO_S8: {
			int8_t *src, *dst, lf, rf, ce;
			src = (int8_t*)cvt->buf+cvt->len_cvt;
			dst = (int8_t*)cvt->buf+cvt->len_cvt*3;
			for ( i=cvt->len_cvt / 2; i; --i ) {
				dst -= 6; src -= 2;
				lf = src[0]; rf = src[1];
				ce = (int8_t)((lf/2) + (rf/2));
				dst[0] = lf; dst[1] = rf;
				dst[2] = lf - ce; dst[3] = rf - ce;
				dst[4] = ce; dst[5] = ce;
			}
			break;
		}

		case AUDIO_U16: {
			uint8_t *src, *dst;
			uint16_t lf, rf, ce, lr, rr;
			src = cvt->buf+cvt->len_cvt;
			dst = cvt->buf+cvt->len_cvt*3;
			if ( (format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len_cvt/4; i; --i ) {
					dst -= 12; src -= 4;
					lf = (uint16_t)(((uint16_t)src[0]<<8)|(uint16_t)src[1]);
					rf = (uint16_t)(((uint16_t)src[2]<<8)|(uint16_t)src[3]);
					ce = (uint16_t)((lf/2) + (rf/2));
					rr = lf - ce; lr = rf - ce;
					dst[1] = (uint8_t)(lf&0xFF); dst[0] = (uint8_t)((lf>>8)&0xFF);
					dst[3] = (uint8_t)(rf&0xFF); dst[2] = (uint8_t)((rf>>8)&0xFF);
					dst[1+4] = (uint8_t)(lr&0xFF); dst[0+4] = (uint8_t)((lr>>8)&0xFF);
					dst[3+4] = (uint8_t)(rr&0xFF); dst[2+4] = (uint8_t)((rr>>8)&0xFF);
					dst[1+8] = (uint8_t)(ce&0xFF); dst[0+8] = (uint8_t)((ce>>8)&0xFF);
					dst[3+8] = (uint8_t)(ce&0xFF); dst[2+8] = (uint8_t)((ce>>8)&0xFF);
				}
			} else {
				for ( i=cvt->len_cvt/4; i; --i ) {
					dst -= 12; src -= 4;
					lf = (uint16_t)(((uint16_t)src[1]<<8)|(uint16_t)src[0]);
					rf = (uint16_t)(((uint16_t)src[3]<<8)|(uint16_t)src[2]);
					ce = (uint16_t)((lf/2) + (rf/2));
					rr = lf - ce; lr = rf - ce;
					dst[0] = (uint8_t)(lf&0xFF); dst[1] = (uint8_t)((lf>>8)&0xFF);
					dst[2] = (uint8_t)(rf&0xFF); dst[3] = (uint8_t)((rf>>8)&0xFF);
					dst[0+4] = (uint8_t)(lr&0xFF); dst[1+4] = (uint8_t)((lr>>8)&0xFF);
					dst[2+4] = (uint8_t)(rr&0xFF); dst[3+4] = (uint8_t)((rr>>8)&0xFF);
					dst[0+8] = (uint8_t)(ce&0xFF); dst[1+8] = (uint8_t)((ce>>8)&0xFF);
					dst[2+8] = (uint8_t)(ce&0xFF); dst[3+8] = (uint8_t)((ce>>8)&0xFF);
				}
			}
			break;
		}

		case AUDIO_S16: {
			uint8_t *src, *dst;
			int16_t lf, rf, ce, lr, rr;
			src = cvt->buf+cvt->len_cvt;
			dst = cvt->buf+cvt->len_cvt*3;
			if ( (format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len_cvt/4; i; --i ) {
					dst -= 12; src -= 4;
					lf = (int16_t)(((uint16_t)src[0]<<8)|(uint16_t)src[1]);
					rf = (int16_t)(((uint16_t)src[2]<<8)|(uint16_t)src[3]);
					ce = (int16_t)((lf/2) + (rf/2));
					rr = lf - ce; lr = rf - ce;
					dst[1] = (uint8_t)(lf&0xFF); dst[0] = (uint8_t)((lf>>8)&0xFF);
					dst[3] = (uint8_t)(rf&0xFF); dst[2] = (uint8_t)((rf>>8)&0xFF);
					dst[1+4] = (uint8_t)(lr&0xFF); dst[0+4] = (uint8_t)((lr>>8)&0xFF);
					dst[3+4] = (uint8_t)(rr&0xFF); dst[2+4] = (uint8_t)((rr>>8)&0xFF);
					dst[1+8] = (uint8_t)(ce&0xFF); dst[0+8] = (uint8_t)((ce>>8)&0xFF);
					dst[3+8] = (uint8_t)(ce&0xFF); dst[2+8] = (uint8_t)((ce>>8)&0xFF);
				}
			} else {
				for ( i=cvt->len_cvt/4; i; --i ) {
					dst -= 12; src -= 4;
					lf = (int16_t)(((uint16_t)src[1]<<8)|(uint16_t)src[0]);
					rf = (int16_t)(((uint16_t)src[3]<<8)|(uint16_t)src[2]);
					ce = (int16_t)((lf/2) + (rf/2));
					rr = lf - ce; lr = rf - ce;
					dst[0] = (uint8_t)(lf&0xFF); dst[1] = (uint8_t)((lf>>8)&0xFF);
					dst[2] = (uint8_t)(rf&0xFF); dst[3] = (uint8_t)((rf>>8)&0xFF);
					dst[0+4] = (uint8_t)(lr&0xFF); dst[1+4] = (uint8_t)((lr>>8)&0xFF);
					dst[2+4] = (uint8_t)(rr&0xFF); dst[3+4] = (uint8_t)((rr>>8)&0xFF);
					dst[0+8] = (uint8_t)(ce&0xFF); dst[1+8] = (uint8_t)((ce>>8)&0xFF);
					dst[2+8] = (uint8_t)(ce&0xFF); dst[3+8] = (uint8_t)((ce>>8)&0xFF);
				}
			}
			break;
		}

		default:;
	}

	cvt->len_cvt *= 3;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 6);
	}
}


// дублировать стерео в псевдостерео-4.0
void audioConvertSurround_4(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i;

	switch (format&0x8018) {
		case AUDIO_U8: {
			uint8_t *src, *dst, lf, rf, ce;
			src = (cvt->buf+cvt->len_cvt);
			dst = (cvt->buf+cvt->len_cvt*2);
			for ( i=cvt->len_cvt/2; i; --i ) {
				dst -= 4; src -= 2;
				lf = src[0]; rf = src[1];
				ce = (uint8_t)((lf/2) + (rf/2));
				dst[0] = lf; dst[1] = rf;
				dst[2] = lf - ce; dst[3] = rf - ce;
			}
			break;
		}

		case AUDIO_S8: {
			int8_t *src, *dst, lf, rf, ce;
			src = (int8_t*)cvt->buf+cvt->len_cvt;
			dst = (int8_t*)cvt->buf+cvt->len_cvt*2;
			for ( i=cvt->len_cvt/2; i; --i ) {
				dst -= 4; src -= 2;
				lf = src[0]; rf = src[1];
				ce = (int8_t)((lf/2) + (rf/2));
				dst[0] = lf; dst[1] = rf;
				dst[2] = lf - ce; dst[3] = rf - ce;
			}
			break;
		}

		case AUDIO_U16: {
			uint8_t *src, *dst;
			uint16_t lf, rf, ce, lr, rr;
			src = cvt->buf+cvt->len_cvt;
			dst = cvt->buf+cvt->len_cvt*2;
			if ( (format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len_cvt/4; i; --i ) {
					dst -= 8; src -= 4;
					lf = (uint16_t)(((uint16_t)src[0]<<8)|(uint16_t)src[1]);
					rf = (uint16_t)(((uint16_t)src[2]<<8)|(uint16_t)src[3]);
					ce = (uint16_t)((lf/2) + (rf/2));
					rr = lf - ce; lr = rf - ce;
					dst[1] = (uint8_t)(lf&0xFF); dst[0] = (uint8_t)((lf>>8)&0xFF);
					dst[3] = (uint8_t)(rf&0xFF); dst[2] = (uint8_t)((rf>>8)&0xFF);
					dst[1+4] = (uint8_t)(lr&0xFF); dst[0+4] = (uint8_t)((lr>>8)&0xFF);
					dst[3+4] = (uint8_t)(rr&0xFF); dst[2+4] = (uint8_t)((rr>>8)&0xFF);
				}
			} else {
				for ( i=cvt->len_cvt/4; i; --i ) {
					dst -= 8; src -= 4;
					lf = (uint16_t)(((uint16_t)src[1]<<8)|(uint16_t)src[0]);
					rf = (uint16_t)(((uint16_t)src[3]<<8)|(uint16_t)src[2]);
					ce = (uint16_t)((lf/2) + (rf/2));
					rr = lf - ce; lr = rf - ce;
					dst[0] = (uint8_t)(lf&0xFF); dst[1] = (uint8_t)((lf>>8)&0xFF);
					dst[2] = (uint8_t)(rf&0xFF); dst[3] = (uint8_t)((rf>>8)&0xFF);
					dst[0+4] = (uint8_t)(lr&0xFF); dst[1+4] = (uint8_t)((lr>>8)&0xFF);
					dst[2+4] = (uint8_t)(rr&0xFF); dst[3+4] = (uint8_t)((rr>>8)&0xFF);
				}
			}
			break;
		}

		case AUDIO_S16: {
			uint8_t *src, *dst;
			int16_t lf, rf, ce, lr, rr;
			src = cvt->buf+cvt->len_cvt;
			dst = cvt->buf+cvt->len_cvt*2;
			if ( (format & 0x1000) == 0x1000 ) {
				for ( i=cvt->len_cvt/4; i; --i ) {
					dst -= 8; src -= 4;
					lf = (int16_t)(uint16_t)(((uint16_t)src[0]<<8)|(uint16_t)src[1]);
					rf = (int16_t)(uint16_t)(((uint16_t)src[2]<<8)|(uint16_t)src[3]);
					ce = (int16_t)((lf/2) + (rf/2));
					rr = lf - ce; lr = rf - ce;
					dst[1] = (uint8_t)(lf&0xFF); dst[0] = (uint8_t)((lf>>8)&0xFF);
					dst[3] = (uint8_t)(rf&0xFF); dst[2] = (uint8_t)((rf>>8)&0xFF);
					dst[1+4] = (uint8_t)(lr&0xFF); dst[0+4] = (uint8_t)((lr>>8)&0xFF);
					dst[3+4] = (uint8_t)(rr&0xFF); dst[2+4] = (uint8_t)((rr>>8)&0xFF);
				}
			} else {
				for ( i=cvt->len_cvt/4; i; --i ) {
					dst -= 8; src -= 4;
					lf = (int16_t)(uint16_t)(((uint16_t)src[1]<<8)|(uint16_t)src[0]);
					rf = (int16_t)(uint16_t)(((uint16_t)src[3]<<8)|(uint16_t)src[2]);
					ce = (int16_t)((lf/2) + (rf/2));
					rr = lf - ce; lr = rf - ce;
					dst[0] = (uint8_t)(lf&0xFF); dst[1] = (uint8_t)((lf>>8)&0xFF);
					dst[2] = (uint8_t)(rf&0xFF); dst[3] = (uint8_t)((rf>>8)&0xFF);
					dst[0+4] = (uint8_t)(lr&0xFF); dst[1+4] = (uint8_t)((lr>>8)&0xFF);
					dst[2+4] = (uint8_t)(rr&0xFF); dst[3+4] = (uint8_t)((rr>>8)&0xFF);
				}
			}
			break;
		}

		default:;
	}

	cvt->len_cvt *= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 4);
	}
}

// Конвертировать 8-bit в 16-bit - LSB
void audioConvert16LSB(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; uint8_t *src, *dst;
	src = cvt->buf+cvt->len_cvt;
	dst = cvt->buf+cvt->len_cvt*2;
	for ( i=cvt->len_cvt; i; --i ) {
		src -= 1; dst -= 2; dst[1] = *src; dst[0] = 0;
	}
	format = (uint16_t)((format & ~0x0008) | AUDIO_U16LSB);
	cvt->len_cvt *= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, channels);
	}
}

// Конвертировать 8-bit в 16-bit - MSB
void audioConvert16MSB(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; uint8_t *src, *dst;
	src = cvt->buf+cvt->len_cvt;
	dst = cvt->buf+cvt->len_cvt*2;
	for ( i=cvt->len_cvt; i; --i ) {
		src -= 1; dst -= 2; dst[0] = *src; dst[1] = 0;
	}
	format = (uint16_t)((format & ~0x0008) | AUDIO_U16MSB);
	cvt->len_cvt *= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, channels);
	}
}

// Конвертировать 16-bit и 8-bit
void audioConvert8(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; uint8_t *src, *dst;
	src = cvt->buf; dst = cvt->buf;
	if ( (format & 0x1000) != 0x1000 ) ++src;
	for ( i=cvt->len_cvt/2; i; --i ) {
		*dst = *src; src += 2; dst += 1;
	}
	format = (uint16_t)((format & ~0x9010) | AUDIO_U8);
	cvt->len_cvt /= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, channels);
	}
}

// Изменить со знаком/без знака
void audioConvertSign(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; uint8_t *data;
	data = cvt->buf;
	if ( (format & 0xFF) == 16 ) {
		if ( (format & 0x1000) != 0x1000 ) ++data;
		for ( i=cvt->len_cvt/2; i; --i ) {
			*data ^= 0x80; data += 2;
		}
	} else {
		for ( i=cvt->len_cvt; i; --i ) {
			*data++ ^= 0x80;
		}
	}
	format = (uint16_t)(format ^ 0x8000);
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, channels);
	}
}

// Изменить порядок байт
void audioConvertEndian(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; uint8_t *data, tmp;
	data = cvt->buf;
	for ( i=cvt->len_cvt/2; i; --i ) {
		tmp = data[0]; data[0] = data[1]; data[1] = tmp; data += 2;
	}
	format = (uint16_t)(format ^ 0x1000);
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, channels);
	}
}

// Уменьшить скорость в 2 раза
void audioRateMUL2(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; uint8_t *src, *dst;
	src = cvt->buf+cvt->len_cvt;
	dst = cvt->buf+cvt->len_cvt*2;

	switch (format & 0xFF) {
		case 8:
			for ( i=cvt->len_cvt; i; --i ) {
				src -= 1; dst -= 2;
				dst[0] = src[0]; dst[1] = src[0];
			}
			break;

		case 16:
			for ( i=cvt->len_cvt/2; i; --i ) {
				src -= 2; dst -= 4;
				dst[0] = src[0]; dst[1] = src[1];
				dst[2] = src[0]; dst[3] = src[1];
			}
			break;

		default:;
	}

	cvt->len_cvt *= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 1);
	}
}

// Уменьшить скорость в 2 раза (стерео)
void audioRateMUL2_c2(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; uint8_t *src, *dst;
	src = cvt->buf+cvt->len_cvt;
	dst = cvt->buf+cvt->len_cvt*2;

	switch (format & 0xFF) {
		case 8:
			for ( i=cvt->len_cvt/2; i; --i ) {
				src -= 2; dst -= 4;
				dst[0] = src[0]; dst[1] = src[1];
				dst[2] = src[0]; dst[3] = src[1];
			}
			break;

		case 16:
			for ( i=cvt->len_cvt/4; i; --i ) {
				src -= 4; dst -= 8;
				dst[0] = src[0]; dst[1] = src[1];
				dst[2] = src[2]; dst[3] = src[3];
				dst[4] = src[0]; dst[5] = src[1];
				dst[6] = src[2]; dst[7] = src[3];
			}
			break;

		default:;
	}

	cvt->len_cvt *= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 2);
	}
}

// Уменьшить скорость в 2 раза (квадро)
void audioRateMUL2_c4(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; uint8_t *src, *dst;
	src = cvt->buf+cvt->len_cvt;
	dst = cvt->buf+cvt->len_cvt*2;

	switch (format & 0xFF) {
		case 8:
			for ( i=cvt->len_cvt/4; i; --i ) {
				src -= 4; dst -= 8;
				dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
				dst[4] = src[0]; dst[5] = src[1]; dst[6] = src[2]; dst[7] = src[3];
			}
			break;

		case 16:
			for ( i=cvt->len_cvt/8; i; --i ) {
				src -= 8; dst -= 16;
				dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
				dst[4] = src[4]; dst[5] = src[5]; dst[6] = src[6]; dst[7] = src[7];
				dst[8] = src[0]; dst[9] = src[1]; dst[10] = src[2]; dst[11] = src[3];
				dst[12] = src[4]; dst[13] = src[5]; dst[14] = src[6]; dst[15] = src[7];
			}
			break;

		default:;
	}

	cvt->len_cvt *= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 4);
	}
}

// Уменьшить скорость в 2 раза (5.1)
void audioRateMUL2_c6(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; uint8_t *src, *dst;
	src = cvt->buf+cvt->len_cvt;
	dst = cvt->buf+cvt->len_cvt*2;

	switch (format & 0xFF) {
		case 8:
			for ( i=cvt->len_cvt/6; i; --i ) {
				src -= 6; dst -= 12;
				dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; dst[4] = src[4]; dst[5] = src[5];
				dst[6] = src[0]; dst[7] = src[1]; dst[8] = src[2]; dst[9] = src[3]; dst[10] = src[4]; dst[11] = src[5];
			}
			break;

		case 16:
			for ( i=cvt->len_cvt/12; i; --i ) {
				src -= 12; dst -= 24;
				dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; dst[4] = src[4]; dst[5] = src[5];
				dst[6] = src[6]; dst[7] = src[7]; dst[8] = src[8]; dst[9] = src[9]; dst[10] = src[10]; dst[11] = src[11];
				dst[12] = src[0]; dst[13] = src[1]; dst[14] = src[2]; dst[15] = src[3]; dst[16] = src[4]; dst[17] = src[5];
				dst[18] = src[6]; dst[19] = src[7]; dst[20] = src[8]; dst[21] = src[9]; dst[22] = src[10]; dst[23] = src[11];
			}
			break;

		default:;
	}

	cvt->len_cvt *= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 6);
	}
}

// Увеличить скорость в 2 раза
void audioRateDIV2(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; uint8_t *src, *dst;
	src = cvt->buf; dst = cvt->buf;

	switch (format & 0xFF) {
		case 8:
			for ( i=cvt->len_cvt/2; i; --i ) {
				dst[0] = src[0]; src += 2; dst += 1;
			}
			break;

		case 16:
			for ( i=cvt->len_cvt/4; i; --i ) {
				dst[0] = src[0]; dst[1] = src[1];
				src += 4; dst += 2;
			}
			break;

		default:;
	}

	cvt->len_cvt /= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 1);
	}
}

// Увеличить скорость в 2 раза (стерео)
void audioRateDIV2_c2(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; uint8_t *src, *dst;
	src = cvt->buf; dst = cvt->buf;

	switch (format & 0xFF) {
		case 8:
			for ( i=cvt->len_cvt/4; i; --i ) {
				dst[0] = src[0]; dst[1] = src[1];
				src += 4; dst += 2;
			}
			break;

		case 16:
			for ( i=cvt->len_cvt/8; i; --i ) {
				dst[0] = src[0]; dst[1] = src[1];
				dst[2] = src[2]; dst[3] = src[3];
				src += 8; dst += 4;
			}
			break;

		default:;
	}

	cvt->len_cvt /= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 2);
	}
}

// Увеличить скорость в 2 раза (квадро)
void audioRateDIV2_c4(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; uint8_t *src, *dst;
	src = cvt->buf; dst = cvt->buf;

	switch (format & 0xFF) {
		case 8:
			for ( i=cvt->len_cvt/8; i; --i ) {
				dst[0] = src[0]; dst[1] = src[1];
				dst[2] = src[2]; dst[3] = src[3];
				src += 8; dst += 4;
			}
			break;

		case 16:
			for ( i=cvt->len_cvt/16; i; --i ) {
				dst[0] = src[0]; dst[1] = src[1];
				dst[2] = src[2]; dst[3] = src[3];
				dst[4] = src[4]; dst[5] = src[5];
				dst[6] = src[6]; dst[7] = src[7];
				src += 16; dst += 8;
			}
			break;

		default:;
	}

	cvt->len_cvt /= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 4);
	}
}

// Увеличить скорость в 2 раза (5.1)
void audioRateDIV2_c6(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	int i; uint8_t *src, *dst;
	src = cvt->buf; dst = cvt->buf;

	switch (format & 0xFF) {
		case 8:
			for ( i=cvt->len_cvt/12; i; --i ) {
				dst[0] = src[0]; dst[1] = src[1];
				dst[2] = src[2]; dst[3] = src[3];
				dst[4] = src[4]; dst[5] = src[5];
				src += 12; dst += 6;
			}
			break;

		case 16:
			for ( i=cvt->len_cvt/24; i; --i ) {
				dst[0] = src[0]; dst[1] = src[1];
				dst[2] = src[2]; dst[3] = src[3];
				dst[4] = src[4]; dst[5] = src[5];
				dst[6] = src[6]; dst[7] = src[7];
				dst[8] = src[8]; dst[9] = src[9];
				dst[10] = src[10]; dst[11] = src[11];
				src += 24; dst += 12;
			}
			break;

		default:;
	}

	cvt->len_cvt /= 2;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, 6);
	}
}

#include <math.h>

// Изменить скорость (медленное преобразование)
void audioRateSLOW(struct tAudioConvert *cvt, uint16_t format, uint8_t channels) {
	double ipos; int i, clen;
	clen = (int)floor((double)cvt->len_cvt / cvt->rate_incr / (double)channels) * (int)channels;

	if ( cvt->rate_incr > 1.0 ) {
		switch (channels) {
			case 6:
				switch (format & 0xFF) {
					case 8: {
						uint8_t *output; output = cvt->buf;
						clen -= clen % 6; ipos = 0.0;
						for ( i=clen/6; i; --i ) {
							int idx = (int)ipos * 6;
							output[0] = cvt->buf[idx];
							output[1] = cvt->buf[idx + 1];
							output[2] = cvt->buf[idx + 2];
							output[3] = cvt->buf[idx + 3];
							output[4] = cvt->buf[idx + 4];
							output[5] = cvt->buf[idx + 5];
							ipos += cvt->rate_incr; output += 6;
						}
						break;
					}

					case 16: {
						uint16_t *output; output = (uint16_t*)cvt->buf;
						clen -= clen % 12; ipos = 0.0;
						for ( i=clen/12; i; --i ) {
							int idx = (int)ipos * 6;
							output[0] = ((uint16_t*)cvt->buf)[idx];
							output[1] = ((uint16_t*)cvt->buf)[idx + 1];
							output[2] = ((uint16_t*)cvt->buf)[idx + 2];
							output[3] = ((uint16_t*)cvt->buf)[idx + 3];
							output[4] = ((uint16_t*)cvt->buf)[idx + 4];
							output[5] = ((uint16_t*)cvt->buf)[idx + 5];
							ipos += cvt->rate_incr; output += 6;
						}
						break;
					}

					default:;
				}
				break;

			case 4:
				switch (format & 0xFF) {
					case 8: {
						uint8_t *output; output = cvt->buf;
						clen &= ~3; ipos = 0.0;
						for ( i=clen/4; i; --i ) {
							int idx = (int)ipos * 4;
							output[0] = cvt->buf[idx];
							output[1] = cvt->buf[idx + 1];
							output[2] = cvt->buf[idx + 2];
							output[3] = cvt->buf[idx + 3];
							ipos += cvt->rate_incr; output += 4;
						}
						break;
					}

					case 16: {
						uint16_t *output; output = (uint16_t*)cvt->buf;
						clen &= ~7; ipos = 0.0;
						for ( i=clen/8; i; --i ) {
							int idx = (int)ipos * 4;
							output[0] = ((uint16_t*)cvt->buf)[idx];
							output[1] = ((uint16_t*)cvt->buf)[idx + 1];
							output[2] = ((uint16_t*)cvt->buf)[idx + 2];
							output[3] = ((uint16_t*)cvt->buf)[idx + 3];
							ipos += cvt->rate_incr; output += 4;
						}
						break;
					}

					default:;
				}
				break;

			case 2:
				switch (format & 0xFF) {
					case 8: {
						uint8_t *output; output = cvt->buf;
						clen &= ~1; ipos = 0.0;
						for ( i=clen/2; i; --i ) {
							int idx = (int)ipos * 2;
							output[0] = cvt->buf[idx];
							output[1] = cvt->buf[idx + 1];
							ipos += cvt->rate_incr; output += 2;
						}
						break;
					}

					case 16: {
						uint16_t *output; output = (uint16_t*)cvt->buf;
						clen &= ~3; ipos = 0.0;
						for ( i=clen/4; i; --i ) {
							int idx = (int)ipos * 2;
							output[0] = ((uint16_t*)cvt->buf)[idx];
							output[1] = ((uint16_t*)cvt->buf)[idx + 1];
							ipos += cvt->rate_incr; output += 2;
						}
						break;
					}

					default:;
				}
				break;

			default:
				switch (format & 0xFF) {
					case 8: {
						uint8_t *output; output = cvt->buf;
						ipos = 0.0;
						for ( i=clen; i; --i ) {
							*output = cvt->buf[(int)ipos];
							ipos += cvt->rate_incr; output += 1;
						}
						break;
					}

					case 16: {
						uint16_t *output; output = (uint16_t*)cvt->buf;
						clen &= ~1; ipos = 0.0;
						for ( i=clen/2; i; --i ) {
							*output=((uint16_t*)cvt->buf)[(int)ipos];
							ipos += cvt->rate_incr; output += 1;
						}
						break;
					}

					default:;
				}
		}
	} else {
		switch (channels) {
			case 6:
				switch (format & 0xFF) {
					case 8: {
						uint8_t *output; clen -= clen % 6;
						output = cvt->buf+clen;
						ipos = cvt->rate_incr * (double)(clen/6);
						for ( i=clen/6; i; --i ) {
							ipos -= cvt->rate_incr; output -= 6;
							int idx = (int)ipos * 6;
							output[0] = cvt->buf[idx];
							output[1] = cvt->buf[idx + 1];
							output[2] = cvt->buf[idx + 2];
							output[3] = cvt->buf[idx + 3];
							output[4] = cvt->buf[idx + 4];
							output[5] = cvt->buf[idx + 5];
						}
						break;
					}

					case 16: {
						uint16_t *output; clen -= clen % 12;
						output = (uint16_t*)(cvt->buf+clen);
						ipos = cvt->rate_incr * (double)(clen/12);
						for ( i=clen/12; i; --i ) {
							ipos -= cvt->rate_incr; output -= 6;
							int idx = (int)ipos * 6;
							output[0] = ((uint16_t*)cvt->buf)[idx];
							output[1] = ((uint16_t*)cvt->buf)[idx + 1];
							output[2] = ((uint16_t*)cvt->buf)[idx + 2];
							output[3] = ((uint16_t*)cvt->buf)[idx + 3];
							output[4] = ((uint16_t*)cvt->buf)[idx + 4];
							output[5] = ((uint16_t*)cvt->buf)[idx + 5];
						}
						break;
					}

					default:;
				}
				break;

			case 4:
				switch (format & 0xFF) {
					case 8: {
						uint8_t *output; clen &= ~3;
						output = cvt->buf+clen;
						ipos = cvt->rate_incr * (double)(clen/4);
						for ( i=clen/4; i; --i ) {
							ipos -= cvt->rate_incr; output -= 4;
							int idx = (int)ipos * 4;
							output[0] = cvt->buf[idx];
							output[1] = cvt->buf[idx + 1];
							output[2] = cvt->buf[idx + 2];
							output[3] = cvt->buf[idx + 3];
						}
						break;
					}

					case 16: {
						uint16_t *output; clen &= ~7;
						output = (uint16_t*)(cvt->buf+clen);
						ipos = cvt->rate_incr * (double)(clen/8);
						for ( i=clen/8; i; --i ) {
							ipos -= cvt->rate_incr; output -= 4;
							int idx = (int)ipos * 4;
							output[0] = ((uint16_t*)cvt->buf)[idx];
							output[1] = ((uint16_t*)cvt->buf)[idx + 1];
							output[2] = ((uint16_t*)cvt->buf)[idx + 2];
							output[3] = ((uint16_t*)cvt->buf)[idx + 3];
						}
						break;
					}

					default:;
				}
				break;

			case 2:
				switch (format & 0xFF) {
					case 8: {
						uint8_t *output; clen &= ~1;
						output = cvt->buf+clen;
						ipos = cvt->rate_incr * (double)(clen/2);
						for ( i=clen/2; i; --i ) {
							ipos -= cvt->rate_incr; output -= 2;
							int idx = (int)ipos * 2;
							output[0] = cvt->buf[idx];
							output[1] = cvt->buf[idx + 1];
						}
						break;
					}

					case 16: {
						uint16_t *output; clen &= ~3;
						output = (uint16_t*)(cvt->buf+clen);
						ipos = cvt->rate_incr * (double)(clen/4);
						for ( i=clen/4; i; --i ) {
							ipos -= cvt->rate_incr; output -= 2;
							int idx = (int)ipos * 2;
							output[0] = ((uint16_t*)cvt->buf)[idx];
							output[1] = ((uint16_t*)cvt->buf)[idx + 1];
						}
						break;
					}

					default:;
				}
				break;

			default:;
				switch (format & 0xFF) {
					case 8: {
						uint8_t *output; output = cvt->buf+clen;
						ipos = cvt->rate_incr * (double)clen;
						for ( i=clen; i; --i ) {
							ipos -= cvt->rate_incr; output -= 1;
							*output = cvt->buf[(int)ipos];
						}
						break;
					}

					case 16: {
						uint16_t *output; clen &= ~1;
						output = (uint16_t*)(cvt->buf+clen);
						ipos = cvt->rate_incr * (double)(clen/2);
						for ( i=clen/2; i; --i ) {
							ipos -= cvt->rate_incr; output -= 1;
							*output=((uint16_t*)cvt->buf)[(int)ipos];
						}
						break;
					}

					default:;
				}
		}
	}

	cvt->len_cvt = (uint32_t)clen;
	if ( cvt->filters[++cvt->filter_index] ) {
		cvt->filters[cvt->filter_index](cvt, format, channels);
	}
}

int convertAudio(struct tAudioConvert *cvt, uint8_t src_channels) {
	// Убеждаемся, что есть данные для преобразований
	if ( cvt->buf == 0 ) {
		// SetError("No buffer allocated for conversion");
		return -1;
	}
	// Возврат, если преобразования не требуются
	cvt->len_cvt = cvt->len;
	if ( cvt->filters[0] == 0 ) return 0;
	// Инициализация преобразований и запуск
	cvt->filter_index = 0;
	cvt->filters[0](cvt, cvt->src_format, src_channels);
	return 0;
}

// Создать набор аудиофильтров для преобразования из одного формата в другой
// normalizeVolume < 0 - оставить текущую громкость | 1 - максимальная громкость
// -1 - преобразование не возможно | 1 - преобразования установлены
int buildAudioCVT(struct tAudioConvert *cvt,
				  uint16_t src_format, uint8_t src_channels, uint32_t src_rate,
				  uint16_t dst_format, uint8_t dst_channels, uint32_t dst_rate)
{
	cvt->needed = 0;
	cvt->filter_index = 0;
	cvt->filters[0] = 0;
	cvt->len_mult = 1;
	cvt->len_ratio = 1.0;

	// Первый фильтр: порядок байт
	if ( (src_format & 0x1000) != (dst_format & 0x1000)
		 && ((src_format & 0xff) == 16) && ((dst_format & 0xff) == 16)) {
		cvt->filters[cvt->filter_index++] = audioConvertEndian;
	}

	// Второй фильтр: со знаком/без знака
	if ( (src_format & 0x8000) != (dst_format & 0x8000) ) {
		cvt->filters[cvt->filter_index++] = audioConvertSign;
	}

	// Третий фильтр: 16 bit <--> 8 bit
	if ( (src_format & 0xFF) != (dst_format & 0xFF) ) {
		switch (dst_format&0x10FF) {
			case AUDIO_U16LSB:
				cvt->filters[cvt->filter_index++] = audioConvert16LSB; cvt->len_mult *= 2; cvt->len_ratio *= 2;
				break;
			case AUDIO_U16MSB:
				cvt->filters[cvt->filter_index++] = audioConvert16MSB; cvt->len_mult *= 2; cvt->len_ratio *= 2;
				break;
			case AUDIO_U8:
			default:
				cvt->filters[cvt->filter_index++] = audioConvert8; cvt->len_ratio /= 2;
				break;
		}
	}

	// Последний фильтр: Моно/Стерео
	if ( src_channels != dst_channels ) {
		if ( (src_channels == 1) && (dst_channels > 1) ) {
			cvt->filters[cvt->filter_index++] = audioConvertStereo;
			cvt->len_mult *= 2; src_channels = 2; cvt->len_ratio *= 2;
		}
		if ( (src_channels == 2) && (dst_channels == 6) ) {
			cvt->filters[cvt->filter_index++] = audioConvertSurround;
			src_channels = 6; cvt->len_mult *= 3; cvt->len_ratio *= 3;
		}
		if ( (src_channels == 2) && (dst_channels == 4) ) {
			cvt->filters[cvt->filter_index++] = audioConvertSurround_4;
			src_channels = 4; cvt->len_mult *= 2; cvt->len_ratio *= 2;
		}
		while ( (src_channels*2) <= dst_channels ) {
			cvt->filters[cvt->filter_index++] = audioConvertStereo;
			cvt->len_mult *= 2; src_channels *= 2; cvt->len_ratio *= 2;
		}
		if ( (src_channels == 6) && (dst_channels <= 2) ) {
			cvt->filters[cvt->filter_index++] = audioConvertStrip;
			src_channels = 2; cvt->len_ratio /= 3;
		}
		if ( (src_channels == 6) && (dst_channels == 4) ) {
			cvt->filters[cvt->filter_index++] = audioConvertStrip_2;
			src_channels = 4; cvt->len_ratio /= 2;
		}
		// Предположим что 4 канальные данные хранятся в формате
		// Левый {перед/зад} + Правый {перед/зад}
		while ( ((src_channels%2) == 0) && ((src_channels/2) >= dst_channels) ) {
			cvt->filters[cvt->filter_index++] = audioConvertMono;
			src_channels /= 2; cvt->len_ratio /= 2;
		}
		// if ( src_channels != dst_channels ) // такого не должно быть со стандартным звуком
	}

	// Преобразование скорости
	cvt->rate_incr = 0.0;
	if ( src_rate != dst_rate ) {
		uint32_t hi_rate, lo_rate;
		int len_mult; double len_ratio;
		AudioConvertFilterCallback rate_cvt;
		if ( src_rate > dst_rate ) {
			hi_rate = src_rate; lo_rate = dst_rate;
			switch (src_channels) {
				case 1: rate_cvt = audioRateDIV2; break;
				case 2: rate_cvt = audioRateDIV2_c2; break;
				case 4: rate_cvt = audioRateDIV2_c4; break;
				case 6: rate_cvt = audioRateDIV2_c6; break;
				default: return -1;
			}
			len_mult = 1; len_ratio = 0.5;
		} else {
			hi_rate = dst_rate; lo_rate = src_rate;
			switch (src_channels) {
				case 1: rate_cvt = audioRateMUL2; break;
				case 2: rate_cvt = audioRateMUL2_c2; break;
				case 4: rate_cvt = audioRateMUL2_c4; break;
				case 6: rate_cvt = audioRateMUL2_c6; break;
				default: return -1;
			}
			len_mult = 2; len_ratio = 2.0;
		}
		// если hi_rate = lo_rate*2^x то преобразование примтивно
		while ( (lo_rate*2) <= hi_rate ) {
			cvt->filters[cvt->filter_index++] = rate_cvt;
			cvt->len_mult *= len_mult; lo_rate *= 2;
			cvt->len_ratio *= len_ratio;
		}
		// Может понадобится медленное преобразование для завершения
		if ( lo_rate != hi_rate ) {
			if ( src_rate < dst_rate ) {
				cvt->rate_incr = (double)lo_rate/hi_rate;
				cvt->len_mult *= 2;
				cvt->len_ratio /= cvt->rate_incr;
			} else {
				cvt->rate_incr = (double)hi_rate/lo_rate;
				cvt->len_ratio *= cvt->rate_incr;
			}
			cvt->filters[cvt->filter_index++] = audioRateSLOW;
		}
	}

	// Завершающие установки фильтра
	if ( cvt->filter_index != 0 ) {
		cvt->needed = 1;
		cvt->src_format = src_format;
		cvt->dst_format = dst_format;
		cvt->len = 0;
		cvt->buf = 0;
		cvt->filters[cvt->filter_index] = 0;
	}

	return(cvt->needed);
}
