#ifndef Header_MultiMixer
#define Header_MultiMixer

#include <math.h>
#include <pthread.h>
#include <map>

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
	int prepare(const char* path, int length);
    bool close(int id);
    bool play(int id);
    bool pause(int id);
    bool isPlaying(int id);
    unsigned int getDuration(int id);
    unsigned int getPosition(int id);
    bool seek(int id, unsigned int milliseconds);
    bool setLooping(int id, bool looping);
    bool isLooping(int id);
    SuperpoweredAdvancedAudioPlayer* getPlayer(int id);
    bool isLoopingNoMutex(int id);

private:
    pthread_mutex_t mutex;
    SuperpoweredAndroidAudioIO *audioSystem;
    float *stereoBuffer;
    unsigned int samplerate;
    std::map<int,SuperpoweredAdvancedAudioPlayer*> players;
    std::map<int,bool> mLooping;
    int nextId;
};

#endif
