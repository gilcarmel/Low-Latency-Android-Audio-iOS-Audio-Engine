#include "DTEMixer.h"
#include "SuperpoweredSimple.h"
#include <jni.h>
#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>

//Hack to get things linking in r11c
extern "C" {
    typedef void (*sighandler_t)(int);
    sighandler_t __unused bsd_signal(int __unused signum, sighandler_t __unused handler);
}

sighandler_t __unused bsd_signal(int __unused signum, sighandler_t __unused handler) {
    return 0;
}

static DTEMixer *mixer = NULL;

typedef std::map<int, DTEChannel *>::iterator channel_iter_type;


static bool audioProcessing(void *clientdata, short int *audioIO, int numberOfSamples,
                            int __unused samplerate) {
    //__android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Processing");
    return ((DTEMixer *) clientdata)->process(audioIO, (unsigned int) numberOfSamples);
}

DTEMixer::DTEMixer(unsigned int buffersize, unsigned int sampleRate) : nextId(1) {
    __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Creating DTEMixer 0x%lx....",
                        (long) this);
    pthread_mutex_init(&mutex,
                       NULL); // This will keep our player volumes and playback states in sync.
    stereoBuffer = (float *) memalign(16, (buffersize + 16) * sizeof(float) * 2);
    mSampleRate = sampleRate;
    audioSystem = new SuperpoweredAndroidAudioIO(mSampleRate, buffersize, false, true,
                                                 audioProcessing, this, -1, SL_ANDROID_STREAM_MEDIA,
                                                 buffersize * 2);
    __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Finished creating DTEMixer 0x%lx....",
                        (long) this);
}

DTEMixer::~DTEMixer() {
    __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Destroying DTEMixer 0x%lx....",
                        (long) this);

    pthread_mutex_lock(&mutex);
    for (channel_iter_type iterator = channels.begin(); iterator != channels.end(); iterator++) {
        DTEChannel *channel = iterator->second;
        __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Destroying channel %d",
                            iterator->first);
        delete channel;
    }
    channels.clear();

    pthread_mutex_unlock(&mutex);

    delete audioSystem;
    free(stereoBuffer);
    pthread_mutex_destroy(&mutex);
    __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Finsihing destroying DTEMixer 0x%lx",
                        (long) this);
}

int DTEMixer::prepare(const char *path, int length, float volume, float duckingVolume) {
    pthread_mutex_lock(&mutex);
    __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Preparing %s. Length=%d", path, length);
    int id = nextId++;
    DTEChannel *channel = new DTEChannel(mSampleRate, path, length, volume, duckingVolume);

    channels[id] = channel;

    pthread_mutex_unlock(&mutex);

    return id;
}

bool DTEMixer::close(int id) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Closing %d.", id);
        delete channel;
        channels.erase(id);
        result = true;
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

//MUTEX MUST BE LOCKED BEFORE CALLING!!
DTEChannel *DTEMixer::getChannel(int id) {
    if (channels.count(id) == 0) {
        __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Invalid channel ID %d.", id);
        return NULL;
    }
    return channels[id];
}

bool DTEMixer::play(int id) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Playing %d.", id);
        channel->play();
        result = true;
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

bool DTEMixer::pause(int id) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Pausing %d.", id);
        channel->pause();
        result = true;
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

bool DTEMixer::isPlaying(int id) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        result = channel->isPlaying();
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

bool DTEMixer::fadeOut(int id, double startTime, double duration,
                       DTEAudioFadeShape fadeShape) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        result = channel->fadeOut(startTime, duration, (DTEAudioFadeShape) fadeShape);
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

bool DTEMixer::fadeIn(int id, double startTime, double duration, DTEAudioFadeShape fadeShape) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        result = channel->fadeIn(startTime, duration, (DTEAudioFadeShape) fadeShape);
    }
    pthread_mutex_unlock(&mutex);

    return result;
}


