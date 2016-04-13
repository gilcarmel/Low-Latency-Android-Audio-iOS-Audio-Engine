//
// Created by Gil Carmel on 4/13/16.
//

#ifndef MULTIMIXER_DTECHANNEL_H
#define MULTIMIXER_DTECHANNEL_H

#include "../../../../../../Superpowered/SuperpoweredAdvancedAudioPlayer.h"

//An audio channel consisting of a SimpleAudioPlayer plus a combined fade/duck filter
class DTEChannel {
    SuperpoweredAdvancedAudioPlayer *player;
    bool mLooping;
public:
    SuperpoweredAdvancedAudioPlayer *getPlayer();

    void onPlayerEvent(SuperpoweredAdvancedAudioPlayerEvent event, void *value);

    DTEChannel(unsigned int sampleRate, const char *path, int length);

    virtual ~DTEChannel();

    static void playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event,
                                    void *value);

    bool process(float *stereoBuffer, bool isSilenceSoFar, unsigned int numSamples);

    void play();

    void pause();

    bool isPlaying();

    unsigned int getDurationMS();

    unsigned int getPositionMS();

    void setPosition(unsigned int milliseconds);

    void setLooping(bool isLooping);

    bool isLooping();
};


#endif //MULTIMIXER_DTECHANNEL_H
