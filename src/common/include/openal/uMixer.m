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

#import "uMixer.h"
#import <OpenAL/al.h>
#import <OpenAl/alc.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIKit.h>
#import <libkern/OSAtomic.h>
#import <data/uString.h>
#import <core/uMemory.h>
#import <core/uAtomic.h>

#define kMaxConcurrentSources 32
#define kMusicSize (1024 * 1024) // 1 MB
#define kMusicBufferSize 4096

static OSSpinLock audioPlayerLock = 0;

void lockAudioPlayer() {
	OSSpinLockLock((OSSpinLock*)&(audioPlayerLock));
}

void unlockAudioPlayer() {
	OSSpinLockUnlock((OSSpinLock*)&(audioPlayerLock));
}

static ALCdevice *openALDevice = NULL;
static ALCcontext *openALContext = NULL;

static NSMutableArray *audioChannels = nil;
static NSMutableArray *audioBuffers = nil;
static volatile int32_t activeCount = 0;

void pauseAudioPlayer() {
	alcMakeContextCurrent(NULL);
	
	if ([[UIDevice currentDevice].systemVersion floatValue] >= 7.0f) {
		NSError *deactivationError = nil;
		[[AVAudioSession sharedInstance] setActive:NO error:&deactivationError];
	} else {
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 70000
		AudioSessionSetActive(false);
#endif
	}
}

void resumeAudioPlayer() {
	if ([[UIDevice currentDevice].systemVersion floatValue] >= 7.0f) {
		NSError *error = nil;
		AVAudioSession *session = [AVAudioSession sharedInstance];
		[session setCategory:AVAudioSessionCategoryPlayback error:&error];
	} else {
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 70000
		AudioSessionSetActive(true);
#endif
	}
	
	alcMakeContextCurrent(openALContext);
}

void AudioPlayerInterruptionListenerCallback(void* user_data, UInt32 interruption_state) {
	if (kAudioSessionBeginInterruption == interruption_state) {
		alcMakeContextCurrent(NULL);
	} else if (kAudioSessionEndInterruption == interruption_state) {
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 70000
		AudioSessionSetActive(true);
#endif
		alcMakeContextCurrent(openALContext);
	}
}

void initAudioPlayer() {
	lockAudioPlayer();
	if (openALDevice == NULL) {
		static bool initOnce = true;
		if (initOnce) {
			initOnce = false;
			
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 70000
			if ([[UIDevice currentDevice].systemVersion floatValue] < 7.0f) {
				AudioSessionInitialize(NULL, NULL, AudioPlayerInterruptionListenerCallback, NULL);
				
				UInt32 session_category = kAudioSessionCategory_MediaPlayback;
				AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(session_category), &session_category);
			}
#endif
		}
		
		if ([[UIDevice currentDevice].systemVersion floatValue] >= 7.0f) {
			NSError *error = nil;
			AVAudioSession *session = [AVAudioSession sharedInstance];
			[session setCategory:AVAudioSessionCategoryPlayback error:&error];
		} else {
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 70000
			AudioSessionSetActive(true);
#endif
		}
		
		openALDevice = alcOpenDevice(NULL);
		if (openALDevice == NULL) {
			return;
		}
		
		openALContext = alcCreateContext(openALDevice, NULL);
		if (openALContext == NULL) {
			alcCloseDevice(openALDevice);
			openALDevice = NULL;
			return;
		}
		alcMakeContextCurrent(openALContext);
		
		audioChannels = [[NSMutableArray alloc] init];
		
		ALuint sourceID;
		for (int i = 0; i < kMaxConcurrentSources; i++) {
			alGenSources(1, &sourceID);
			[audioChannels addObject:[NSNumber numberWithUnsignedInt:sourceID]];
		}
		
		audioBuffers = [[NSMutableArray alloc] init];
	}
	unlockAudioPlayer();
}

struct AudioData {
	ExtAudioFileRef handle;
	int64_t totalFrames;
	AudioStreamBasicDescription format;
	int64_t totalSize;
	AudioBufferList list;
};

struct AudioData* initAudioData(NSString* audioFilePath) {
	bool mono = false;
	
	OSStatus error = 0;
	UInt32 size;
	
	NSURL *audioFileURL = [NSURL fileURLWithPath:audioFilePath];
	
	struct AudioData* data = (struct AudioData*)mmalloc(sizeof(struct AudioData));
	if (NULL == data) {
#ifdef DEBUG
		NSLog(@"AudioPlayer: Out of memory");
#endif
		return NULL;
	}
	
	if (0 != (error = ExtAudioFileOpenURL((__bridge CFURLRef)audioFileURL, &(data->handle)))) {
		mmfree(data);
#ifdef DEBUG
		NSLog(@"AudioPlayer: Could not open %@: %d", audioFilePath, (int)error);
#endif
		return NULL;
	}
	
	size = sizeof(SInt64);
	if (0 != (error = ExtAudioFileGetProperty(data->handle, kExtAudioFileProperty_FileLengthFrames, &size, &(data->totalFrames)))) {
		ExtAudioFileDispose(data->handle);
		mmfree(data);
#ifdef DEBUG
		NSLog(@"AudioPlayer: Could not get frame count for %@: %d", audioFilePath, (int)error);
#endif
		return NULL;
	}
	
	if (data->totalFrames <= 0) {
		ExtAudioFileDispose(data->handle);
		mmfree(data);
#ifdef DEBUG
		NSLog(@"AudioPlayer: Could not get frame count for %@: %d", audioFilePath, (int)error);
#endif
		return NULL;
	}
	
	size = sizeof(AudioStreamBasicDescription);
	if (0 != (error = ExtAudioFileGetProperty(data->handle, kExtAudioFileProperty_FileDataFormat, &size, &(data->format)))) {
		ExtAudioFileDispose(data->handle);
		mmfree(data);
#ifdef DEBUG
		NSLog(@"AudioPlayer: Could not get audio format for %@: %d", audioFilePath, (int)error);
#endif
		return NULL;
	}
	
