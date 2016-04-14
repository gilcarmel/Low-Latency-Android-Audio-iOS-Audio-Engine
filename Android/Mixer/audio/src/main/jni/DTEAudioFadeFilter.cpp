//
// Created by Gil Carmel on 4/13/16.
//

#include "DTEAudioFadeFilter.h"
#include <android/log.h>


DTEAudioFadeFilter::DTEAudioFadeFilter(float volume, float duckingVolume, UInt32 sampleRate) {
    _volume = volume;
    if (volume == 0) {
        _duckingVolume = 0;
    } else {
        _duckingVolume = duckingVolume / volume;
    }

    _currentVolume = volume;
    _currentDuckingVolume = 1.0;

    _fadeInCommand = (DTEFadeCommand *) calloc(1, sizeof(DTEFadeCommand));
    _fadeOutCommand = (DTEFadeCommand *) calloc(1, sizeof(DTEFadeCommand));
    _duckingCommand = (DTEFadeCommand *) calloc(1, sizeof(DTEFadeCommand));

    _sampleRate = sampleRate;
    _ducking = false;
    setCurrentTime(0.0);
}

DTEAudioFadeFilter::~DTEAudioFadeFilter() {
    free(_fadeInCommand);
    free(_fadeOutCommand);
    free(_duckingCommand);
}

double DTEAudioFadeFilter::getCurrentTime() {
    return ((double) _playhead) / _sampleRate;
}

void DTEAudioFadeFilter::setCurrentTime(double currentTime) {
    _playhead = (uint32_t) (currentTime * _sampleRate);
}

void DTEAudioFadeFilter::setFadeInAtStartTime(double startTime,
                                              double duration,
                                              DTEAudioFadeShape fadeShape) {
    //TODO: Remove "startTime" argument, or rework the module to support arbitrary start times
    _fadeInCommand->startTime = 0;
    _fadeInCommand->duration = duration;
    _fadeInCommand->durationInFrames = (UInt32) (duration * _sampleRate);
    _fadeInCommand->endVolume = _volume;
    _fadeInCommand->startVolume = 0.0f;
    _fadeInCommand->fadeShape = fadeShape;
}

void DTEAudioFadeFilter::setFadeOutAtStartTime(double startTime, double duration,
                                               DTEAudioFadeShape fadeShape) /*completion: (void (^)())completion */
{

//fadeOutCompletion = completion;
//_fadeOutCompletedCallbackScheduled = NO;

// Are we already in a fade out?
    double currentTime = getCurrentTime();
    if (currentTime >= _fadeOutCommand->startTime &&
        currentTime < _fadeOutCommand->startTime + _fadeOutCommand->duration) {
        return;
    }

    _fadeOutCommand->startTime = startTime;
    _fadeOutCommand->duration = duration;
    _fadeOutCommand->durationInFrames = (UInt32) (duration * _sampleRate);
    _fadeOutCommand->endVolume = 0.0f;
    _fadeOutCommand->startVolume = _currentVolume;
    _fadeOutCommand->fadeShape = fadeShape;
    _fadeOutCommand->framesProcessed = 0;
}

void DTEAudioFadeFilter::beginDuckingAtStartTime(double startTime, double duration,
                                                 DTEAudioFadeShape fadeShape) {
    //TODO: Remove startTime and use current time?
    // This doesn't truly support scheduling duck starts for the future (e.g. it interrupts
    // the current duck curve even if scheduled for later).
    _duckingCommand->startTime = startTime;
    _duckingCommand->duration = duration;
    _duckingCommand->durationInFrames = (UInt32) (_duckingCommand->duration * _sampleRate);
    _duckingCommand->endVolume = _duckingVolume;
    _duckingCommand->startVolume = _currentDuckingVolume;
    _duckingCommand->fadeShape = fadeShape;
    _duckingCommand->framesProcessed = 0;
    _ducking = true;
}

void DTEAudioFadeFilter::endDuckingAtStartTime(double startTime, double duration,
                                               DTEAudioFadeShape fadeShape) {
    //TODO: Remove startTime and use current time?
    // This doesn't truly support scheduling duck ends for the future (e.g. it interrupts
    // the current duck curve even if scheduled for later).
    _duckingCommand->startTime = startTime;
    _duckingCommand->duration = duration;
    _duckingCommand->durationInFrames = (UInt32) (_duckingCommand->duration * _sampleRate);
    _duckingCommand->endVolume = 1.0;
    _duckingCommand->startVolume = _currentDuckingVolume;
    _duckingCommand->fadeShape = fadeShape;
    _duckingCommand->framesProcessed = 0;
    _ducking = false;
}

//- (void) setupWithAudioController: (AEAudioController * ) audioController {
//    _sampleRate = audioController.audioDescription.mSampleRate;
//
//    // Update command durations with the sample rate
//    _fadeInCommand->
//    durationInFrames = _fadeInCommand->duration * _sampleRate;
//    _fadeOutCommand->
//    durationInFrames = _fadeOutCommand->duration * _sampleRate;
//    _duckingCommand->
//    durationInFrames = _duckingCommand->duration * _sampleRate;
//}

