package com.detour.mixer;

import android.content.Context;
import android.media.AudioManager;
import android.os.Build;

import java.io.File;
import java.util.ArrayList;

/**
 * Java interface for mixing an arbitrary number of audio streams using SuperPowered
 */
public class Mixer {
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

    private native void _create(long[] params);

    private native void _destroy();

    private native long _prepare (String filename, long length);

    private native boolean _close(long id);

    private native boolean _pause (long id);

    private native boolean _play (long id);

    private native boolean _isPlaying(long id);

    private native boolean _seek(long id, long milliseconds);

    private native long _getDuration(long id);

    private native long _getPosition(long id);

    private native boolean _isLooping(long id);

    private native boolean _setLooping(long id, boolean looping);

    ArrayList<Long> streams = new ArrayList<>();

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

    public long prepare(String filename) {
        File file = new File(filename);
        final long length = file.length();
        long id = _prepare(filename, length);
        streams.add(id);
        return id;
    }

    public void close(long id) {
        streams.remove(id);
        _close(id);
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
        System.loadLibrary("DTEMixer");
    }

}
