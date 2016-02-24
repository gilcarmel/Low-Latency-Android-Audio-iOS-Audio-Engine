package com.superpowered.multimixer;

import android.content.Context;
import android.media.AudioManager;
import android.os.Build;

import java.io.File;

/**
 * Java interface for mixing an arbitrary number of audio streams using SuperPowered
 */
public class MultiMixer {
    private static MultiMixer instance;
    public static MultiMixer create(Context context) {
        if (instance != null) {
            throw new AssertionError("Only one instance of MultiMixer allowed.\n");
        }
        instance = new MultiMixer(context);
        return instance;
    }

    public static MultiMixer get() {
        return instance;
    }

    private native void _create(long[] params);

    private native long _prepare (String filename, long length);

    private native boolean _pause (long id);

    private native boolean _play (long id);

    private native boolean _isPlaying(long id);

    private native boolean _seek(long id, long milliseconds);

    private native long _getDuration(long id);

    private native long _getPosition(long id);

    private native boolean _isLooping(long id);

    private native boolean _setLooping(long id, boolean looping);

    private MultiMixer(Context context) {
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

    public long prepare(String filename) {
        File file = new File(filename);
        final long length = file.length();
        return _prepare(filename, length);
    }

    public boolean play(long id) {
        return _play(id);
    }

    public boolean pause(long id) {
        return _pause(id);
    }
    public boolean isPlaying(long id) {
        return _isPlaying(id);
    }

    public boolean seek(long id, double seconds) {
        return _seek(id, (long) (seconds*1000));
    }

    public double getDuration(long id) {
        return _getDuration(id) / 1000.0;
    }


    public double getPosition(long id) {
        return _getPosition(id) / 1000.0;
    }

    public boolean setLooping(long id, boolean looping) {
        return _setLooping(id, looping);
    }

    public boolean isLooping(long id) {
        return _isLooping(id);
    }

    static {
        System.loadLibrary("MultiMixer");
    }
}
