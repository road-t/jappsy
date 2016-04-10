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

#ifndef JAPPSY_UMD5_H
#define JAPPSY_UMD5_H

#include "../platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct tMD5 {
    int64_t count;
    uint8_t part[64];
    uint32_t state[4];
};

void md5_init(struct tMD5 *md5);
void md5_transform(struct tMD5 *md5, uint32_t *x);
void md5_update(struct tMD5 *md5, uint8_t *buffer, uint32_t size);
void md5_final(struct tMD5 *md5);

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_UMD5_H