	data->format.mFormatID = kAudioFormatLinearPCM;
	data->format.mFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	data->format.mBitsPerChannel = 16;
	if (mono) {
		data->format.mChannelsPerFrame = 1;
	} else {
		data->format.mChannelsPerFrame = 2;
	}
	data->format.mBytesPerFrame = data->format.mChannelsPerFrame * data->format.mBitsPerChannel / 8;
	data->format.mFramesPerPacket = 1;
	data->format.mBytesPerPacket = data->format.mBytesPerFrame;
	if (data->format.mSampleRate != 44100) {
		data->totalFrames = (int64_t)floor((double)(data->totalFrames) * 44100.0 / (data->format.mSampleRate));
		data->format.mSampleRate = 44100;
	}
	
	size = sizeof(AudioStreamBasicDescription);
	if (0 != (error = ExtAudioFileSetProperty(data->handle, kExtAudioFileProperty_ClientDataFormat, size, &(data->format)))) {
		ExtAudioFileDispose(data->handle);
		mmfree(data);
#ifdef DEBUG
		NSLog(@"AudioPlayer: Could not set new audio format for %@: %d", audioFilePath, (int)error);
#endif
		return NULL;
	}
	
	data->totalSize = data->format.mBytesPerFrame * data->totalFrames;
	
	data->list.mNumberBuffers = 1;
	data->list.mBuffers[0].mNumberChannels = data->format.mChannelsPerFrame;
	
	return data;
}

struct PrepareAudioData {
	char* filePath;
	void* userData;
	AudioReadyCallback onAudioReady;
	AudioErrorCallback onAudioError;
};

#define StateLock       0x8000
#define StateWait       0x4000
#define StateRequest    0x2000
#define StateRelease    0x1000
#define StateThread     0x0800
#define StateRestart    0x0200
#define StateInit       0x0100
#define StateStop       0x0001
#define StatePause      0x0002
#define StatePlay       0x0004
#define StateUpdate     0x0008

uint32_t audioStateLock(uint32_t* state) {
	uint32_t res;
	do {
		res = OSAtomicOr32Orig(StateLock, state);
		if ((res & StateLock) == 0) break;
		usleep(10);
	} while (true);
	return res | StateLock;
}

uint32_t audioStateWaitAndLock(uint32_t* state) {
	uint32_t res;
	do {
		res = OSAtomicOr32Orig(StateLock, state);
		if ((res & StateLock) == 0) {
			if ((res & StateWait) == 0) break;
			OSAtomicXor32(StateLock, state);
		}
		usleep(10);
	} while (true);
	return res | StateLock;
}

// oldState (StateStop | StatePause | StatePlay)
// newState (StateRelease | StateStop | StatePlay)
// returns true on new thread required
bool audioStateCheckThreadAndRequest(uint32_t* state, uint32_t oldState, uint32_t newState) {
	uint32_t res = audioStateWaitAndLock(state);
	if ((res & StateRelease) == 0) {
		if ((res & oldState) != 0) {
			OSAtomicAnd32(~(StateStop | StatePause | StatePlay), state);
			OSAtomicOr32(StateThread | StateRequest, state);
			OSAtomicXor32(newState | StateLock, state);
			return ((res & StateThread) == 0);
		}
	}
	OSAtomicXor32(StateLock, state);
	return false;
}

// returns true on update
bool audioStateUpdate(uint32_t* state) {
	uint32_t res = audioStateWaitAndLock(state);
	if ((res & StateRelease) == 0) {
		if ((res & StateUpdate) == 0) {
			OSAtomicXor32(StateLock | StateUpdate, state);
		} else {
			OSAtomicXor32(StateLock, state);
		}
		return true;
	}
	OSAtomicXor32(StateLock, state);
	return false;
}

// returns true on playing
bool audioStatePlaying(uint32_t* state) {
	uint32_t res = audioStateWaitAndLock(state);
	OSAtomicXor32(StateLock, state);
	return ((res & StatePlay) != 0);
}

// returns true on pause request
bool audioStatePause(uint32_t* state) {
	uint32_t res = audioStateWaitAndLock(state);
	if ((res & StateRelease) == 0) {
		if ((res & StatePlay) != 0) {
			OSAtomicOr32(StateRequest, state);
			OSAtomicXor32(StatePause | StatePlay | StateLock, state);
			return true;
		}
	}
	OSAtomicXor32(StateLock, state);
	return false;
}

// returns true on update request
bool audioStateCheckUpdate(uint32_t* state) {
	uint32_t res = audioStateLock(state);
	if ((res & StateUpdate) != 0) {
		OSAtomicXor32(StateLock | StateUpdate, state);
		return true;
	}
	OSAtomicXor32(StateLock, state);
	return false;
}

// state (StateRelease | StateStop | StatePause | StatePlay)
// returns true on update to new state
bool audioStateCheckRequestAndWait(uint32_t* state, uint32_t oldState, uint32_t newState) {
	uint32_t res = audioStateLock(state);
	if ((res & StateRequest) != 0) {
		if ((res & oldState) != 0) {
			OSAtomicAnd32(~(StateRequest | oldState), state);
			OSAtomicXor32(newState | StateLock | StateWait, state);
			return true;
		}
	}
	OSAtomicXor32(StateLock, state);
	return false;
}

// oldState (StateStop | StatePause | StatePlay)
void audioStateCheckRequestAndStop(uint32_t* state, uint32_t oldState) {
	uint32_t res = audioStateLock(state);
	if ((res & StateRelease) == 0) {
		if ((res & oldState) != 0) {
			OSAtomicAnd32(~(StateStop | StatePause | StatePlay), state);
			OSAtomicOr32(StateRequest | StateStop, state);
		}
	}
	OSAtomicXor32(StateLock, state);
}

