package com.superpowered.crossexample;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.media.AudioManager;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import java.io.IOException;
import android.os.Build;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Button;
import android.view.View;

public class MainActivity extends AppCompatActivity {
    boolean playing = false;
    MultiMixer mixer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mixer = new MultiMixer(this);
        setContentView(R.layout.activity_main);



//        // Arguments: path to the APK file, offset and length of the two resource files, sample rate, audio buffer size.
//        SuperpoweredExample(getPackageResourcePath(), params);

//        // crossfader events
//        final SeekBar crossfader = (SeekBar)findViewById(R.id.crossFader);
//        crossfader.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
//
//            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
//                onCrossfader(progress);
//            }
//
//            public void onStartTrackingTouch(SeekBar seekBar) {}
//            public void onStopTrackingTouch(SeekBar seekBar) {}
//        });
//
//        // fx fader events
//        final SeekBar fxfader = (SeekBar)findViewById(R.id.fxFader);
//        fxfader.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
//
//            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
//                onFxValue(progress);
//            }
//
//            public void onStartTrackingTouch(SeekBar seekBar) {
//                onFxValue(seekBar.getProgress());
//            }
//
//            public void onStopTrackingTouch(SeekBar seekBar) {
//                onFxOff();
//            }
//        });
//
//        // fx select event
//        final RadioGroup group = (RadioGroup)findViewById(R.id.radioGroup1);
//        group.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
//            public void onCheckedChanged(RadioGroup radioGroup, int checkedId) {
//                RadioButton checkedRadioButton = (RadioButton)radioGroup.findViewById(checkedId);
//                onFxSelect(radioGroup.indexOfChild(checkedRadioButton));
//            }
//        });
    }

    public void SuperpoweredExample_PlayPause(View button) {  // Play/pause.
        mixer.playFile("hello.wav");
//        playing = !playing;
//        onPlayPause(playing);
//        Button b = (Button) findViewById(R.id.playPause);
//        b.setText(playing ? "Pause" : "Play");
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

//    private native void SuperpoweredExample(String apkPath, long[] offsetAndLength);
//    private native void onPlayPause(boolean play);
//    private native void onCrossfader(int value);
//    private native void onFxSelect(int value);
//    private native void onFxOff();
//    private native void onFxValue(int value);


}
