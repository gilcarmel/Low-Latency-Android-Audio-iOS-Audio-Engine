//
// Created by Gil Carmel on 4/13/16.
//

#ifndef MULTIMIXER_DTEAUDIOFADEFILTER_H
#define MULTIMIXER_DTEAUDIOFADEFILTER_H

#include "DTEAudioProcessing.h"

typedef enum DTEAudioFadeShape {
    DTEAudioFadeShapeLinear = 0,
    DTEAudioFadeShapeExponential = 1
} DTEAudioFadeShape;

typedef struct DTEFadeCommand {
    double startTime;
    double duration;
    UInt32 durationInFrames;
    float startVolume;
    float endVolume;
    DTEAudioFadeShape fadeShape;
    SInt64 framesProcessed;
} DTEFadeCommand;

class DTEAudioFadeFilter {
private:
    float _volume;
    float _duckingVolume;
    float _currentVolume;
    float _currentDuckingVolume;
    DTEFadeCommand *_fadeInCommand;
    DTEFadeCommand *_fadeOutCommand;
    DTEFadeCommand *_duckingCommand;
public:
    DTEAudioFadeFilter(float volume, float duckingVolume, UInt32 sampleRate);

    virtual ~DTEAudioFadeFilter();

    double getCurrentTime();

    uint32_t _playhead;
    double _sampleRate;

    void setCurrentTime(double currentTime);

    void setFadeInAtStartTime(double startTime, double duration, DTEAudioFadeShape fadeShape);

    void setFadeOutAtStartTime(double startTime, double duration, DTEAudioFadeShape fadeShape);

    void beginDuckingAtStartTime(double startTime, double duration, DTEAudioFadeShape fadeShape);

    int _ducking;

    void endDuckingAtStartTime(double startTime, double duration, DTEAudioFadeShape fadeShape);

    bool process(float *stereoBuffer, unsigned int numSamples);

    void applyFadeCommand(DTEFadeCommand *command, StereoBuffer stereoBuffer, UInt32 numberOfFrames,
                          bool duckingCommand);
};


#endif //MULTIMIXER_DTEAUDIOFADEFILTER_H