// returns true on play request
bool audioStateCheckPlay(uint32_t* state) {
	uint32_t res = audioStateLock(state);
	if ((res & (StateRequest | StatePlay)) == (StateRequest | StatePlay)) {
		OSAtomicXor32(StateLock | StateRequest, state);
		return true;
	}
	OSAtomicXor32(StateLock, state);
	return false;
}

// returns true on init request
bool audioStateCheckInit(uint32_t* state) {
	uint32_t res = audioStateLock(state);
	if ((res & StateInit) != 0) {
		OSAtomicXor32(StateLock | StateInit, state);
		return true;
	}
	OSAtomicXor32(StateLock, state);
	return false;
}

// returns true on restart request
bool audioStateCheckRestart(uint32_t* state) {
	uint32_t res = audioStateLock(state);
	if ((res & StateRestart) != 0) {
		OSAtomicXor32(StateLock | StateRestart, state);
		return true;
	}
	OSAtomicXor32(StateLock, state);
	return false;
}

void audioStateEndThread(uint32_t* state) {
	OSAtomicAnd32(~(StateWait | StateThread), state);
}

struct SoundBuffer {
	int64_t size;
	volatile int32_t atom_state;
	volatile ALuint sampleBuffer;
	volatile int32_t atom_volume;
	volatile int32_t atom_loop;
	volatile uint32_t atom_activeChannel;
	
	volatile int64_t totalSize;
};

struct SoundBuffer* mmallocSoundBuffer(struct AudioData* data) {
	struct SoundBuffer* buffer = (struct SoundBuffer*)mmalloc(sizeof(struct SoundBuffer));
	if (buffer == NULL) {
		return NULL;
	} else {
		OSStatus error = 0;
		
		void* streamData = mmalloc((uint32_t)(data->totalSize));
		if (NULL == streamData) {
			mmfree(buffer);
			return NULL;
		}
		
		UInt32 bufferOffset = 0;
		UInt32 numFramesRead;
		for (uint32_t framesToRead = (uint32_t)(data->totalFrames); framesToRead > 0; framesToRead -= numFramesRead) {
			data->list.mBuffers[0].mDataByteSize = data->format.mBytesPerFrame * framesToRead;
			data->list.mBuffers[0].mData = (char*)streamData + bufferOffset;
			
			numFramesRead = framesToRead;
			if (0 != (error = ExtAudioFileRead(data->handle, &numFramesRead, &(data->list)))) {
				mmfree(streamData);
				mmfree(buffer);
				return NULL;
			}
			
			bufferOffset += data->format.mBytesPerFrame * numFramesRead;
			
			if (numFramesRead == 0) {
				break;
			}
		}
		data->totalSize = bufferOffset;
		
		ALuint outputBuffer;
		alGenBuffers(1, &outputBuffer);
		
		alBufferData(outputBuffer, AL_FORMAT_STEREO16, streamData, (ALsizei)(data->totalSize), 44100);
		
		mmfree(streamData);
		
		buffer->size = sizeof(struct SoundBuffer);
		AtomicExchange(&(buffer->atom_state), StateInit | StateStop);
		AtomicExchange((int32_t*)&(buffer->atom_activeChannel), -1);
		buffer->sampleBuffer = outputBuffer;
		AtomicExchange(&(buffer->atom_volume), 65536);
		AtomicExchange(&(buffer->atom_loop), 0);
		buffer->totalSize = data->totalSize;
		
		return buffer;
	}
}

struct MusicBuffer {
	int64_t size;
	volatile int32_t atom_state;
	volatile ALuint sampleBuffers[3];
	volatile int32_t atom_activeBuffers[3];
	volatile int32_t atom_volume;
	volatile int32_t atom_loop;
	volatile uint32_t atom_activeChannel;
	
	volatile void* streamData;
	volatile int32_t bufferSize;
	volatile int64_t totalSize;
	
	ExtAudioFileRef handle;
	int64_t totalFrames;
	UInt32 bytesPerFrame;
	AudioBufferList list;
	volatile int32_t atom_currentFrame;
};

SInt32 fillSampleBuffer(int32_t sampleBuffer, struct MusicBuffer* music, bool loop) {
	OSStatus error = 0;
	
	UInt32 bufferOffset = 0;
	UInt32 numFramesRead;
	uint32_t framesToRead = (uint32_t)(music->totalFrames - AtomicCompareExchange(&(music->atom_currentFrame), 0, 0));
	if (framesToRead > kMusicBufferSize) framesToRead = kMusicBufferSize;
	for (; framesToRead > 0; framesToRead -= numFramesRead) {
		music->list.mBuffers[0].mDataByteSize = music->bytesPerFrame * framesToRead;
		music->list.mBuffers[0].mData = (char*)(music->streamData) + bufferOffset;
		
		numFramesRead = framesToRead;
		if (0 != (error = ExtAudioFileRead(music->handle, &numFramesRead, &(music->list)))) {
			return -1;
		}
		
		bufferOffset += music->bytesPerFrame * numFramesRead;
		AtomicAdd(&(music->atom_currentFrame), numFramesRead);
		
		if ((numFramesRead == 0) || (AtomicCompareExchange(&(music->atom_currentFrame), 0, 0) >= music->totalFrames)) {
			music->totalFrames = AtomicCompareExchange(&(music->atom_currentFrame), 0, 0);
			if (loop) {
				ExtAudioFileSeek(music->handle, 0);
				AtomicExchange(&(music->atom_currentFrame), 0);
			}
			break;
		}
	}
	
	if (bufferOffset > 0) {
		alBufferData(sampleBuffer, AL_FORMAT_STEREO16, (void*)(music->streamData), (ALsizei)(bufferOffset), 44100);
	}
	
	return bufferOffset;
}

