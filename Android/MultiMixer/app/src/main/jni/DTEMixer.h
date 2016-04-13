#ifndef Header_MultiMixer
#define Header_MultiMixer

#include <math.h>
#include <pthread.h>
#include <map>

#include "DTEChannel.h"
#include "../../../../../../Superpowered/SuperpoweredAndroidAudioIO.h"

class DTEMixer {
public:

	DTEMixer(int *params);
	~DTEMixer();

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
    SuperpoweredAdvancedAudioPlayer *getPlayerForChannel(int id);
    DTEChannel *getChannel(int id);
    bool isLoopingNoMutex(int id);

private:
    pthread_mutex_t mutex;
    SuperpoweredAndroidAudioIO *audioSystem;
    float *stereoBuffer;
    unsigned int samplerate;
    std::map<int,DTEChannel*> channels;
    int nextId;
};

#endif
