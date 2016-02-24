#include "MultiMixer.h"
#include "SuperpoweredSimple.h"
#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>

static void playerEventCallbackA(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
    	SuperpoweredAdvancedAudioPlayer *playerA = *((SuperpoweredAdvancedAudioPlayer **)clientData);
        playerA->setBpm(126.0f);
        playerA->setFirstBeatMs(353);
        playerA->setPosition(playerA->firstBeatMs, false, false);
        __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Player 0x%x loaded", playerA);
    };
}

static void playerEventCallbackB(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
    	SuperpoweredAdvancedAudioPlayer *playerB = *((SuperpoweredAdvancedAudioPlayer **)clientData);
        playerB->setBpm(123.0f);
        playerB->setFirstBeatMs(40);
        playerB->setPosition(playerB->firstBeatMs, false, false);
    };
}

static bool audioProcessing(void *clientdata, short int *audioIO, int numberOfSamples, int samplerate) {
    //__android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Processing");
	return ((MultiMixer *)clientdata)->process(audioIO, numberOfSamples);
}

MultiMixer::MultiMixer(int *params) {
    pthread_mutex_init(&mutex, NULL); // This will keep our player volumes and playback states in sync.
    samplerate = params[0];
    unsigned int buffersize = params[1];
    stereoBuffer = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);
    playerA = NULL;

    audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioProcessing, this, -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2);
}

MultiMixer::~MultiMixer() {
    delete audioSystem;
    free(stereoBuffer);
    pthread_mutex_destroy(&mutex);
}

void MultiMixer::play(const char* path, int length) {
    pthread_mutex_lock(&mutex);
    __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Playing %s. Length=%d", path, length);
    playerA = new SuperpoweredAdvancedAudioPlayer(&playerA , playerEventCallbackA, samplerate, 0);
    playerA->open(path, 0, length);
    playerA->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_None;

    playerA->play(false);
    pthread_mutex_unlock(&mutex);
}

bool MultiMixer::process(short int *output, unsigned int numberOfSamples) {
    pthread_mutex_lock(&mutex);

    bool silence = true;
    if (playerA) {
        silence = !playerA->process(stereoBuffer, false, numberOfSamples);
    }
    // if (playerB->process(stereoBuffer, !silence, numberOfSamples, volB, masterBpm, msElapsedSinceLastBeatA)) silence = false;

    pthread_mutex_unlock(&mutex);

    // The stereoBuffer is ready now, let's put the finished audio into the requested buffers.
    if (!silence) SuperpoweredFloatToShortInt(stereoBuffer, output, numberOfSamples);
    return !silence;
}

extern "C" {
	JNIEXPORT void Java_com_superpowered_multimixer_MultiMixer_create(JNIEnv *javaEnvironment, jobject self, jlongArray offsetAndLength);
	JNIEXPORT void Java_com_superpowered_multimixer_MultiMixer_play(JNIEnv *javaEnvironment, jobject self, jstring jpath, jlong length);
}

static MultiMixer *mixer = NULL;

// Android is not passing more than 2 custom parameters, so we had to pack file offsets and lengths into an array.
JNIEXPORT void Java_com_superpowered_multimixer_MultiMixer_create(JNIEnv *javaEnvironment, jobject self, jlongArray params) {
    __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Initializing MultiMixer");

	// Convert the input jlong array to a regular int array.
    jlong *longParams = javaEnvironment->GetLongArrayElements(params, JNI_FALSE);
    int arr[2];
    for (int n = 0; n < 2; n++) arr[n] = longParams[n];
    javaEnvironment->ReleaseLongArrayElements(params, longParams, JNI_ABORT);

    mixer  = new MultiMixer(arr);
}

JNIEXPORT void Java_com_superpowered_multimixer_MultiMixer_play(JNIEnv *javaEnvironment, jobject self, jstring jpath, jlong length) {
    const char *path = javaEnvironment->GetStringUTFChars(jpath, JNI_FALSE);
	mixer->play(path, length);
    javaEnvironment->ReleaseStringUTFChars(jpath, path);
}