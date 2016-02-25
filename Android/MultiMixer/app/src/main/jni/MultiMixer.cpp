#include "MultiMixer.h"
#include "SuperpoweredSimple.h"
#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>


static MultiMixer *mixer = NULL;

typedef std::map<int,SuperpoweredAdvancedAudioPlayer*>::iterator it_type;

static void playerEventCallbackA(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void *value) {
    int id = (long)clientData;
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
        __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Player %d loaded", id);
    }
    else if (event == SuperpoweredAdvancedAudioPlayerEvent_EOF) {
        __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Player %d EOF", id);
        SuperpoweredAdvancedAudioPlayer* player = mixer->getPlayer(id);
        if (player && !mixer->isLoopingNoMutex(id)) {
            __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Player %d not looping, so pausing on EOF", id);
            player->pause();
        }
    }
}


static bool audioProcessing(void *clientdata, short int *audioIO, int numberOfSamples, int samplerate) {
    //__android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Processing");
	return ((MultiMixer *)clientdata)->process(audioIO, numberOfSamples);
}

MultiMixer::MultiMixer(int *params) : nextId(1) {
    __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Creating MultiMixer 0x%lx....", (long)this);
    pthread_mutex_init(&mutex, NULL); // This will keep our player volumes and playback states in sync.
    samplerate = params[0];
    unsigned int buffersize = params[1];
    stereoBuffer = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);

    audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioProcessing, this, -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2);
    __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Finished creating MultiMixer 0x%lx....", (long)this);
}

MultiMixer::~MultiMixer() {
    __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Destroying MultiMixer 0x%lx....", (long)this);

    pthread_mutex_lock(&mutex);
    for(it_type iterator = players.begin(); iterator != players.end(); iterator++) {
        SuperpoweredAdvancedAudioPlayer* player = iterator->second;
        __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Destroying player %d", iterator->first);
        delete player;
    }

    pthread_mutex_unlock(&mutex);

    delete audioSystem;
    free(stereoBuffer);
    pthread_mutex_destroy(&mutex);
    __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Finsihing destroying MultiMixer 0x%lx", (long)this);
}

int MultiMixer::prepare(const char* path, int length) {
    pthread_mutex_lock(&mutex);
    __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Preparing %s. Length=%d", path, length);
    int id = nextId++;
    SuperpoweredAdvancedAudioPlayer* player = new SuperpoweredAdvancedAudioPlayer((void*)(long)id, playerEventCallbackA, samplerate, 0);
    player->open(path, 0, length);
    player->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_None;

    players[id]=player;
    mLooping[id] = false;

    pthread_mutex_unlock(&mutex);

    return id;
}

