#include "MultiMixer.h"
#include "SuperpoweredSimple.h"
#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>


typedef std::map<int,SuperpoweredAdvancedAudioPlayer*>::iterator it_type;

static void playerEventCallbackA(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
    	SuperpoweredAdvancedAudioPlayer *player = *((SuperpoweredAdvancedAudioPlayer **)clientData);
//        playerA->setBpm(126.0f);
//        playerA->setFirstBeatMs(353);
//        playerA->setPosition(playerA->firstBeatMs, false, false);
        __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Player 0x%lx loaded", (long)player);
    };
}


static bool audioProcessing(void *clientdata, short int *audioIO, int numberOfSamples, int samplerate) {
    //__android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Processing");
	return ((MultiMixer *)clientdata)->process(audioIO, numberOfSamples);
}

MultiMixer::MultiMixer(int *params) : nextId(1) {
    pthread_mutex_init(&mutex, NULL); // This will keep our player volumes and playback states in sync.
    samplerate = params[0];
    unsigned int buffersize = params[1];
    stereoBuffer = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);

    audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioProcessing, this, -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2);
}

MultiMixer::~MultiMixer() {
    delete audioSystem;
    free(stereoBuffer);
    pthread_mutex_destroy(&mutex);
}

int MultiMixer::prepare(const char* path, int length) {
    pthread_mutex_lock(&mutex);
    __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Preparing %s. Length=%d", path, length);
    SuperpoweredAdvancedAudioPlayer* player = new SuperpoweredAdvancedAudioPlayer(&player , playerEventCallbackA, samplerate, 0);
    player->open(path, 0, length);
    player->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_None;

    int id = nextId++;
    players[id]=player;

    pthread_mutex_unlock(&mutex);

    return id;
}

bool MultiMixer::play(int id) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    if (players.count(id) == 0) {
        __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Invalid player ID %d.", id);
    }
    else {
        __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Playing %d.", id);
        SuperpoweredAdvancedAudioPlayer* player = players[id];
        player->play(false);
        result = true;
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

bool MultiMixer::process(short int *output, unsigned int numberOfSamples) {
    pthread_mutex_lock(&mutex);

    bool silence = true;
    for(it_type iterator = players.begin(); iterator != players.end(); iterator++) {
        SuperpoweredAdvancedAudioPlayer* player = iterator->second;
        if (player->process(stereoBuffer, !silence, numberOfSamples)) {
            silence = false;
        }
    }

    pthread_mutex_unlock(&mutex);

    // The stereoBuffer is ready now, let's put the finished audio into the requested buffers.
    if (!silence) SuperpoweredFloatToShortInt(stereoBuffer, output, numberOfSamples);
    return !silence;
}

extern "C" {
	JNIEXPORT void Java_com_superpowered_multimixer_MultiMixer__1create(JNIEnv *javaEnvironment, jobject self, jlongArray offsetAndLength);
	JNIEXPORT jlong Java_com_superpowered_multimixer_MultiMixer__1prepare(JNIEnv *javaEnvironment, jobject self, jstring jpath, jlong length);
	JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1play(JNIEnv *javaEnvironment, jobject self, jlong id);
}

static MultiMixer *mixer = NULL;

// Android is not passing more than 2 custom parameters, so we had to pack file offsets and lengths into an array.
JNIEXPORT void Java_com_superpowered_multimixer_MultiMixer__1create(JNIEnv *javaEnvironment, jobject self, jlongArray params) {
    __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Initializing MultiMixer");

	// Convert the input jlong array to a regular int array.
    jlong *longParams = javaEnvironment->GetLongArrayElements(params, JNI_FALSE);
    int arr[2];
    for (int n = 0; n < 2; n++) arr[n] = longParams[n];
    javaEnvironment->ReleaseLongArrayElements(params, longParams, JNI_ABORT);

    mixer  = new MultiMixer(arr);
}

JNIEXPORT jlong Java_com_superpowered_multimixer_MultiMixer__1prepare(JNIEnv *javaEnvironment, jobject self, jstring jpath, jlong length) {
    const char *path = javaEnvironment->GetStringUTFChars(jpath, JNI_FALSE);
	int id = mixer->prepare(path, length);
    javaEnvironment->ReleaseStringUTFChars(jpath, path);
    return id;
}

JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1play(JNIEnv *javaEnvironment, jobject self, jlong id) {
	return mixer->play(id);

}
