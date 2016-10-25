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

#ifndef JAPPSY_UMP3TABLES_H
#define JAPPSY_UMP3TABLES_H

#include <platform.h>
#include "uMP3Decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int mpadec_tables_init;
extern struct tMPADecoderTables mpadec_tables;

void mpadec_init_tables(struct tMPADecoder* mpadec, double scale, int32_t sblimit);

extern struct tMPABandInfo mpadec_band_info[9];
extern struct tMPAAllocTable *mpadec_alloc_tables[5];
extern const double mpadec_newcos[8];
extern const double mpadec_tfcos36[9];
extern const double mpadec_tfcos12[3];
extern const double mpadec_cs[8];
extern const double mpadec_ca[8];
extern int16_t mpadec_tab0[];
extern int16_t mpadec_tab1[];
extern int16_t mpadec_tab2[];
extern int16_t mpadec_tab3[];
extern int16_t mpadec_tab5[];
extern int16_t mpadec_tab6[];
extern int16_t mpadec_tab7[];
extern int16_t mpadec_tab8[];
extern int16_t mpadec_tab9[];
extern int16_t mpadec_tab10[];
extern int16_t mpadec_tab11[];
extern int16_t mpadec_tab12[];
extern int16_t mpadec_tab13[];
extern int16_t mpadec_tab15[];
extern int16_t mpadec_tab16[];
extern int16_t mpadec_tab24[];
extern int16_t mpadec_tab_c0[];
extern int16_t mpadec_tab_c1[];
extern struct tMPANewHuff mpadec_hufft[];
extern struct tMPANewHuff mpadec_hufftc[];

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_UMP3TABLES_H
