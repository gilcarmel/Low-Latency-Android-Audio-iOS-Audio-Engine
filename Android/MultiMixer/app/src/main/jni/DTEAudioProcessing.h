//
//  DTEAudioProcessing.h
//  DetourEngine
//
//  Created by Syed Haris Ali on 12/18/14.
//  Copyright (c) Detour. All rights reserved.
//

#ifndef __DetourEngine__DTEAudioProcessing__
#define __DetourEngine__DTEAudioProcessing__

#include <stdio.h>
#include <stdlib.h>

typedef float*StereoBuffer;
typedef int64_t SInt64;
typedef uint32_t UInt32;

//------------------------------------------------------------------------------

typedef float (*FadeMappingProcess)(StereoBuffer stereoBuffer, StereoBuffer scratchBuffer, SInt64 framesProcessed, float startVolume, float endVolume, UInt32 numberOfFrames, UInt32 durationInFrames, UInt32 frameOffset);

//------------------------------------------------------------------------------

void ApplyGain(StereoBuffer audioBufferList,
               UInt32 numberOfFrames,
               UInt32 frameOffset,
               float *currentVolume,
               float gain);

//------------------------------------------------------------------------------

void Fade(StereoBuffer audioBufferList,
          float              *scratchBuffer,
          UInt32             numberOfFrames,
          float              startVolume,
          float              endVolume,
          SInt64             *framesProcessed,
          float              *currentVolume,
          UInt32             fadeDurationInFrames,
          FadeMappingProcess fadeMappingProcess);

//------------------------------------------------------------------------------

float MapExponentialFade(StereoBuffer audioBufferList,
                         float *scratchBuffer,
                         SInt64 framesProcessed,
                         float startVolume,
                         float endVolume,
                         UInt32 numberOfFrames,
                         UInt32 durationInFrames,
                         UInt32 frameOffset);

//------------------------------------------------------------------------------

float MapLinearFade(StereoBuffer audioBufferList,
                    float *scratchBuffer,
                    SInt64 framesProcessed,
                    float startVolume,
                    float endVolume,
                    UInt32 numberOfFrames,
                    UInt32 durationInFrames,
                    UInt32 frameOffset);

//------------------------------------------------------------------------------

void SilenceAudioBufferList(StereoBuffer *audioBufferList,
                            UInt32          numberOfFrames);

//------------------------------------------------------------------------------

float MapExponentialFadeSingleFrame(float  framesProcessed,
                                    float  startVolume,
                                    float  endVolume,
                                    SInt64 durationInFrames);

//------------------------------------------------------------------------------

float MapLinearFadeSingleFrame(float  framesProcessed,
                               float  startVolume,
                               float  endVolume,
                               SInt64 durationInFrames);

//------------------------------------------------------------------------------

#endif
