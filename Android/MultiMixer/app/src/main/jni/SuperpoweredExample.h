#ifndef Header_SuperpoweredExample
#define Header_SuperpoweredExample

#include <math.h>
#include <pthread.h>

#include "SuperpoweredExample.h"
#include "../../../../../../Superpowered/SuperpoweredAdvancedAudioPlayer.h"
#include "../../../../../../Superpowered/SuperpoweredAndroidAudioIO.h"

#define NUM_BUFFERS 2
#define HEADROOM_DECIBEL 3.0f
static const float headroom = powf(10.0f, -HEADROOM_DECIBEL * 0.025);

class SuperpoweredExample {
public:

	SuperpoweredExample(int *params);
	~SuperpoweredExample();

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
