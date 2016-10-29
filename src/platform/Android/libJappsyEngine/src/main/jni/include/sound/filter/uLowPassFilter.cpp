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

#include "uLowPassFilter.h"
#include <math.h>
#include <core/uMemory.h>
#include <core/uAtomic.h>

#ifdef __cplusplus
extern "C" {
#endif

static struct tLowPassFilter* lowPassFilterCache = NULL;
static int lowPassFilterCacheSize = 0;
static jlock lowPassFilterCacheLock = false;

void initLowPassFilter() {

}

void quitLowPassFilter() {
	if (lowPassFilterCacheSize != 0) {
		for (int i = 0; i < lowPassFilterCacheSize; i++) {
			memFree(lowPassFilterCache[i].filter);
			lowPassFilterCache[i].filter = NULL;
		}

		lowPassFilterCacheSize = 0;
	}

	if (lowPassFilterCache != NULL) {
		memFree(lowPassFilterCache);

		lowPassFilterCache = NULL;
	}
}

// Модифицированная функция Бесселя первого рода нулевого порядка
static double kaiserIzero(double x) {
	double sum, u, xx;
	int n;

	sum = u = 1.0;
	xx = x * x / 4.0;

	n = 1;
	do {
		u *= xx / (double)(n * n);
		n++;
		sum += u;
	} while (u >= 1E-21);

	return sum;
}

#define LPF_IMPULSE_BITS	8
#define LPF_IMPULSE_WIDTH	(1<<LPF_IMPULSE_BITS)
#define LPF_FILTER_BITS		16
#define LPF_SCALE_BITS		13

void tLowPassFilter::buildKaiserWindowedSincFilter() {
	double* doubleFilter = memAlloc(double, doubleFilter, sizeof(double) * filterSize);
	if (doubleFilter == NULL) {
		ready = false;
		return;
	}

	LOG("buildKaiserWindowedSincFilter: Size %d, Freq %lf, Beta %lf", filterSize, freqPercent, betaEnergyCoef);

	// Формируем Sinc импульс
	doubleFilter[0] = 2.0 * freqPercent;
	for (uint32_t i = 1; i < filterSize; i++) {
		double x = M_PI * (double)i / (double)impulseWidth;
		doubleFilter[i] = sin(2.0 * x * freqPercent) / x;
	}

	// Применяем Окно Кайзера и вычисляем максимум
	double max = 0.0;
	double Ibeta = 1.0 / kaiserIzero(betaEnergyCoef);
	double inm12 = 1.0 / (double)((filterSize - 1) * (filterSize - 1));
	for (uint32_t i = 1; i < filterSize; i++) {
		doubleFilter[i] *= kaiserIzero(betaEnergyCoef * sqrt(1.0 - (double)(i * i) * inm12)) * Ibeta;
		if (max < fabs(doubleFilter[i])) max = fabs(doubleFilter[i]);

		//LOG("sinc: %lf", doubleFilter[i]);
	}

	// Вычисляем усиление
	double gain = 0.0;
	for (uint32_t i = impulseWidth; i < filterSize; i += impulseWidth) {
		gain += doubleFilter[i];
	}
	gain = 2.0 * gain + doubleFilter[0];
	//LOG("gain: %lf", gain);

	// Вычисляем умножение
	double scale = (double)((1<<(LPF_FILTER_BITS - 1))-1) / max;
	//LOG("scale: %lf", scale);
	scaleFactor = fabs((double)(1<<(LPF_SCALE_BITS+LPF_FILTER_BITS)) / (gain * scale));
	//LOG("scaleFactor: %lf", scaleFactor);
	if (scaleFactor >= (double)(1<<16)) {
		LOG("buildKaiserWindowedSincFilter: Error filter scale factor overflows 16 bit!");

		memFree(doubleFilter);
		ready = false;
		return;
	}

	// Исправляем знак фильтра
	if (doubleFilter[0] < 0) {
		scale = -scale;
	}

	// Масштабируем фильтр
	for (uint32_t i = 0; i < filterSize; i++) {
		doubleFilter[i] *= scale;
		filter[i] = (int32_t)(doubleFilter[i] + 0.5);
	}

	// Вычисляем разницу
	for (uint32_t i = 0; i < (filterSize - 1); i++) {
		filterDiff[i] = filter[i+1] - filter[i];
	}
	filterDiff[filterSize - 1] = -filter[filterSize - 1];

	memFree(doubleFilter);
	ready = true;
	return;
}

// impulseCount от 3 до 65
// freqPercent по умолчанию 0.9, начало затухания частоты выше половины несущей (частота затухания freq * freqPercent / 2)
// betaEnergyCoef обычно используется от 4 до 9
struct tLowPassFilter* buildLowPassFilter(uint32_t impulseCount, double freqPercent, double betaEnergyCoef) {
	impulseCount ^= 1; // Исправляем число импульсов до нечетного
	if (impulseCount < 3) impulseCount = 3;

	uint32_t filterSize = LPF_IMPULSE_WIDTH * (impulseCount - 1) / 2;

	if (filterSize > 8192) {
		LOG("buildLowPassFilter: Error filter size!");
		return NULL;
	}

	if ((freqPercent <= 0.0) || (freqPercent > 1.0)) {
		LOG("buildLowPassFilter: Error freq percent!");
		return NULL;
	}

	if (betaEnergyCoef < 1.0) {
		LOG("buildLowPassFilter: Error beta energy coef!");
		return NULL;
	}

	// Формируем функцию фильтра

	AtomicLock(&lowPassFilterCacheLock);

	struct tLowPassFilter* lpFilter = NULL;
	if ((lowPassFilterCache != NULL) && (lowPassFilterCacheSize > 0)) {
		for (int i = 0; i < lowPassFilterCacheSize; i++) {
			if ((lowPassFilterCache[i].filterSize == filterSize) &&
						(lowPassFilterCache[i].freqPercent == freqPercent) &&
						(lowPassFilterCache[i].betaEnergyCoef == betaEnergyCoef) &&
						(lowPassFilterCache[i].impulseWidth == LPF_IMPULSE_WIDTH)) {
				lpFilter = &(lowPassFilterCache[i]);
				break;
			}
		}
	}
	if (lpFilter == NULL) {
		int32_t* filterData = memAlloc(int32_t, filterData, sizeof(int32_t) * filterSize * 2);

		if (filterData == NULL) {
			AtomicUnlock(&lowPassFilterCacheLock);
			return NULL;
		}

		struct tLowPassFilter* newCache = memRealloc(struct tLowPassFilter, newCache, lowPassFilterCache, sizeof(struct tLowPassFilter) * (lowPassFilterCacheSize + 1));
		if (newCache == NULL) {
			memDelete(filterData);
			AtomicUnlock(&lowPassFilterCacheLock);
			return NULL;
		}

		lowPassFilterCache = newCache;
		lpFilter = &(lowPassFilterCache[lowPassFilterCacheSize++]);

		lpFilter->impulseCount = impulseCount;
		lpFilter->filter = filterData;
		lpFilter->filterDiff = filterData + filterSize;
		lpFilter->filterSize = filterSize;
		lpFilter->freqPercent = freqPercent;
		lpFilter->betaEnergyCoef = betaEnergyCoef;
		lpFilter->impulseWidth = LPF_IMPULSE_WIDTH;

		lpFilter->buildKaiserWindowedSincFilter();
	}

	AtomicUnlock(&lowPassFilterCacheLock);

	if (lpFilter->ready) {
		return lpFilter;
	}

	return NULL;
}

#define LPF_LINEAR_INTERPOLATION_BITS	7
#define LPF_FIXED_POINT_BITS			(LPF_IMPULSE_BITS + LPF_LINEAR_INTERPOLATION_BITS)
#define LPF_LINEAR_INTERPOLATION_MASK	((1<<LPF_LINEAR_INTERPOLATION_BITS)-1)
#define LPF_FIXED_POINT_MASK			((1<<LPF_FIXED_POINT_BITS)-1)

//#define Nhc       8				//LPF_IMPULSE_BITS
//#define Na        7				//LPF_LINEAR_INTERPLATION_BITS
//#define Np       (Nhc+Na)			//LPF_FIXED_POINT_BITS
//#define Npc      (1<<Nhc)			//LPF_IMPULSE_WIDTH
//#define Amask    ((1<<Na)-1)		//LPF_LINEAR_INTERPOLATION_MASK
//#define Pmask    ((1<<Np)-1)		//LPF_FIXED_POINT_MASK
#define Nh       16					//LPF_FILTER_BITS
#define Nb       16
#define Nhxn     14
#define Nhg      (Nh-Nhxn)
#define NLpScl   13					//LPF_SCALE_BITS

int16_t tLowPassFilter::filterUpSampling(bool interpolate, int32_t* Xp, int32_t Ph, int32_t inc) {
	int32_t* Hp;
	int32_t* Hdp = NULL;
	int32_t* End;
	int32_t a = 0;
	int16_t v, t;

	v = 0;
	Hp = &filter[Ph>>LPF_LINEAR_INTERPOLATION_BITS];
	End = &filter[filterSize];
	if (interpolate) {
		Hdp = &filterDiff[Ph>>LPF_LINEAR_INTERPOLATION_BITS];
		a = Ph & LPF_LINEAR_INTERPOLATION_MASK;
	}
	if (inc == 1) {
		End--;
		if (Ph == 0) {
			Hp += LPF_IMPULSE_WIDTH;
			Hdp += LPF_IMPULSE_WIDTH;
		}
	}
	if (interpolate) {
		while (Hp < End) {
			t = *Hp;
			t += (((int16_t)*Hdp)*a)>>LPF_LINEAR_INTERPOLATION_BITS;
			Hdp += LPF_IMPULSE_WIDTH;
			t *= *Xp;
			if (t & (1<<(Nhxn-1))) {
				t += (1<<(Nhxn-1));
			}
			t >>= Nhxn;
			v += t;
			Hp += LPF_IMPULSE_WIDTH;
			Xp += inc;
		}
	} else {
		while (Hp < End) {
			t = *Hp;
			t *= *Xp;
			if (t & (1<<(Nhxn-1))) {
				t += (1<<(Nhxn-1));
			}
			t >>= Nhxn;
			v += t;
			Hp += LPF_IMPULSE_WIDTH;
			Xp += inc;
		}
	}

	return v;
}

int16_t tLowPassFilter::filterUpDownSampling(bool interpolate, int32_t* Xp, int32_t Ph, int32_t inc, uint32_t dhb) {
	int32_t a;
	int32_t* Hp;
	int32_t* Hdp;
	int32_t* End;
	int16_t v, t;
	int32_t Ho;

	v = 0;
	Ho = (Ph*(uint16_t)dhb)>>LPF_FIXED_POINT_BITS;
	End = &filter[filterSize];
	if (inc == 1) {
		End--;
		if (Ph == 0) {
			Ho += dhb;
		}
	}
	if (interpolate) {
		while ((Hp = &filter[Ho>>LPF_LINEAR_INTERPOLATION_BITS]) < End) {
			t = *Hp;
			Hdp = &filterDiff[Ho>>LPF_LINEAR_INTERPOLATION_BITS];
			a = Ho & LPF_LINEAR_INTERPOLATION_MASK;
			t += (((int16_t)*Hdp)*a)>>LPF_LINEAR_INTERPOLATION_BITS;
			t *= *Xp;
			if (t & (1<<(Nhxn-1))) {
				t += (1<<(Nhxn-1));
			}
			t >>= Nhxn;
			v += t;
			Ho += dhb;
			Xp += inc;
		}
	} else {
		while ((Hp = &filter[Ho>>LPF_LINEAR_INTERPOLATION_BITS]) < End) {
			t = *Hp;
			t *= *Xp;
			if (t & (1<<(Nhxn-1))) {
				t += (1<<(Nhxn-1));
			}
			t >>= Nhxn;
			v += t;
			Ho += dhb;
			Xp += inc;
		}
	}

	return v;
}

int resampleLinearMono(int16_t* inSamples, int inSamplesCount, uint32_t inFreq, int16_t* outSamples, int outSamplesCount, uint32_t outFreq, uint32_t& time) {
	uint32_t timeDelta = ((1<<LPF_FIXED_POINT_BITS) * inFreq * 2 / outFreq) >> 1;
	uint32_t endTime = time + (1<<LPF_FIXED_POINT_BITS) * inSamplesCount;

	int16_t* out = outSamples;
	while ((time < endTime) && (outSamplesCount-- > 0)) {
		uint32_t fract = time & LPF_FIXED_POINT_MASK;
		int16_t* in = &(inSamples[time >> LPF_FIXED_POINT_BITS]);
		*out++ = (int16_t)((((int32_t)in[0] * ((1<<LPF_FIXED_POINT_BITS) - fract)) + ((int32_t)in[1] * fract)) >> LPF_FIXED_POINT_BITS);
		time += timeDelta;
	}

	return (int)(((intptr_t)out - (intptr_t)outSamples) / sizeof(int16_t));
}

int resampleLinearStereo(int16_t* inSamples, int inSamplesCount, uint32_t inFreq, int16_t* outSamples, int outSamplesCount, uint32_t outFreq, uint32_t& time) {
	uint32_t timeDelta = ((1<<LPF_FIXED_POINT_BITS) * inFreq * 2 / outFreq) >> 1;
	uint32_t endTime = time + (1<<LPF_FIXED_POINT_BITS) * inSamplesCount;

	int16_t* out = outSamples;
	while ((time < endTime) && (outSamplesCount-- > 0)) {
		uint32_t fract = time & LPF_FIXED_POINT_MASK;
		int16_t* in = &(inSamples[(time >> LPF_FIXED_POINT_BITS) * 2]);
		*out++ = (int16_t)((((int32_t)in[0] * ((1<<LPF_FIXED_POINT_BITS) - fract)) + ((int32_t)in[2] * fract)) >> LPF_FIXED_POINT_BITS);
		*out++ = (int16_t)((((int32_t)in[1] * ((1<<LPF_FIXED_POINT_BITS) - fract)) + ((int32_t)in[3] * fract)) >> LPF_FIXED_POINT_BITS);
		time += timeDelta;
	}

	return (int)(((intptr_t)out - (intptr_t)outSamples) / (sizeof(int16_t) * 2));
}


// freqFactor = outFreq / srcFreq
int resampleFilter(int16_t* inSamples, int inSamplesCount, uint32_t inFreq, int16_t* outSamples, int outSamplesCount, uint32_t outFreq, int channels, uint32_t& time, struct tLowPassFilter* filter) {
	double freqFactor = (double)outFreq / (double)inFreq;

}

#ifdef __cplusplus
}
#endif
