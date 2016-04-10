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

#ifndef MODDISPLAY_H
#define MODDISPLAY_H

#include <stdint.h>

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct dis_rect {
	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t orientation;
};

typedef void(*dis_change_func)();

// Возвращает число доступных экранов
DLL_EXPORT int __cdecl dis_count();
// Установка видеорежима для экрана
DLL_EXPORT bool __cdecl dis_fullscreen(uint32_t id, uint32_t width, uint32_t height, uint32_t depth, uint32_t orientation);
// Восстановить видеорежим
DLL_EXPORT void __cdecl dis_restore();
// Получить информацию об экране (максимальное разрешение)
DLL_EXPORT bool __cdecl dis_info(uint32_t id, dis_rect* current, dis_rect* maximum);
// Установить событие на измения экранов
DLL_EXPORT void __cdecl dis_onchange(dis_change_func cb);

#ifdef __cplusplus
}
#endif

#endif