bool DTEMixer::beginDucking(int id, double startTime, double duration,
                            DTEAudioFadeShape fadeShape) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        result = channel->beginDucking(startTime, duration, (DTEAudioFadeShape) fadeShape);
    }
    pthread_mutex_unlock(&mutex);

    return result;
}
bool DTEMixer::endDucking(int id, double startTime, double duration,
                          DTEAudioFadeShape fadeShape) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        result = channel->endDucking(startTime, duration, (DTEAudioFadeShape) fadeShape);
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

unsigned int DTEMixer::getDuration(int id) {
    unsigned int milliseconds = 0;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        milliseconds = channel->getDurationMS();
    }
    pthread_mutex_unlock(&mutex);

    return milliseconds;
}

unsigned int DTEMixer::getPosition(int id) {
    unsigned int milliseconds = 0;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        milliseconds = channel->getPositionMS();
    }
    pthread_mutex_unlock(&mutex);

    return milliseconds;
}

bool DTEMixer::seek(int id, unsigned int milliseconds) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "seek(%d, %d).", id, milliseconds);
        channel->setPosition(milliseconds);
        result = true;
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

bool DTEMixer::setLooping(int id, bool looping) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "setLooping(%d, %s).", id,
                            looping ? "true" : "false");
        channel->setLooping(looping);
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

bool DTEMixer::isLooping(int id) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        result = channel->isLooping();
    }
    pthread_mutex_unlock(&mutex);

    return result;
}


bool DTEMixer::setVolume(int id, float volume) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "setVolume(%d, %f).", id, volume);
        channel->setVolume(volume);
        result = true;
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

bool DTEMixer::setRegionDuration(int id, double duration) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "setRegionDuration(%d, %f).", id, duration);
        channel->setRegionDuration(duration);
        result = true;
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

bool DTEMixer::setRegionStartTime(int id, double startTime) {
    bool result = false;
    pthread_mutex_lock(&mutex);
    DTEChannel *channel = getChannel(id);
    if (channel) {
        __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "setRegionStartTime(%d, %f).", id, startTime);
        channel->setRegionStartTime(startTime);
        result = true;
    }
    pthread_mutex_unlock(&mutex);

    return result;}


bool DTEMixer::process(short int *output, unsigned int numberOfSamples) {
    pthread_mutex_lock(&mutex);

    bool silence = true;
    for (channel_iter_type iterator = channels.begin(); iterator != channels.end(); iterator++) {
        DTEChannel *channel = iterator->second;
        if (channel->process(stereoBuffer, silence, numberOfSamples)) {
            silence = false;
        }
    }

    pthread_mutex_unlock(&mutex);

    // The stereoBuffer is ready now, let's put the finished audio into the requested buffers.
    if (!silence) {
        SuperpoweredFloatToShortInt(stereoBuffer, output, numberOfSamples);
    }
    return !silence;
}