bool MultiMixer::close(int id) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    SuperpoweredAdvancedAudioPlayer* player = getPlayer(id);
    if (player) {
        __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Closing %d.", id);
        delete player;
        players.erase(id);
        mLooping.erase(id);
        result = true;
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

//MUTEX MUST BE LOCKED BEFORE CALLING!!
SuperpoweredAdvancedAudioPlayer* MultiMixer::getPlayer(int id) {
    if (players.count(id) == 0) {
        __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Invalid player ID %d.", id);
        return NULL;
    }
    return players[id];
}

bool MultiMixer::play(int id) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    SuperpoweredAdvancedAudioPlayer* player = getPlayer(id);
    if (player) {
        __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Playing %d.", id);
        player->play(false);
        result = true;
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

bool MultiMixer::pause(int id) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    SuperpoweredAdvancedAudioPlayer* player = getPlayer(id);
    if (player) {
        __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Pausing %d.", id);
        player->pause();
        result = true;
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

bool MultiMixer::isPlaying(int id) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    SuperpoweredAdvancedAudioPlayer* player = getPlayer(id);
    if (player) {
        result = player->playing;
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

unsigned int MultiMixer::getDuration(int id) {
    unsigned int milliseconds = 0;
    pthread_mutex_lock(&mutex);
    SuperpoweredAdvancedAudioPlayer* player = getPlayer(id);
    if (player) {
        milliseconds = player->durationMs;
    }
    pthread_mutex_unlock(&mutex);

    return milliseconds;
}

unsigned int MultiMixer::getPosition(int id) {
    unsigned int milliseconds = 0;
    pthread_mutex_lock(&mutex);
    SuperpoweredAdvancedAudioPlayer* player = getPlayer(id);
    if (player) {
        milliseconds = player->positionMs;
    }
    pthread_mutex_unlock(&mutex);

    return milliseconds;
}

bool MultiMixer::seek(int id, unsigned int milliseconds) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    SuperpoweredAdvancedAudioPlayer* player = getPlayer(id);
    if (player) {
        __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "seek(%d, %d).", id, milliseconds);
        player->setPosition((double)milliseconds, false, false);
        result = true;
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

bool MultiMixer::setLooping(int id, bool looping) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    SuperpoweredAdvancedAudioPlayer* player = getPlayer(id);
    if (player) {
        __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "setLooping(%d, %s).", id, looping ? "true" : "false");
        mLooping[id] = looping;
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

bool MultiMixer::isLooping(int id) {
    pthread_mutex_lock(&mutex);
    bool result = isLoopingNoMutex(id);
    pthread_mutex_unlock(&mutex);

    return result;
}

bool MultiMixer::isLoopingNoMutex(int id) {
    bool result = false;
    SuperpoweredAdvancedAudioPlayer* player = getPlayer(id);
    if (player) {
        result = mLooping[id];
    }
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
	JNIEXPORT void Java_com_superpowered_multimixer_MultiMixer__1destroy(JNIEnv *javaEnvironment, jobject self);
	JNIEXPORT jlong Java_com_superpowered_multimixer_MultiMixer__1prepare(JNIEnv *javaEnvironment, jobject self, jstring jpath, jlong length);
	JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1close(JNIEnv *javaEnvironment, jobject self, jlong id);
	JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1play(JNIEnv *javaEnvironment, jobject self, jlong id);
	JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1pause(JNIEnv *javaEnvironment, jobject self, jlong id);
	JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1isPlaying(JNIEnv *javaEnvironment, jobject self, jlong id);
	JNIEXPORT jlong Java_com_superpowered_multimixer_MultiMixer__1getDuration(JNIEnv *javaEnvironment, jobject self, jlong id);
	JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1seek(JNIEnv *javaEnvironment, jobject self, jlong id, jlong milliseconds);
    JNIEXPORT jlong Java_com_superpowered_multimixer_MultiMixer__1getPosition(JNIEnv *javaEnvironment, jobject self, jlong id);
	JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1setLooping(JNIEnv *javaEnvironment, jobject self, jlong id, jboolean looping);
	JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1isLooping(JNIEnv *javaEnvironment, jobject self, jlong id);
}

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

JNIEXPORT void Java_com_superpowered_multimixer_MultiMixer__1destroy(JNIEnv *javaEnvironment, jobject self) {
    __android_log_print(ANDROID_LOG_VERBOSE, "MultiMixer", "Destroying MultiMixer");
    delete mixer;
    mixer = NULL;
}

JNIEXPORT jlong Java_com_superpowered_multimixer_MultiMixer__1prepare(JNIEnv *javaEnvironment, jobject self, jstring jpath, jlong length) {
    const char *path = javaEnvironment->GetStringUTFChars(jpath, JNI_FALSE);
	int id = mixer->prepare(path, length);
    javaEnvironment->ReleaseStringUTFChars(jpath, path);
    return id;
}

JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1close(JNIEnv *javaEnvironment, jobject self, jlong id) {
	return mixer->close(id);
}

JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1play(JNIEnv *javaEnvironment, jobject self, jlong id) {
	return mixer->play(id);
}

JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1pause(JNIEnv *javaEnvironment, jobject self, jlong id) {
    return mixer->pause(id);
}

JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1isPlaying(JNIEnv *javaEnvironment, jobject self, jlong id) {
    return mixer->isPlaying(id);
}

JNIEXPORT jlong Java_com_superpowered_multimixer_MultiMixer__1getDuration(JNIEnv *javaEnvironment, jobject self, jlong id) {
    return mixer->getDuration(id);
}

JNIEXPORT jlong Java_com_superpowered_multimixer_MultiMixer__1getPosition(JNIEnv *javaEnvironment, jobject self, jlong id) {
    return mixer->getPosition(id);
}

JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1seek(JNIEnv *javaEnvironment, jobject self, jlong id, jlong milliseconds) {
    return mixer->seek(id, milliseconds);
}

JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1setLooping(JNIEnv *javaEnvironment, jobject self, jlong id, jboolean looping) {
    return mixer->setLooping(id, looping);
}

JNIEXPORT jboolean Java_com_superpowered_multimixer_MultiMixer__1isLooping(JNIEnv *javaEnvironment, jobject self, jlong id) {
    return mixer->isLooping(id);
}
