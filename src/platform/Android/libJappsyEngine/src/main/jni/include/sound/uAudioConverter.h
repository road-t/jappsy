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

#ifndef JAPPSY_UAUDIOCONVERT_H
#define JAPPSY_UAUDIOCONVERT_H

#include <platform.h>
#include <data/uObject.h>

#define AUDIO_FORMAT_BUFFER_LENGTH	4096

#define AUDIO_FORMAT_U8		0x0008	/* Unsigned 8-bit samples */
#define AUDIO_FORMAT_S8		0x8008	/* Signed 8-bit samples */
#define AUDIO_FORMAT_U16LSB	0x0010	/* Unsigned 16-bit samples */
#define AUDIO_FORMAT_S16LSB	0x8010	/* Signed 16-bit samples */
#define AUDIO_FORMAT_U16MSB	0x1010	/* As above, but big-endian byte order */
#define AUDIO_FORMAT_S16MSB	0x9010	/* As above, but big-endian byte order */
#define AUDIO_FORMAT_U16	AUDIO_FORMAT_U16LSB
#define AUDIO_FORMAT_S16	AUDIO_FORMAT_S16LSB

struct tAudioSpec {
	uint32_t frequency;		// Audio frequency
	uint16_t format;		// Audio format (signed, byte order, bit count)
	uint8_t channels;		// Audio channels (mono, stereo, quadro, 5.1)
	size_t sample;			// Audio sample size in bytes
};

class AudioConverter;

typedef void (*AudioFormatConverterCallback)(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength);

class AudioConverter : public CObject {
private:
	AudioFormatConverterCallback formatConverter[10] = {0};

	static void convertEndian(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength);

	static void convertSign(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength);

	static void convert16LSB(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength);
	static void convert16MSB(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength);
	static void convert8(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength);

	static void convertStereo(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength);
	static void convertSurround40(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength);
	static void convertSurround51(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength);

	static void convertMono(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength);
	static void convertStrip(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength);
	static void convertStrip2(AudioConverter* context, int32_t converterIndex, uint16_t currentFormat, uint8_t currentChannels, size_t currentLength);

public:
	bool convert = false;

	struct tAudioSpec inSpec = {0};
	struct tAudioSpec outSpec = {0};

	uint8_t* formatBuffer = NULL;
	size_t formatBufferInLength = 0;
	size_t formatBufferOutLength = 0;

	AudioConverter();
	~AudioConverter();

	bool build(uint16_t outFormat, uint8_t outChannels, uint32_t outFreq);
	size_t process(size_t filled, size_t& used);
};

#endif //JAPPSY_UAUDIOCONVERT_H