extern "C" {
JNIEXPORT void Java_com_detour_audio_Mixer__1create(JNIEnv *javaEnvironment, jobject self,
                                                    jlongArray offsetAndLength);
JNIEXPORT void Java_com_detour_audio_Mixer__1destroy(JNIEnv *javaEnvironment, jobject self);
JNIEXPORT jint Java_com_detour_audio_Mixer__1prepare(JNIEnv *javaEnvironment, jobject __unused self,
                                                     jstring jpath, jint length,
                                                     jfloat volume,
                                                     jfloat duckingVolume);
JNIEXPORT jboolean Java_com_detour_audio_Mixer__1close(JNIEnv __unused *javaEnvironment,
                                                       jobject self,
                                                       jint id);
JNIEXPORT jboolean Java_com_detour_audio_Mixer__1play(JNIEnv *javaEnvironment, jobject self,
                                                      jint id);
JNIEXPORT jboolean Java_com_detour_audio_Mixer__1pause(JNIEnv *javaEnvironment, jobject self,
                                                       jint id);
JNIEXPORT jboolean Java_com_detour_audio_Mixer__1isPlaying(JNIEnv *javaEnvironment, jobject self,
                                                           jint id);
JNIEXPORT jint Java_com_detour_audio_Mixer__1getDuration(JNIEnv *javaEnvironment, jobject self,
                                                         jint id);
JNIEXPORT jboolean Java_com_detour_audio_Mixer__1seek(JNIEnv *javaEnvironment, jobject self,
                                                      jint id, jint milliseconds);
JNIEXPORT jint Java_com_detour_audio_Mixer__1getPosition(JNIEnv *javaEnvironment, jobject self,
                                                         jint id);
JNIEXPORT jboolean Java_com_detour_audio_Mixer__1setLooping(JNIEnv *javaEnvironment, jobject self,
                                                            jint id, jboolean looping);
JNIEXPORT jboolean Java_com_detour_audio_Mixer__1isLooping(JNIEnv *javaEnvironment, jobject self,
                                                           jint id);
JNIEXPORT jboolean JNICALL Java_com_detour_audio_Mixer__1fadeOut(JNIEnv *env, jobject instance,
                                                                 jint id, jdouble startTime,
                                                                 jdouble duration, jint fadeShape);
JNIEXPORT jboolean JNICALL Java_com_detour_audio_Mixer__1fadeIn(JNIEnv *env, jobject instance,
                                                                jint id, jdouble startTime,
                                                                jdouble duration, jint fadeShape);
JNIEXPORT jboolean JNICALL Java_com_detour_audio_Mixer__1endDucking(JNIEnv *env,
                                                                    jobject instance,
                                                                    jint id,
                                                                    jdouble startTime,
                                                                    jdouble duration,
                                                                    jint fadeShape);
JNIEXPORT jboolean JNICALL Java_com_detour_audio_Mixer__1beginDucking(JNIEnv *env,
                                                                      jobject instance,
                                                                      jint id,
                                                                      jdouble startTime,
                                                                      jdouble duration,
                                                                      jint fadeShape);
JNIEXPORT jboolean JNICALL Java_com_detour_audio_Mixer__1setVolume(JNIEnv *env, jobject instance, jint id, jfloat volume);

JNIEXPORT jboolean JNICALL Java_com_detour_audio_Mixer__1setRegionDuration(JNIEnv *env, jobject instance, jint id, jdouble duration);

JNIEXPORT jboolean JNICALL Java_com_detour_audio_Mixer__1setRegionStartTime(JNIEnv *env, jobject instance, jint id, jdouble startTime);
}

// Android is not passing more than 2 custom parameters, so we had to pack file offsets and lengths into an array.
JNIEXPORT void Java_com_detour_audio_Mixer__1create(JNIEnv *javaEnvironment, jobject __unused self,
                                                    jlongArray params) {
    __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Initializing DTEMixer");

    // Convert the input jlong array to a regular int array.
    jlong *longParams = javaEnvironment->GetLongArrayElements(params, JNI_FALSE);
    long long int arr[2];
    for (int n = 0; n < 2; n++) arr[n] = longParams[n];
    javaEnvironment->ReleaseLongArrayElements(params, longParams, JNI_ABORT);

    unsigned int samplerate = (unsigned int) arr[0];
    unsigned int buffersize = (unsigned int) arr[1];
    mixer = new DTEMixer(buffersize, samplerate);
}

JNIEXPORT void Java_com_detour_audio_Mixer__1destroy(JNIEnv __unused *javaEnvironment,
                                                     jobject __unused self) {
    __android_log_print(ANDROID_LOG_VERBOSE, "DTEMixer", "Destroying DTEMixer");
    delete mixer;
    mixer = NULL;
}

JNIEXPORT jint Java_com_detour_audio_Mixer__1prepare(JNIEnv *javaEnvironment, jobject __unused self,
                                                     jstring jpath, jint length,
                                                     jfloat volume,
                                                     jfloat duckingVolume) {
    const char *path = javaEnvironment->GetStringUTFChars(jpath, JNI_FALSE);
    int id = mixer->prepare(path, length, volume, duckingVolume);
    javaEnvironment->ReleaseStringUTFChars(jpath, path);
    return id;
}

JNIEXPORT jboolean Java_com_detour_audio_Mixer__1close(JNIEnv __unused *javaEnvironment,
                                                       jobject __unused self,
                                                       jint id) {
    return (jboolean) mixer->close(id);
}