struct MusicBuffer* mmallocMusicBuffer(struct AudioData* data) {
	struct MusicBuffer* buffer = (struct MusicBuffer*)mmalloc(sizeof(struct MusicBuffer));
	if (buffer == NULL) {
		return NULL;
	} else {
		buffer->bytesPerFrame = data->format.mBytesPerFrame;
		buffer->bufferSize = data->format.mBytesPerFrame * kMusicBufferSize;
		buffer->streamData = mmalloc(buffer->bufferSize);
		if (NULL == buffer->streamData) {
			mmfree(buffer);
			return NULL;
		}
		
		buffer->size = sizeof(struct MusicBuffer);
		AtomicExchange(&(buffer->atom_state), StateInit | StateStop);
		AtomicExchange(&(buffer->atom_volume), 65536);
		AtomicExchange(&(buffer->atom_loop), 0);
		AtomicExchange((int32_t*)&(buffer->atom_activeChannel), -1);
		buffer->totalSize = data->totalSize;
		buffer->handle = data->handle;
		buffer->totalFrames = data->totalFrames;
		buffer->list.mNumberBuffers = 1;
		buffer->list.mBuffers[0].mNumberChannels = data->format.mChannelsPerFrame;
		AtomicExchange(&(buffer->atom_currentFrame), 0);
		
		alGenBuffers(3, (ALuint*)&(buffer->sampleBuffers));
		
		SInt32 result;
		for (int i = 0; i < 3; i++) {
			AtomicExchange(&(buffer->atom_activeBuffers[i]), 1);
			result = fillSampleBuffer(buffer->sampleBuffers[i], buffer, false);
			if (result <= 0) {
				alDeleteBuffers(3, (ALuint*)&(buffer->sampleBuffers));
				mmfree((void*)(buffer->streamData));
				mmfree(buffer);
				if (result == 0) {
					void* audioHandle = mmallocSoundBuffer(data);
					if (audioHandle != NULL) {
						ExtAudioFileDispose(data->handle);
					}
					return audioHandle;
				} else {
					return NULL;
				}
			}
		}
		
		return buffer;
	}
}

void mmfreeAudioBuffer(void* handle) {
	if (handle == NULL) return;
	int64_t size = *((int64_t*)handle);
	if (size == sizeof(struct SoundBuffer)) {
		struct SoundBuffer* buffer = (struct SoundBuffer*)handle;
		alDeleteBuffers(1, (ALuint*)&(buffer->sampleBuffer));
		mmfree(buffer);
	} else if (size == sizeof(struct MusicBuffer)) {
		struct MusicBuffer* buffer = (struct MusicBuffer*)handle;
		alDeleteBuffers(3, (ALuint*)&(buffer->sampleBuffers));
		mmfree((void*)(buffer->streamData));
		ExtAudioFileDispose(buffer->handle);
		mmfree(buffer);
	}
}

int32_t findAudioBuffer(void* handle) {
	NSValue* val = [NSValue valueWithPointer:handle];
	NSUInteger idx = [audioBuffers indexOfObject:val];
	if (idx == NSNotFound)
		return -1;
	return (int32_t)idx;
}

bool shutdownAudioPlayer() {
	lockAudioPlayer();
	if (openALDevice == NULL) {
		unlockAudioPlayer();
		return true;
	}
	
	if (audioBuffers.count > 0) {
		unlockAudioPlayer();
		return false;
	}
	audioBuffers = nil;
	
	for (int i = (int)audioChannels.count - 1; i >= 0; i--) {
		NSNumber* num = [audioChannels objectAtIndex:i];
		unsigned int audioChannel = (unsigned int)[num unsignedIntegerValue];
		alSourceStop(audioChannel);
		alDeleteSources(1, &audioChannel);
	}
	[audioChannels removeAllObjects];
	audioChannels = nil;
	
	alcDestroyContext(openALContext);
	openALContext = NULL;
	
	alcCloseDevice(openALDevice);
	openALDevice = NULL;
	
	pauseAudioPlayer();
	
	unlockAudioPlayer();
	return true;
}

@interface iOSAudioPlayerThread : NSObject

+(void)prepareAudio:(NSValue*)prepareData;
+(void)playAudio:(NSValue*)audioData;

@end

@implementation iOSAudioPlayerThread

+(void)prepareAudio:(NSValue*)prepareData {
	struct PrepareAudioData* threadData = (struct PrepareAudioData*)[prepareData pointerValue];
	NSString* filePath = [NSString stringWithCString:threadData->filePath encoding:NSUTF8StringEncoding];
	
	struct AudioData* data = initAudioData(filePath);
	void* audioHandle = NULL;
	if (data == NULL) {
		threadData->onAudioError(threadData->userData);
	} else {
		if (data->totalSize < kMusicSize) {
			audioHandle = mmallocSoundBuffer(data);
			ExtAudioFileDispose(data->handle);
			if (audioHandle != NULL) {
				lockAudioPlayer();
				if (openALDevice == NULL) {
					unlockAudioPlayer();
					mmfreeAudioBuffer(audioHandle);
					audioHandle = NULL;
				} else {
					[audioBuffers addObject:[NSValue valueWithPointer:audioHandle]];
					unlockAudioPlayer();
					threadData->onAudioReady(audioHandle, (int32_t)(((struct SoundBuffer*)audioHandle)->totalSize), ((struct SoundBuffer*)audioHandle)->totalSize, threadData->userData);
				}
			}
		} else {
			audioHandle = mmallocMusicBuffer(data);
			if (audioHandle == NULL) {
				ExtAudioFileDispose(data->handle);
			} else {
				lockAudioPlayer();
				if (openALDevice == NULL) {
					unlockAudioPlayer();
					mmfreeAudioBuffer(audioHandle);
					audioHandle = NULL;
				} else {
					[audioBuffers addObject:[NSValue valueWithPointer:audioHandle]];
					unlockAudioPlayer();
					threadData->onAudioReady(audioHandle, (int32_t)(((struct MusicBuffer*)audioHandle)->bufferSize * 4), ((struct MusicBuffer*)audioHandle)->totalSize, threadData->userData);
				}
			}
		}
		mmfree(data);
		if (audioHandle == NULL) {
			threadData->onAudioError(threadData->userData);
		}
	}
	
	mmfree(threadData->filePath);
	mmfree(threadData);
}

