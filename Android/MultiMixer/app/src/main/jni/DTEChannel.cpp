//
// Created by Gil Carmel on 4/13/16.
//

#include <android/log.h>
#include "DTEChannel.h"


void DTEChannel::playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event,
                                     void *value) {
    DTEChannel *channel = (DTEChannel *) clientData;
    channel->onPlayerEvent(event, value);
}

DTEChannel::DTEChannel(unsigned int sampleRate, const char *path, int length) {
    player = new SuperpoweredAdvancedAudioPlayer(this, playerEventCallback, sampleRate, 0);
    player->open(path, 0, length);
    player->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_None;
    mLooping = false;
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
    //__android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Processing 0x%lx", (unsigned long) this);
    return player->process(stereoBuffer, !isSilenceSoFar, numSamples);
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
}

void DTEChannel::setLooping(bool isLooping) {
    mLooping = isLooping;
}

bool DTEChannel::isLooping() {
    return mLooping;
}
