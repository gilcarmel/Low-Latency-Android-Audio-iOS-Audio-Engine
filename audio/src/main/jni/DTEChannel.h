//
// Created by Gil Carmel on 4/13/16.
//

#ifndef MULTIMIXER_DTECHANNEL_H
#define MULTIMIXER_DTECHANNEL_H

#include "SuperpoweredAdvancedAudioPlayer.h"
#include "DTEAudioFadeFilter.h"

//An audio channel consisting of a SimpleAudioPlayer going through a fade/duck filter
class DTEChannel {
public:
    DTEChannel(unsigned int sampleRate, const char *path, int length, float volume, float duckingVolume);

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

    bool fadeOut(double startTime, double duration, DTEAudioFadeShape fadeShape);

    bool fadeIn(double startTime, double duration, DTEAudioFadeShape fadeShape);

    bool beginDucking(double startTime, double duration, DTEAudioFadeShape fadeShape);

    bool endDucking(double startTime, double duration, DTEAudioFadeShape fadeShape);

    void setVolume(float volume);

    void setRegionDuration(double duration);

    void setRegionStartTime(double startTime);

private:
    SuperpoweredAdvancedAudioPlayer *player;
    DTEAudioFadeFilter fadeFilter;
    bool mLooping;
    float *scratchBuffer;
    UInt32 scratchBufferSamples;

    void allocateScratchBuffer(unsigned int numSamples);

    static void playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event,
                                    void *value);

    double startOffset;
};


#endif //MULTIMIXER_DTECHANNEL_H