void dequeueBuffers(ALuint channel) {
	alSourceStop(channel);
	
	/*
	 do {
	 ALint status;
	 alGetSourcei(channel, AL_SOURCE_STATE, &status);
	 if (status != AL_PLAYING) {
	 break;
	 }
	 
	 usleep(10000);
	 } while (true);
	 
	 ALint count;
	 alGetSourcei(channel, AL_BUFFERS_PROCESSED, &count);
	 while (count-- > 0) {
	 ALuint sampleBuffer;
	 alSourceUnqueueBuffers(channel, 1, &sampleBuffer);
	 }
	 
	 alGetSourcei(channel, AL_BUFFERS_QUEUED, &count);
	 while (count-- > 0) {
	 ALuint sampleBuffer;
	 alSourceUnqueueBuffers(channel, 1, &sampleBuffer);
	 }
	 */
	alSourcei(channel, AL_BUFFER, AL_NONE);
}

+(void)playAudio:(NSValue*)audioData {
	void* audioHandle = (void*)[audioData pointerValue];
	if (audioHandle == NULL) return;
	
	int64_t size = *((int64_t*)audioHandle);
	if (size == sizeof(struct SoundBuffer)) {
		struct SoundBuffer* buffer = (struct SoundBuffer*)audioHandle;
		ALuint activeChannel = AtomicCompareExchange((int32_t*)&(buffer->atom_activeChannel), 0, 0);
		do {
			bool requestPlay = false;
			if (audioStateCheckRequestAndWait((uint32_t*)&(buffer->atom_state), StateRelease, StateRelease)) {
				if (activeChannel != -1) {
					//NSLog(@"Release %d on %d", buffer->sampleBuffer, activeChannel);
					alSourceStop(activeChannel);
					alSourcei(activeChannel, AL_BUFFER, AL_NONE);
					alSourcei(activeChannel, AL_LOOPING, AL_FALSE);
					AtomicExchange((int32_t*)&(buffer->atom_activeChannel), -1);
					
					audioStateEndThread((uint32_t*)&(buffer->atom_state));
					
					lockAudioPlayer();
					[audioChannels addObject:[NSNumber numberWithUnsignedInt:activeChannel]];
					AtomicDecrement(&activeCount);
				} else {
					//NSLog(@"Release %d", buffer->sampleBuffer);
					
					audioStateEndThread((uint32_t*)&(buffer->atom_state));
					
					lockAudioPlayer();
				}
				
				int32_t index = findAudioBuffer(buffer);
				if (index >= 0) {
					mmfreeAudioBuffer(buffer);
					[audioBuffers removeObjectAtIndex:index];
				}
				unlockAudioPlayer();
				
				break;
			} else if (audioStateCheckRequestAndWait((uint32_t*)&(buffer->atom_state), StateStop, StateStop)) {
				if (activeChannel != -1) {
					//NSLog(@"Stop %d on %d", buffer->sampleBuffer, activeChannel);
					alSourceStop(activeChannel);
					alSourcei(activeChannel, AL_BUFFER, AL_NONE);
					alSourcei(activeChannel, AL_LOOPING, AL_FALSE);
					AtomicExchange((int32_t*)&(buffer->atom_activeChannel), -1);
					
					audioStateEndThread((uint32_t*)&(buffer->atom_state));
					
					lockAudioPlayer();
					[audioChannels addObject:[NSNumber numberWithUnsignedInt:activeChannel]];
					AtomicDecrement(&activeCount);
					unlockAudioPlayer();
				} else {
					audioStateEndThread((uint32_t*)&(buffer->atom_state));
				}
				break;
			} else if (audioStateCheckRequestAndWait((uint32_t*)&(buffer->atom_state), StatePause, StatePause)) {
				if (activeChannel != -1) {
					//NSLog(@"Pause %d on %d", buffer->sampleBuffer, activeChannel);
					alSourcePause(activeChannel);
					
					audioStateEndThread((uint32_t*)&(buffer->atom_state));
				} else {
					audioStateEndThread((uint32_t*)&(buffer->atom_state));
				}
				break;
			} else if (audioStateCheckPlay((uint32_t*)&(buffer->atom_state))) {
				if (audioStateCheckInit((uint32_t*)&(buffer->atom_state))) {
					alSourcef(activeChannel, AL_PITCH, 1.0f);
					alSourcef(activeChannel, AL_GAIN, AtomicCompareExchange(&(buffer->atom_volume), 0, 0) / 65536.0);
					alSourcei(activeChannel, AL_LOOPING, AtomicCompareExchange(&(buffer->atom_loop), 0, 0) != 0 ? AL_TRUE : AL_FALSE);
				}
				alSourcei(activeChannel, AL_BUFFER, buffer->sampleBuffer);
				requestPlay = true;
			}
			if (audioStateCheckUpdate((uint32_t*)&(buffer->atom_state))) {
				alSourcef(activeChannel, AL_GAIN, AtomicCompareExchange(&(buffer->atom_volume), 0, 0) / 65536.0);
				alSourcei(activeChannel, AL_LOOPING, AtomicCompareExchange(&(buffer->atom_loop), 0, 0) != 0 ? AL_TRUE : AL_FALSE);
			}
			if (requestPlay) {
				if (audioStateCheckRestart((uint32_t*)&(buffer->atom_state))) {
					alSourceStop(activeChannel);
				}
				alSourcePlay(activeChannel);
				//NSLog(@"Play %d on %d", buffer->sampleBuffer, activeChannel);
			}
			
			ALint status;
			alGetSourcei(activeChannel, AL_SOURCE_STATE, &status);
			if (status != AL_PLAYING) {
				//NSLog(@"AutoStop %d on %d", buffer->sampleBuffer, activeChannel);
				audioStateCheckRequestAndStop((uint32_t*)&(buffer->atom_state), StatePlay | StatePause);
				continue;
			}
			
			usleep(10000);
		} while (true);
		
	} else if (size == sizeof(struct MusicBuffer)) {
		struct MusicBuffer* buffer = (struct MusicBuffer*)audioHandle;
		ALuint activeChannel = AtomicCompareExchange((int32_t*)&(buffer->atom_activeChannel), 0, 0);
		bool ending = false;
		do {
			bool requestPlay = false;
			if (audioStateCheckRequestAndWait((uint32_t*)&(buffer->atom_state), StateRelease, StateRelease)) {
				if (activeChannel != -1) {
					//NSLog(@"Release %d,%d,%d on %d", buffer->sampleBuffers[0], buffer->sampleBuffers[1], buffer->sampleBuffers[2], activeChannel);
					dequeueBuffers(activeChannel);
					
					AtomicExchange(&(buffer->atom_activeBuffers[0]), 0);
					AtomicExchange(&(buffer->atom_activeBuffers[1]), 0);
					AtomicExchange(&(buffer->atom_activeBuffers[2]), 0);
					
					AtomicExchange((int32_t*)&(buffer->atom_activeChannel), -1);
					
					audioStateEndThread((uint32_t*)&(buffer->atom_state));
					
					lockAudioPlayer();
					[audioChannels addObject:[NSNumber numberWithUnsignedInt:activeChannel]];
					AtomicDecrement(&activeCount);
				} else {
					//NSLog(@"Release %d,%d,%d", buffer->sampleBuffers[0], buffer->sampleBuffers[1], buffer->sampleBuffers[2]);
					
					audioStateEndThread((uint32_t*)&(buffer->atom_state));
					
					lockAudioPlayer();
				}
				
				int32_t index = findAudioBuffer(buffer);
				if (index >= 0) {
					mmfreeAudioBuffer(buffer);
					[audioBuffers removeObjectAtIndex:index];
				}
				unlockAudioPlayer();
				
				break;
			} else if (audioStateCheckRequestAndWait((uint32_t*)&(buffer->atom_state), StateStop, StateStop)) {
				if (activeChannel != -1) {
					//NSLog(@"Stop %d,%d,%d on %d", buffer->sampleBuffers[0], buffer->sampleBuffers[1], buffer->sampleBuffers[2], activeChannel);
					dequeueBuffers(activeChannel);
					
					AtomicExchange(&(buffer->atom_activeBuffers[0]), 0);
					AtomicExchange(&(buffer->atom_activeBuffers[1]), 0);
					AtomicExchange(&(buffer->atom_activeBuffers[2]), 0);
					
					AtomicExchange((int32_t*)&(buffer->atom_activeChannel), -1);
					AtomicExchange(&(buffer->atom_currentFrame), 0);
					ExtAudioFileSeek(buffer->handle, 0);
					
					audioStateEndThread((uint32_t*)&(buffer->atom_state));
					
					lockAudioPlayer();
					[audioChannels addObject:[NSNumber numberWithUnsignedInt:activeChannel]];
					AtomicDecrement(&activeCount);
					unlockAudioPlayer();
				} else {
					//NSLog(@"Stop %d,%d,%d", buffer->sampleBuffers[0], buffer->sampleBuffers[1], buffer->sampleBuffers[2]);
					AtomicExchange(&(buffer->atom_currentFrame), 0);
					ExtAudioFileSeek(buffer->handle, 0);
					
					audioStateEndThread((uint32_t*)&(buffer->atom_state));
				}
				break;
			} else if (audioStateCheckRequestAndWait((uint32_t*)&(buffer->atom_state), StatePause, StatePause)) {
				if (activeChannel != -1) {
					//NSLog(@"Pause %d,%d,%d on %d", buffer->sampleBuffers[0], buffer->sampleBuffers[1], buffer->sampleBuffers[2], activeChannel);
					alSourcePause(activeChannel);
					
					audioStateEndThread((uint32_t*)&(buffer->atom_state));
				} else {
					audioStateEndThread((uint32_t*)&(buffer->atom_state));
				}
				break;
			} else if (audioStateCheckPlay((uint32_t*)&(buffer->atom_state))) {
				if (audioStateCheckInit((uint32_t*)&(buffer->atom_state))) {
					alSourcef(activeChannel, AL_PITCH, 1.0f);
					alSourcef(activeChannel, AL_GAIN, AtomicCompareExchange(&(buffer->atom_volume), 0, 0) / 65536.0);
					alSourcei(activeChannel, AL_LOOPING, AL_FALSE);
					
					//NSLog(@"Dequeue %d", activeChannel);
					dequeueBuffers(activeChannel);
					
					//NSLog(@"Queue %d,%d,%d on %d", buffer->sampleBuffers[0], buffer->sampleBuffers[1], buffer->sampleBuffers[2], activeChannel);
					alSourceQueueBuffers(activeChannel, 3, (ALuint*)(buffer->sampleBuffers));
				}
				requestPlay = true;
			}
			if (audioStateCheckUpdate((uint32_t*)&(buffer->atom_state))) {
				alSourcef(activeChannel, AL_GAIN, AtomicCompareExchange(&(buffer->atom_volume), 0, 0) / 65536.0);
			}
			
			ALint count;
			alGetSourcei(activeChannel, AL_BUFFERS_PROCESSED, &count);
			while (count > 0) {
				ALuint sampleBuffer = -1;
				alSourceUnqueueBuffers(activeChannel, 1, &sampleBuffer);
				//NSLog(@"Unqueue %d on %d", sampleBuffer, activeChannel);
				if (sampleBuffer == 0) {
					sleep(0);
				} else if (buffer->sampleBuffers[0] == sampleBuffer) {
					AtomicExchange(&(buffer->atom_activeBuffers[0]), 0);
				} else if (buffer->sampleBuffers[1] == sampleBuffer) {
					AtomicExchange(&(buffer->atom_activeBuffers[1]), 0);
				} else if (buffer->sampleBuffers[2] == sampleBuffer) {
					AtomicExchange(&(buffer->atom_activeBuffers[2]), 0);
				}
				count--;
			}
			
			if (audioStateCheckRestart((uint32_t*)&(buffer->atom_state))) {
				//NSLog(@"Dequeue %d", activeChannel);
				dequeueBuffers(activeChannel);
				
				AtomicExchange(&(buffer->atom_activeBuffers[0]), 0);
				AtomicExchange(&(buffer->atom_activeBuffers[1]), 0);
				AtomicExchange(&(buffer->atom_activeBuffers[2]), 0);
				
				AtomicExchange(&(buffer->atom_currentFrame), 0);
				ExtAudioFileSeek(buffer->handle, 0);
			}
			
			alGetSourcei(activeChannel, AL_BUFFERS_QUEUED, &count);
			int index = -1;
			bool loop = AtomicCompareExchange(&(buffer->atom_loop), 0, 0) != 0;
			if ((ending) && (loop)) {
				ending = false;
			}
			if ((count < 3) && (!ending)) {
				for (int i = 0; i < 3; i++) {
					if (AtomicCompareExchange(&(buffer->atom_activeBuffers[i]), 0, 0) == 0) {
						index = i;
						break;
					}
				}
				if (index == -1) {
					count++; // fake count for bussy buffers in other stopping thread
				}
			}
			if (index != -1) {
				SInt32 size = fillSampleBuffer(buffer->sampleBuffers[index], buffer, loop);
				if ((size == 0) && (loop)) {
					size = fillSampleBuffer(buffer->sampleBuffers[index], buffer, loop);
				}
				if (size == 0) {
					ending = true;
				} else {
					//NSLog(@"Queue %d on %d", buffer->sampleBuffers[index], activeChannel);
					alSourceQueueBuffers(activeChannel, 1, (ALuint*)&(buffer->sampleBuffers[index]));
					AtomicExchange(&(buffer->atom_activeBuffers[index]), 1);
					if (count == 0) {
						alSourcePlay(activeChannel);
					}
					count++;
				}
			}
			
			if (requestPlay) {
				alSourcePlay(activeChannel);
				//NSLog(@"Play %d,%d,%d on %d", buffer->sampleBuffers[0], buffer->sampleBuffers[1], buffer->sampleBuffers[2], activeChannel);
			}
			
			if (count == 0) {
				ALint status;
				alGetSourcei(activeChannel, AL_SOURCE_STATE, &status);
				if (status != AL_PLAYING) {
					//NSLog(@"AutoStop %d,%d,%d on %d", buffer->sampleBuffers[0], buffer->sampleBuffers[1], buffer->sampleBuffers[2], activeChannel);
					audioStateCheckRequestAndStop((uint32_t*)&(buffer->atom_state), StatePlay | StatePause);
					continue;
				}
			}
			
			usleep(10000);
		} while (true);
	}
}