FadeMappingProcess fadeProcessForType(DTEAudioFadeShape fadeShape) {
    //TODO: implement exponential
    return &MapLinearFade;
//    FadeMappingProcess process;
//    switch (fadeShape) {
//        case DTEAudioFadeShapeLinear:
//            process = &MapLinearFade;
//            break;
//        case DTEAudioFadeShapeExponential:
//            process = &MapExponentialFade;
//            break;
//        default:
//            process = &MapExponentialFade;
//            break;
//    }
//    return process;
}

//bool resizeScratchBuffer(DTEAudioFadeFilter *filter, UInt32 frames) {
//    // Not supposed to call any allocs on the render thread, but this will almost never be true
//    if (filter->_scratchBufferLength < frames) {
//        float *tmpBuffer = realloc(filter->_scratchBuffer, sizeof(float) * frames);
//        if (tmpBuffer) {
//            filter->_scratchBufferLength = frames;
//            filter->_scratchBuffer = tmpBuffer;
//        } else {
//            // Realloc failed.
//            return false;
//        }
//    }
//    return true;
//}

//static void notifyFadeOutCompleted(void *userInfo, int length) {
//    DTEAudioFadeFilter *THIS = (__bridge DTEAudioFadeFilter*)*(void**)userInfo;
//    if ( THIS.fadeOutCompletion) {
//        THIS.fadeOutCompletion();
//        THIS.fadeOutCompletion = nil;
//    }
//}

void DTEAudioFadeFilter::applyFadeCommand(DTEFadeCommand *command, StereoBuffer stereoBuffer,
                                          UInt32 numberOfFrames, bool duckingCommand) {
    Fade(stereoBuffer,
         NULL,
         numberOfFrames,
         command->startVolume,
         command->endVolume,
         &(command->framesProcessed),
         duckingCommand ? &_currentDuckingVolume : &_currentVolume,
         command->durationInFrames,
         fadeProcessForType(command->fadeShape));
}

#pragma mark - Audio Render Callback

bool DTEAudioFadeFilter::process(float *stereoBuffer, unsigned int frames) {
    uint32_t playhead = _playhead;

    DTEFadeCommand *fadeIn = _fadeInCommand;
    UInt32 fadeInStartFrame = (UInt32) (fadeIn->startTime * _sampleRate);
    UInt32 fadeInEndFrame = fadeInStartFrame + fadeIn->durationInFrames;

    DTEFadeCommand *fadeOut = _fadeOutCommand;
    UInt32 fadeOutStartFrame = (UInt32) (fadeOut->startTime * _sampleRate);

    DTEFadeCommand *ducking = _duckingCommand;
    UInt32 duckingStartFrame = (UInt32) (ducking->startTime * _sampleRate);
    UInt32 duckingEndFrame = duckingStartFrame + ducking->durationInFrames;

    bool inDuckingCurve = (playhead + frames >= duckingStartFrame && playhead < duckingEndFrame);
    //TODO: Steve - do this on iOS too, so that setFadeOutAtStartTime and setFadeInAtStartTime are not manadatory?
    //fadeIn and fadeOut are designed as on-shot effecs (non-repeatable). I guess that's OK for Detour?
    bool shouldFadeOut = fadeOut->durationInFrames > 0 && (playhead + frames >= fadeOutStartFrame);
    bool shouldFadeIn = fadeIn->durationInFrames > 0 && (playhead < fadeInEndFrame);

    if (shouldFadeIn) {
        //__android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "shouldFadeIn");
        // Process the fade-in command
        fadeIn->endVolume = _volume;
        fadeIn->framesProcessed = playhead - fadeInStartFrame;
        applyFadeCommand(fadeIn, stereoBuffer, frames, false);

    } else if (shouldFadeOut) {
        //__android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "shouldFadeOut");
        // Process the fade-out command
        fadeOut->startVolume = _volume;
        fadeOut->framesProcessed = (SInt64) playhead - (SInt64) fadeOutStartFrame;
        applyFadeCommand(fadeOut, stereoBuffer, frames, false);
    } else {
        //__android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "shouldn't fade");
        _currentVolume = _volume;
        ApplyGain(stereoBuffer, frames, 0, &_currentVolume, _currentVolume);
    }

    // Ducking is independent of fading in and fading out
    if (inDuckingCurve) {
        //__android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "inDuckingCurve");
        applyFadeCommand(ducking, stereoBuffer, frames, true);
    } else {
        //__android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "not in ducking curve");
        _currentDuckingVolume = _ducking ? _duckingVolume : 1.0f;
        ApplyGain(stereoBuffer, frames, 0, &_currentDuckingVolume, _currentDuckingVolume);
    }

    // Advance playhead
    playhead += frames;
    _playhead = playhead;

    //Returns true if fade out is done
    return (playhead >= fadeOutStartFrame + fadeOut->durationInFrames);
}
