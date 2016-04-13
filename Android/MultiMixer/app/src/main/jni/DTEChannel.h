//
// Created by Gil Carmel on 4/13/16.
//

#ifndef MULTIMIXER_DTECHANNEL_H
#define MULTIMIXER_DTECHANNEL_H

#include "../../../../../../Superpowered/SuperpoweredAdvancedAudioPlayer.h"
#include "DTEAudioFadeFilter.h"

//An audio channel consisting of a SimpleAudioPlayer going through a fade/duck filter
class DTEChannel {
public:
    DTEChannel(unsigned int sampleRate, const char *path, int length);

    virtual ~DTEChannel();

    SuperpoweredAdvancedAudioPlayer *getPlayer();

    void onPlayerEvent(SuperpoweredAdvancedAudioPlayerEvent event, void *value);

    bool process(float *stereoBuffer, bool isSilenceSoFar, unsigned int numSamples);

    void play();

    void pause();

    bool isPlaying();

    unsigned int getDurationMS();

    unsigned int getPositionMS();

    void setPosition(unsigned int milliseconds);

    void setLooping(bool isLooping);

    bool isLooping();
private:
    SuperpoweredAdvancedAudioPlayer *player;
    DTEAudioFadeFilter fadeFilter;
    bool mLooping;
    float *scratchBuffer;
    UInt32 scratchBufferSamples;

    void allocateScratchBuffer(unsigned int numSamples);

    static void playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event,
                                    void *value);
};


#endif //MULTIMIXER_DTECHANNEL_H
