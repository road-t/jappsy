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

#ifndef JAPPSY_UAUDIOCVT_H
#define JAPPSY_UAUDIOCVT_H

#include <platform.h>
#include <sound/uAudio.h>

#ifdef __cplusplus
extern "C" {
#endif

int convertAudio(struct tAudioConvert *cvt, uint8_t src_channels);

int buildAudioCVT(struct tAudioConvert *cvt,
				  uint16_t src_format, uint8_t src_channels, uint32_t src_rate,
				  uint16_t dst_format, uint8_t dst_channels, uint32_t dst_rate);

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_UAUDIOCVT_H
