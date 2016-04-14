#ifndef Header_MultiMixer
#define Header_MultiMixer

#include <math.h>
#include <pthread.h>
#include <map>

#include "DTEChannel.h"
#include "SuperpoweredAndroidAudioIO.h"

class DTEMixer {
public:

	DTEMixer(unsigned int buffersize, unsigned int sampleRate);
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
    DTEChannel *getChannel(int id);

	bool fadeOut(int id, double startTime, double duration, DTEAudioFadeShape fadeShape);

	bool fadeIn(int id, double startTime, double duration, DTEAudioFadeShape fadeShape);

private:
    pthread_mutex_t mutex;
    SuperpoweredAndroidAudioIO *audioSystem;
    float *stereoBuffer;
    unsigned int mSampleRate;
    std::map<int,DTEChannel*> channels;
    int nextId;
};

#endif