JNIEXPORT jboolean Java_com_detour_audio_Mixer__1play(JNIEnv __unused *javaEnvironment,
                                                      jobject __unused self,
                                                      jint id) {
    return (jboolean) mixer->play(id);
}

JNIEXPORT jboolean Java_com_detour_audio_Mixer__1pause(JNIEnv __unused *javaEnvironment,
                                                       jobject __unused self,
                                                       jint id) {
    return (jboolean) mixer->pause(id);
}

JNIEXPORT jboolean Java_com_detour_audio_Mixer__1isPlaying(JNIEnv __unused *javaEnvironment,
                                                           jobject __unused self,
                                                           jint id) {
    return (jboolean) mixer->isPlaying(id);
}

JNIEXPORT jint Java_com_detour_audio_Mixer__1getDuration(JNIEnv __unused *javaEnvironment,
                                                         jobject __unused self,
                                                         jint id) {
    return mixer->getDuration(id);
}

JNIEXPORT jint Java_com_detour_audio_Mixer__1getPosition(JNIEnv __unused *javaEnvironment,
                                                         jobject __unused self,
                                                         jint id) {
    return mixer->getPosition(id);
}

JNIEXPORT jboolean Java_com_detour_audio_Mixer__1seek(JNIEnv __unused *javaEnvironment,
                                                      jobject __unused self,
                                                      jint id, jint milliseconds) {
    return (jboolean) mixer->seek(id, (unsigned int) milliseconds);
}

JNIEXPORT jboolean Java_com_detour_audio_Mixer__1setLooping(JNIEnv __unused *javaEnvironment,
                                                            jobject __unused self,
                                                            jint id, jboolean looping) {
    return (jboolean) mixer->setLooping(id, looping);
}

JNIEXPORT jboolean Java_com_detour_audio_Mixer__1isLooping(JNIEnv __unused *javaEnvironment,
                                                           jobject __unused self,
                                                           jint id) {
    return (jboolean) mixer->isLooping(id);
}

JNIEXPORT jboolean JNICALL
Java_com_detour_audio_Mixer__1fadeOut(JNIEnv __unused *env, jobject __unused self, jint id,
                                      jdouble startTime,
                                      jdouble duration, jint fadeShape) {
    return (jboolean) mixer->fadeOut(id, startTime, duration, (DTEAudioFadeShape) fadeShape);
}

JNIEXPORT jboolean JNICALL
Java_com_detour_audio_Mixer__1fadeIn(JNIEnv __unused *env, jobject __unused self, jint id,
                                     jdouble startTime,
                                     jdouble duration, jint fadeShape) {
    return (jboolean) mixer->fadeIn(id, startTime, duration, (DTEAudioFadeShape) fadeShape);
}

JNIEXPORT jboolean JNICALL
Java_com_detour_audio_Mixer__1endDucking(JNIEnv *env, jobject instance, jint id,
                                         jdouble startTime, jdouble duration,
                                         jint fadeShape) {

    return (jboolean) mixer->endDucking(id, startTime, duration, (DTEAudioFadeShape) fadeShape);
}

JNIEXPORT jboolean JNICALL
Java_com_detour_audio_Mixer__1beginDucking(JNIEnv *env, jobject instance, jint id,
                                           jdouble startTime, jdouble duration,
                                           jint fadeShape) {

    return (jboolean) mixer->beginDucking(id, startTime, duration, (DTEAudioFadeShape) fadeShape);
}

JNIEXPORT jboolean JNICALL
Java_com_detour_audio_Mixer__1setVolume(JNIEnv *env, jobject instance, jint id, jfloat volume) {

    return (jboolean) mixer->setVolume(id, volume);
}

JNIEXPORT jboolean JNICALL
Java_com_detour_audio_Mixer__1setRegionDuration(JNIEnv *env, jobject instance, jint id,
                                                jdouble duration) {

    return (jboolean) mixer->setRegionDuration(id, duration);
}

JNIEXPORT jboolean JNICALL
Java_com_detour_audio_Mixer__1setRegionStartTime(JNIEnv *env, jobject instance, jint id,
                                                 jdouble startTime) {

    return (jboolean) mixer->setRegionStartTime(id, startTime);
}