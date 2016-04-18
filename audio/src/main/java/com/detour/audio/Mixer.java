package com.detour.audio;

import android.content.Context;
import android.media.AudioManager;
import android.os.Build;

import java.io.File;
import java.util.ArrayList;

/**
 * Java interface for mixing an arbitrary number of audio streams using SuperPowered
 */
public class Mixer {

    //Values must match DTEAudioFadeShape in DTEAudioFadeFilter
    public enum FadeShape {
        Linear(0), Exponential(1);

        private final int value;

        FadeShape(int value) {
            this.value = value;
        }

        public static FadeShape fromInt(int n) {
            for (FadeShape fadeShape : values()) {
                if (fadeShape.getValue() == n) {
                    return fadeShape;
                }
            }
            return null;
        }

        public int getValue() {
            return value;
        }
    }

    private static Mixer instance;

    public static Mixer create(Context context) {
        if (instance != null) {
            throw new AssertionError("Only one instance of DTEMixer allowed.\n");
        }
        instance = new Mixer(context);
        return instance;
    }

    public static void destroy() {
        instance.onDestroy();
        instance = null;
    }

    public static Mixer get() {
        return instance;
    }

    public ArrayList<Integer> streams = new ArrayList<>();

    private Mixer(Context context) {
        // Get the device's sample rate and buffer size to enable low-latency Android audio output, if available.
        String samplerateString = null, buffersizeString = null;
        if (Build.VERSION.SDK_INT >= 17) {
            AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
            samplerateString = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            buffersizeString = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        }
        if (samplerateString == null) samplerateString = "44100";
        if (buffersizeString == null) buffersizeString = "512";
        long[] params = {
                Integer.parseInt(samplerateString),
                Integer.parseInt(buffersizeString)
        };
        _create(params);
    }

    private void onDestroy() {
        //native code destroys any native streams
        _destroy();
    }

    public int prepare(String filename, float volume, float duckingVolume) {
        File file = new File(filename);
        int id = _prepare(filename, (int) file.length(), volume, duckingVolume);
        streams.add(id);
        return id;
    }

    public void close(int id) {
        streams.remove(Integer.valueOf(id));    //Disambiguate so it resolves to remove(Object), not remove(int index)
        _close(id);
    }

    public boolean play(int id) {
        return _play(id);
    }

    public boolean pause(int id) {
        return _pause(id);
    }

    public boolean isPlaying(int id) {
        return _isPlaying(id);
    }

    public boolean seek(int id, double seconds) {
        return _seek(id, (int) (seconds * 1000));
    }

    public double getDuration(int id) {
        return _getDuration(id) / 1000.0;
    }

    public double getPosition(int id) {
        return _getPosition(id) / 1000.0;
    }

    public boolean setLooping(int id, boolean looping) {
        return _setLooping(id, looping);
    }

    public boolean isLooping(int id) {
        return _isLooping(id);
    }

    public boolean setVolume(int id, float volume) {
        return _setVolume(id, volume);
    }

    public boolean setRegionStartTime(int id, double startTime) {
        return _setRegionStartTime(id, startTime);
    }

    public boolean setRegionDuration(int id, double duration) {
        return _setRegionDuration(id, duration);
    }

    public boolean fadeOut(int id, double startTime, double duration, FadeShape fadeShape, Runnable completion) {
        return _fadeOut(id, startTime, duration, fadeShape.getValue());
    }

    public boolean fadeIn(int id, double startTime, double duration, FadeShape fadeShape) {
        return _fadeIn(id, startTime, duration, fadeShape.getValue());
    }

    public boolean beginDuckingAtStartTime(int id, double startTime, double duration, FadeShape fadeShape) {
        return _beginDucking(id, startTime, duration, fadeShape.getValue());
    }

    public boolean endDuckingAtStartTime(int id, double startTime, double duration, FadeShape fadeShape) {
        return _endDucking(id, startTime, duration, fadeShape.getValue());
    }


    //Native methods:
    private native void _create(long[] params);

    private native void _destroy();

    private native int _prepare(String filename, int length, float volume, float duckingVolume);

    private native boolean _close(int id);

    private native boolean _pause(int id);

    private native boolean _play(int id);

    private native boolean _isPlaying(int id);

    private native boolean _seek(int id, int milliseconds);

    private native int _getDuration(int id);

    private native int _getPosition(int id);

    private native boolean _isLooping(int id);

    private native boolean _setLooping(int id, boolean looping);

    private native boolean _fadeIn(int id, double startTime, double duration, int value);

    private native boolean _fadeOut(int id, double startTime, double duration, int fadeShape);

    private native boolean _beginDucking(int id, double startTime, double duration, int fadeShape);

    private native boolean _endDucking(int id, double startTime, double duration, int fadeShape);

    private native boolean _setVolume(int id, float volume);

    private native boolean _setRegionDuration(int id, double duration);

    private native boolean _setRegionStartTime(int id, double startTime);

    static {
        System.loadLibrary("DTEMixer");
    }

}
