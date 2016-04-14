//
// Created by Gil Carmel on 4/13/16.
//

//
//  DTEAudioProcessing.c
//  DetourEngine
//
//  Created by Syed Haris Ali on 12/18/14.
//  Copyright (c) 2014 Detour. All rights reserved.
//

#include <SuperpoweredSimple.h>
#include "DTEAudioProcessing.h"

const float DTEAudioProcessingExponentialFadeBase = 128.0f;
const float DTEAudioProcessingExponentialFadeBaseLog = 4.85203026392;  // log(DTEAudioProcessingExponentialFadeBase)
const float DTEAudioProcessingExponentialFadeMultiplier = 1.0f / (DTEAudioProcessingExponentialFadeBase - 1.0f);

//------------------------------------------------------------------------------

void ApplyGain(StereoBuffer stereoBuffer,
               UInt32 numberOfFrames,
               UInt32 frameOffset,
               float *currentVolume,
               float gain)
{
    if (gain == 1.0 && *currentVolume == 1.0) {
        return;
    }

    SuperpoweredVolume(stereoBuffer, stereoBuffer, gain, gain, numberOfFrames);
    *currentVolume = gain;
}

//------------------------------------------------------------------------------

void Fade(StereoBuffer stereoBuffer,
          float *scratchBuffer,
          UInt32 numberOfFrames,
          float startVolume,
          float endVolume,
          SInt64 *framesProcessed,
          float *currentVolume,
          UInt32 fadeDurationInFrames,
          FadeMappingProcess fadeMappingProcess)
{
    if (*framesProcessed >= fadeDurationInFrames)
    {
        ApplyGain(stereoBuffer, numberOfFrames, 0, currentVolume, endVolume);
        return;
    }

    int framesBeforeFade = *framesProcessed < 0 ? (int)(-(*framesProcessed)) : 0;

    // change gain for the frames before the fade
    if (framesBeforeFade > 0)
    {
        ApplyGain(stereoBuffer, (UInt32) framesBeforeFade, 0, currentVolume, startVolume);
        *framesProcessed += framesBeforeFade;
    }

    int framesRemaining = numberOfFrames - framesBeforeFade;
    int framesRemainingInTotalFade = fadeDurationInFrames - (int)*framesProcessed;

    int fadeFramesToProcess = framesRemaining < framesRemainingInTotalFade ? framesRemaining : framesRemainingInTotalFade;

    // perform the actual fade
    if (fadeFramesToProcess > 0)
    {
        *currentVolume = fadeMappingProcess(stereoBuffer, scratchBuffer, *framesProcessed, startVolume, endVolume,
                                            (UInt32) fadeFramesToProcess, fadeDurationInFrames,
                                            (UInt32) framesBeforeFade);
        *framesProcessed += fadeFramesToProcess;
    }

    int framesAfterFade = numberOfFrames - (framesBeforeFade + fadeFramesToProcess);

    // change gain for the frames after the fade
    if (fadeFramesToProcess < framesRemaining)
    {
        ApplyGain(stereoBuffer, framesAfterFade, framesBeforeFade + fadeFramesToProcess, currentVolume, endVolume);
    }
}

