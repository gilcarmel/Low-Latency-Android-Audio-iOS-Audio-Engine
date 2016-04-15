//
// Created by Gil Carmel on 4/13/16.
//

#include <android/log.h>
#include <SuperpoweredSimple.h>
#include "DTEChannel.h"

void DTEChannel::playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event,
                                     void *value) {
    DTEChannel *channel = (DTEChannel *) clientData;
    channel->onPlayerEvent(event, value);
}

DTEChannel::DTEChannel(unsigned int sampleRate, const char *path, int length, float duckingVolume)
        : fadeFilter(1.0, duckingVolume, sampleRate) {
    player = new SuperpoweredAdvancedAudioPlayer(this, playerEventCallback, sampleRate, 0);
    player->open(path, 0, length);
    player->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_None;
    mLooping = false;
    scratchBuffer = NULL;
    scratchBufferSamples = 0;
}

DTEChannel::~DTEChannel() {
    delete player;
}

SuperpoweredAdvancedAudioPlayer *DTEChannel::getPlayer() {
    return player;
}

void DTEChannel::onPlayerEvent(SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
        __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Player 0x%lx loaded",
                            (unsigned long) this);
    }
    else if (event == SuperpoweredAdvancedAudioPlayerEvent_EOF) {
        __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Player 0x%lx EOF",
                            (unsigned long) this);
        if (player && !mLooping) {
            __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer",
                                "Player 0x%lx not looping, so pausing on EOF",
                                (unsigned long) this);
            player->pause();
        }
    }

}

/**
 * Adds channel output to a stereo buffer.
 */
bool DTEChannel::process(float *stereoBuffer, bool isSilenceSoFar, unsigned int numSamples) {
    bool hasAudio;
    //Process directly into output buffer
    if (isSilenceSoFar) {
        hasAudio = player->process(stereoBuffer, false, numSamples);
        if (hasAudio) {
            fadeFilter.process(stereoBuffer, numSamples);
        }
    }
    //Process in scratch buffer
    else {
        if (scratchBufferSamples < numSamples) {
            allocateScratchBuffer(numSamples);
        }
        hasAudio = player->process(scratchBuffer, false, numSamples);
        if (hasAudio) {
            fadeFilter.process(scratchBuffer, numSamples);
            //Add scratch buffer to output buffer
            SuperpoweredVolumeAdd(scratchBuffer, stereoBuffer, 1.0f, 1.0f, numSamples);
        }
    }
    return hasAudio;
}

void DTEChannel::play() {
    player->play(false);
}

void DTEChannel::pause() {
    player->pause();
}

bool DTEChannel::isPlaying() {
    return player->playing;
}

unsigned int DTEChannel::getDurationMS() {
    return player->durationMs;
}

unsigned int DTEChannel::getPositionMS() {
    return (unsigned int) player->positionMs;
}

void DTEChannel::setPosition(unsigned int milliseconds) {
    player->setPosition((double) milliseconds, false, false);
    fadeFilter.setCurrentTime((double)milliseconds/1000.0);

}

void DTEChannel::setLooping(bool isLooping) {
    mLooping = isLooping;
}

bool DTEChannel::isLooping() {
    return mLooping;
}

void DTEChannel::allocateScratchBuffer(unsigned int numSamples) {
    if (scratchBuffer) {
        delete[] scratchBuffer;
    }
    scratchBuffer = new float[numSamples * 2 + 64 / sizeof(float)];  //Superpowered wants 64 extra bytes
    scratchBufferSamples = numSamples;
}

bool DTEChannel::fadeOut(double startTime, double duration, DTEAudioFadeShape fadeShape) {
    fadeFilter.setFadeOutAtStartTime(startTime, duration, fadeShape);
    return true;
}

bool DTEChannel::fadeIn(double startTime, double duration, DTEAudioFadeShape fadeShape) {
    fadeFilter.setFadeInAtStartTime(startTime, duration, fadeShape);
    return true;
}

bool DTEChannel::beginDucking(double startTime, double duration, DTEAudioFadeShape fadeShape) {
    fadeFilter.beginDuckingAtStartTime(startTime, duration, fadeShape);
    return true;
}

bool DTEChannel::endDucking(double startTime, double duration, DTEAudioFadeShape fadeShape) {
    fadeFilter.endDuckingAtStartTime(startTime, duration, fadeShape);
    return true;
}
