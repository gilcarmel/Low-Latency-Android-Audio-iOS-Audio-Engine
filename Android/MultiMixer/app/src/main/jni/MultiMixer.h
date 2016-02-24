#ifndef Header_MultiMixer
#define Header_MultiMixer

#include <math.h>
#include <pthread.h>

#include "../../../../../../Superpowered/SuperpoweredAdvancedAudioPlayer.h"
#include "../../../../../../Superpowered/SuperpoweredAndroidAudioIO.h"

#define NUM_BUFFERS 2
#define HEADROOM_DECIBEL 3.0f
static const float headroom = powf(10.0f, -HEADROOM_DECIBEL * 0.025);

class MultiMixer {
public:

	MultiMixer(int *params);
	~MultiMixer();

	bool process(short int *output, unsigned int numberOfSamples);
	void play(const char* path, int length);

private:
    pthread_mutex_t mutex;
    SuperpoweredAndroidAudioIO *audioSystem;
    SuperpoweredAdvancedAudioPlayer *playerA, *playerB;
    float *stereoBuffer;
    unsigned int samplerate;
};

#endif
