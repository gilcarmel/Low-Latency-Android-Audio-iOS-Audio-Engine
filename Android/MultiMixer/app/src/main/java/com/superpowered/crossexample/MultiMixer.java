package com.superpowered.crossexample;

import android.content.Context;
import android.media.AudioManager;
import android.os.Build;

/**
 * Java interface for mixing an arbitrary number of audio streams using SuperPowered
 */
public class MultiMixer {

    private native void create(long[] params);

    private native void play(String filename);

    public MultiMixer(Context context) {
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
        create(params);
    }

    public void playFile(String filename) {
        play(filename);
    }

    static {
        System.loadLibrary("SuperpoweredExample");
    }
}