@end

bool prepareAudio(NSString* filePath, void* userData, bool threaded, AudioReadyCallback onAudioReady, AudioErrorCallback onAudioError) {
	const char* src = filePath.UTF8String;
	int len = (int)strlen(src);
	struct PrepareAudioData* threadData = (struct PrepareAudioData*)mmalloc(sizeof(struct PrepareAudioData));
	if (threadData) {
		threadData->filePath = (char*)mmalloc(len+1);
		if (threadData->filePath) {
			memcpy(threadData->filePath, src, len+1);
			threadData->userData = userData;
			threadData->onAudioReady = onAudioReady;
			threadData->onAudioError = onAudioError;
			
			if (threaded) {
				[NSThread detachNewThreadSelector:@selector(prepareAudio:) toTarget:[iOSAudioPlayerThread class] withObject:[NSValue valueWithPointer:threadData]];
			} else {
				[iOSAudioPlayerThread prepareAudio:[NSValue valueWithPointer:threadData]];
			}
			return true;
		}
		mmfree(threadData);
	}
	return false;
}

void destroyAudio(void* audioHandle) {
	if (audioHandle == NULL) return;
	int64_t size = *((int64_t*)audioHandle);
	if (size == sizeof(struct SoundBuffer)) {
		struct SoundBuffer* buffer = (struct SoundBuffer*)audioHandle;
		if (audioStateCheckThreadAndRequest((uint32_t*)&(buffer->atom_state), StateStop | StatePlay | StatePause, StateRelease)) {
			[NSThread detachNewThreadSelector:@selector(playAudio:) toTarget:[iOSAudioPlayerThread class] withObject:[NSValue valueWithPointer:audioHandle]];
		}
	} else if (size == sizeof(struct MusicBuffer)) {
		struct MusicBuffer* buffer = (struct MusicBuffer*)audioHandle;
		if (audioStateCheckThreadAndRequest((uint32_t*)&(buffer->atom_state), StateStop | StatePlay | StatePause, StateRelease)) {
			[NSThread detachNewThreadSelector:@selector(playAudio:) toTarget:[iOSAudioPlayerThread class] withObject:[NSValue valueWithPointer:audioHandle]];
		}
	}
}