//------------------------------------------------------------------------------
//TODO:
//float MapExponentialFade(StereoBuffer stereoBuffer,
//                         float *scratchBuffer,
//                         SInt64 framesProcessed,
//                         float startVolume,
//                         float endVolume,
//                         UInt32 numberOfFrames,
//                         UInt32 durationInFrames,
//                         UInt32 frameOffset)
//{
//    bool isFadeOut = endVolume < startVolume;
//
//    if (isFadeOut)
//    {
//        float tmp = startVolume;
//        startVolume = endVolume;
//        endVolume = tmp;
//
//        framesProcessed = durationInFrames - (framesProcessed + numberOfFrames);
//    }
//
//    float finalFrameVolume = 0;
//    float rampStart, rampEnd, multiplier;
//    float *buffer;
//    int intNumberOfFrames = (int)numberOfFrames;
//
//    for (int i = 0; i < stereoBuffer->mNumberBuffers; i++)
//    {
//        rampStart = framesProcessed * DTEAudioProcessingExponentialFadeBaseLog / ((float)durationInFrames - 1.0f);
//        rampEnd = (framesProcessed + numberOfFrames - 1.0f) * DTEAudioProcessingExponentialFadeBaseLog / ((float)durationInFrames - 1.0f);
//
//        multiplier = DTEAudioProcessingExponentialFadeMultiplier * (endVolume - startVolume);
//
//        vDSP_vclr(scratchBuffer, 1, numberOfFrames);
//
//        // scratch buffer is a linear ramp from
//        //
//        //   framesProcessed * log(128)         (framesProcessed + numberOfFrames - 1) * log(128)
//        //   --------------------------  ---->  -------------------------------------------------
//        //      durationInFrames - 1                           durationInFrames - 1
//        //
//        vDSP_vgen(&rampStart, &rampEnd, scratchBuffer, 1, numberOfFrames);
//
//        // Replaces each x in the scratch buffer with
//        //
//        //    x
//        //   e  - 1
//        //
//        //                   (framesProcessed / (durationInFrames - 1))
//        // so x[0]' = -1 + 128
//        //
//        // etc.
//        vvexpm1f(scratchBuffer, scratchBuffer, &intNumberOfFrames);
//
//        // x'' = x' * (endVolume - startVolume)
//        //       ------------------------------  +  startVolume
//        //                   128 - 1
//        vDSP_vsmsa(scratchBuffer, 1, &multiplier, &startVolume, scratchBuffer, 1, numberOfFrames);
//
//        if (isFadeOut)
//        {
//            vDSP_vrvrs(scratchBuffer, 1, numberOfFrames);
//        }
//
//        // Scratch buffer is the volume curve, which we then multiply element-wise by the audio buffer
//        buffer = (float *)(stereoBuffer->mBuffers[i].mData) + frameOffset;
//        vDSP_vmul(buffer, 1, scratchBuffer, 1, buffer, 1, numberOfFrames);
//
//        finalFrameVolume = scratchBuffer[numberOfFrames - 1];
//    }
//
//    return finalFrameVolume;
//}

//------------------------------------------------------------------------------

float MapLinearFade(StereoBuffer stereoBuffer,
                    float *scratchBuffer,
                    SInt64 framesProcessed,
                    float startVolume,
                    float endVolume,
                    UInt32 numberOfFrames,
                    UInt32 durationInFrames,
                    UInt32 frameOffset)
{
    float initialVolume = 0.0f;
    float finalVolume = 0.0f;
    initialVolume = (endVolume - startVolume) * framesProcessed / (float)durationInFrames + startVolume;
    finalVolume = initialVolume + ((endVolume - startVolume) / (float)durationInFrames) * numberOfFrames;
    SuperpoweredVolume(stereoBuffer, stereoBuffer, initialVolume, finalVolume, numberOfFrames);

    return finalVolume;
}

//------------------------------------------------------------------------------

//void SilencestereoBuffer(StereoBuffer stereoBuffer, UInt32 numberOfFrames)
//{
//    for (int i = 0; i < stereoBuffer->mNumberBuffers; i++)
//    {
//        vDSP_vclr(stereoBuffer->mBuffers[i].mData, 1, numberOfFrames);
//    }
//}

//------------------------------------------------------------------------------

//float MapExponentialFadeSingleFrame(float  framesProcessed,
//                                    float  startVolume,
//                                    float  endVolume,
//                                    SInt64 durationInFrames)
//{
//    float exponential = framesProcessed / (float) (durationInFrames - 1.0f);
//    float volume;
//    if (endVolume - startVolume > 0)
//    {
//        // fade in
//        volume = ((powf(DTEAudioProcessingExponentialFadeBase, exponential) - 1.0f) *
//                  DTEAudioProcessingExponentialFadeMultiplier * (endVolume - startVolume) +
//                  startVolume);
//    }
//    else
//    {
//        // fade out
//        volume = ((powf(DTEAudioProcessingExponentialFadeBase, 1.0f - exponential) - 1.0f) *
//                  DTEAudioProcessingExponentialFadeMultiplier * (startVolume - endVolume)
//                  + endVolume);
//    }
//    return volume;
//}

//------------------------------------------------------------------------------

//float MapLinearFadeSingleFrame(float  framesProcessed,
//                               float  startVolume,
//                               float  endVolume,
//                               SInt64 durationInFrames)
//{
//    float amount = framesProcessed / (float) durationInFrames;
//    return (endVolume - startVolume) * amount + startVolume;
//}

