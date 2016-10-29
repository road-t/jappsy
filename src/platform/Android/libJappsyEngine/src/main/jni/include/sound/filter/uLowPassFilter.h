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

#ifndef JAPPSY_ULOWPASSFILTER_H
#define JAPPSY_ULOWPASSFILTER_H

#include <platform.h>

#ifdef __cplusplus
extern "C" {
#endif

int resampleLinearMono(int16_t* inSamples, int inSamplesCount, uint32_t inFreq, int16_t* outSamples, int outSamplesCount, uint32_t outFreq, uint32_t& time);
int resampleLinearStereo(int16_t* inSamples, int inSamplesCount, uint32_t inFreq, int16_t* outSamples, int outSamplesCount, uint32_t outFreq, uint32_t& time);

struct tLowPassFilter {
	bool ready;

	uint32_t impulseCount;

	int32_t* filter;
	int32_t* filterDiff;

	uint32_t filterSize;
	double freqPercent;
	double betaEnergyCoef;
	uint32_t impulseWidth;
	double scaleFactor;

	void buildKaiserWindowedSincFilter();
	int16_t filterUpSampling(bool interpolate, int32_t* Xp, int32_t Ph, int32_t inc);
	int16_t filterUpDownSampling(bool interpolate, int32_t* Xp, int32_t Ph, int32_t inc, uint32_t dhb);
};

struct tLowPassFilter* buildLowPassFilter(uint32_t impulseCount, double freqPercent, double betaEnergyCoef);

int resampleLinear(int16_t* inSamples, int inSamplesCount, uint32_t inFreq, int16_t* outSamples, int outSamplesCount, uint32_t outFreq, int channels, uint32_t& time);
int resampleFilter(int16_t* inSamples, int inSamplesCount, uint32_t inFreq, int16_t* outSamples, int outSamplesCount, uint32_t outFreq, int channels, uint32_t& time, struct tLowPassFilter* filter);

void initLowPassFilter();
void quitLowPassFilter();

#ifdef __cplusplus
}
#endif

#endif //JAPPSY_ULOWPASSFILTER_H