void playAudio(void* audioHandle, bool loop) {
	lockAudioPlayer();
	if (openALDevice == NULL) {
		unlockAudioPlayer();
		return;
	}
	unlockAudioPlayer();
	
	if (audioHandle == NULL) return;
	
	int64_t size = *((int64_t*)audioHandle);
	if (size == sizeof(struct SoundBuffer)) {
		struct SoundBuffer* buffer = (struct SoundBuffer*)audioHandle;
		
		bool thread = false;
		if (audioStateCheckThreadAndRequest((uint32_t*)&(buffer->atom_state), StateStop, StatePlay)) {
			ALuint channel = -1;
			lockAudioPlayer();
			if (audioChannels.count > 0) {
				NSNumber *num = [audioChannels objectAtIndex:0];
				[audioChannels removeObjectAtIndex:0];
				AtomicIncrement(&activeCount);
				channel = [num unsignedIntValue];
			}
			unlockAudioPlayer();
			AtomicExchange((int32_t*)&(buffer->atom_activeChannel), channel);
			
			thread = true;
		} else if (audioStateCheckThreadAndRequest((uint32_t*)&(buffer->atom_state), StatePause, StatePlay)) {
			thread = true;
		}
		if (AtomicExchange(&(buffer->atom_loop), loop ? 1 : 0) != (loop ? 1 : 0)) {
			audioStateUpdate((uint32_t*)&(buffer->atom_state));
		}
		if (thread) {
			[NSThread detachNewThreadSelector:@selector(playAudio:) toTarget:[iOSAudioPlayerThread class] withObject:[NSValue valueWithPointer:audioHandle]];
		}
	} else if (size == sizeof(struct MusicBuffer)) {
		struct MusicBuffer* buffer = (struct MusicBuffer*)audioHandle;
		
		bool thread = false;
		if (audioStateCheckThreadAndRequest((uint32_t*)&(buffer->atom_state), StateStop, StatePlay)) {
			ALuint channel = -1;
			lockAudioPlayer();
			if (audioChannels.count > 0) {
				NSNumber *num = [audioChannels objectAtIndex:0];
				[audioChannels removeObjectAtIndex:0];
				AtomicIncrement(&activeCount);
				channel = [num unsignedIntValue];
			}
			unlockAudioPlayer();
			AtomicExchange((int32_t*)&(buffer->atom_activeChannel), channel);
			
			thread = true;
		} else if (audioStateCheckThreadAndRequest((uint32_t*)&(buffer->atom_state), StatePause, StatePlay)) {
			thread = true;
		}
		if (AtomicExchange(&(buffer->atom_loop), loop ? 1 : 0) != (loop ? 1 : 0)) {
			audioStateUpdate((uint32_t*)&(buffer->atom_state));
		}
		if (thread) {
			[NSThread detachNewThreadSelector:@selector(playAudio:) toTarget:[iOSAudioPlayerThread class] withObject:[NSValue valueWithPointer:audioHandle]];
		}
	}
}

