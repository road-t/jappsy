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

#ifndef JAPPSY_UMP3SYNTH_H
#define JAPPSY_UMP3SYNTH_H

#include <platform.h>
#include "uMP3Decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*tMPADecSynth)(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);

void synth_full16lmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_full16lsm synth_full16lmm
void synth_full16lms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full16lss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full16bmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_full16bsm synth_full16bmm
void synth_full16bms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full16bss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full24lmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_full24lsm synth_full24lmm
void synth_full24lms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full24lss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full24bmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_full24bsm synth_full24bmm
void synth_full24bms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full24bss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full32lmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_full32lsm synth_full32lmm
void synth_full32lms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full32lss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full32bmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_full32bsm synth_full32bmm
void synth_full32bms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full32bss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full32flmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_full32flsm synth_full32flmm
void synth_full32flms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full32flss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full32fbmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_full32fbsm synth_full32fbmm
void synth_full32fbms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_full32fbss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half16lmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_half16lsm synth_half16lmm
void synth_half16lms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half16lss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half16bmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_half16bsm synth_half16bmm
void synth_half16bms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half16bss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half24lmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_half24lsm synth_half24lmm
void synth_half24lms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half24lss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half24bmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_half24bsm synth_half24bmm
void synth_half24bms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half24bss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half32lmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_half32lsm synth_half32lmm
void synth_half32lms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half32lss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half32bmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_half32bsm synth_half32bmm
void synth_half32bms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half32bss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half32flmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_half32flsm synth_half32flmm
void synth_half32flms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half32flss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half32fbmm(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
#define synth_half32fbsm synth_half32fbmm
void synth_half32fbms(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);
void synth_half32fbss(struct tMPADecoder* mpadec, double *bandptr, int channel, uint8_t *buffer);

extern tMPADecSynth mpadec_synth_table[2][2][4][4];

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_UMP3SYNTH_H
