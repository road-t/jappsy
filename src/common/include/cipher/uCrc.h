/*
 * Created by VipDev on 01.04.16.
 *
 * Copyright (C) 2016 The Jappsy Open Source Project
 *
 * Project Web Page http://jappsy.com
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

#ifndef JAPPSY_UCRC_H
#define JAPPSY_UCRC_H

#include "../platform.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t mmcrc7(register uint8_t crc, void *data, register uint32_t len);
uint16_t mmcrc16(register uint16_t crc, void *data, register uint32_t len);
uint32_t mmcrc32(register uint32_t crc, void *data, register uint32_t len);

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_UCRC_H