// returns true if playing
bool playingAudio(void* audioHandle) {
	if (audioHandle == NULL) return false;
	bool result = false;
	int64_t size = *((int64_t*)audioHandle);
	if (size == sizeof(struct SoundBuffer)) {
		struct SoundBuffer* buffer = (struct SoundBuffer*)audioHandle;
		result = audioStatePlaying((uint32_t*)&(buffer->atom_state));
	} else if (size == sizeof(struct MusicBuffer)) {
		struct MusicBuffer* buffer = (struct MusicBuffer*)audioHandle;
		result = audioStatePlaying((uint32_t*)&(buffer->atom_state));
	}
	return result;
}

void volumeAudio(void* audioHandle, float volume) {
	if (audioHandle == NULL) return;
	int64_t size = *((int64_t*)audioHandle);
	if (size == sizeof(struct SoundBuffer)) {
		struct SoundBuffer* buffer = (struct SoundBuffer*)audioHandle;
		AtomicExchange(&(buffer->atom_volume), volume * 65536.0);
		audioStateUpdate((uint32_t*)&(buffer->atom_state));
	} else if (size == sizeof(struct MusicBuffer)) {
		struct MusicBuffer* buffer = (struct MusicBuffer*)audioHandle;
		AtomicExchange(&(buffer->atom_volume), volume * 65536.0);
		audioStateUpdate((uint32_t*)&(buffer->atom_state));
	}
}

// returns true if pause requested
bool pauseAudio(void* audioHandle) {
	if (audioHandle == NULL) return false;
	int64_t size = *((int64_t*)audioHandle);
	if (size == sizeof(struct SoundBuffer)) {
		struct SoundBuffer* buffer = (struct SoundBuffer*)audioHandle;
		return audioStatePause((uint32_t*)&(buffer->atom_state));
	} else if (size == sizeof(struct MusicBuffer)) {
		struct MusicBuffer* buffer = (struct MusicBuffer*)audioHandle;
		return audioStatePause((uint32_t*)&(buffer->atom_state));
	}
	return false;
}

void stopAudio(void* audioHandle) {
	if (audioHandle == NULL) return;
	int64_t size = *((int64_t*)audioHandle);
	if (size == sizeof(struct SoundBuffer)) {
		struct SoundBuffer* buffer = (struct SoundBuffer*)audioHandle;
		if (audioStateCheckThreadAndRequest((uint32_t*)&(buffer->atom_state), StatePlay | StatePause, StateStop | StateRestart)) {
			[NSThread detachNewThreadSelector:@selector(playAudio:) toTarget:[iOSAudioPlayerThread class] withObject:[NSValue valueWithPointer:audioHandle]];
		}
	} else if (size == sizeof(struct MusicBuffer)) {
		struct MusicBuffer* buffer = (struct MusicBuffer*)audioHandle;
		if (audioStateCheckThreadAndRequest((uint32_t*)&(buffer->atom_state), StatePlay | StatePause, StateStop | StateRestart)) {
			[NSThread detachNewThreadSelector:@selector(playAudio:) toTarget:[iOSAudioPlayerThread class] withObject:[NSValue valueWithPointer:audioHandle]];
		}
	}
}

int32_t freeChannels() {
	int32_t result;
	lockAudioPlayer();
	result = (int32_t)audioChannels.count;
	unlockAudioPlayer();
	return result;
}

int32_t activeChannels() {
	return AtomicCompareExchange(&activeCount, 0, 0);
}
