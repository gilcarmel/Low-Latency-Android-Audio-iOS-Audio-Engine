package com.superpowered.multimixer;

import android.annotation.TargetApi;
import android.content.Context;
import android.os.Build;
import android.os.Handler;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

public class StreamRow extends RelativeLayout {


    private long id;
    private Button playPauseButton;
    private Runnable uiUpdater;
    private boolean trackingSeek;
    private SeekBar seekBar;

    public StreamRow(Context context) {
        super(context);
        setUpChildren(context);
    }

    public StreamRow(Context context, AttributeSet attrs) {
        super(context, attrs);
        setUpChildren(context);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    public StreamRow(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        setUpChildren(context);
    }

    public StreamRow(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        setUpChildren(context);

    }

    private void setUpChildren(Context context) {
        LayoutInflater.from(context).inflate(R.layout.stream_row_contents, this, true);
    }

    public static StreamRow inflate(ViewGroup parent) {
        return (StreamRow) LayoutInflater.from(parent.getContext())
                .inflate(R.layout.stream_row, parent, false);
    }


    public void setId(long id) {
        this.id = id;
        TextView streamIdView = (TextView) findViewById(R.id.stream_id);
        streamIdView.setText(((Long)id).toString());
        setUpPlayPauseButton();
        setUpSeekBar();
        setUpLoopCheckbox();
        createUiUpdater();
    }

    private void setUpLoopCheckbox() {
        CheckBox loop = (CheckBox) findViewById(R.id.stream_loop);
        loop.setChecked(MultiMixer.get().isLooping(id));
        loop.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                MultiMixer.get().setLooping(id, isChecked);
            }
        });
    }

    private void setUpSeekBar() {
        seekBar = (SeekBar) findViewById(R.id.seekBar);
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (trackingSeek) {
                    double durationSeconds = MultiMixer.get().getDuration(id);
                    if (durationSeconds > 0) {
                        MultiMixer.get().seek(id, progress/100.0 * durationSeconds);
                    }
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                trackingSeek = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                trackingSeek = false;
            }
        });

    }

    private void createUiUpdater() {
        final Handler handler = new Handler();
        uiUpdater = new Runnable() {
            @Override
            public void run() {
                updateSeekBar();
                updatePlayPauseButton();
                handler.postDelayed(uiUpdater, 100);
            }
        };
        handler.postDelayed(uiUpdater, 100);
    }

    private void updateSeekBar() {
        if (!trackingSeek) {
            double durationSeconds = MultiMixer.get().getDuration(id);
            double positionSeconds = MultiMixer.get().getPosition(id);
            seekBar.setProgress((int) (100.0 * positionSeconds / durationSeconds));
        }
    }

    private void setUpPlayPauseButton() {
        playPauseButton = (Button) findViewById(R.id.row_play_pause);
        playPauseButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (isPlaying()) {
                    MultiMixer.get().pause(id);
                }
                else {
                    MultiMixer.get().play(id);
                }
                updatePlayPauseButton();
            }
        });
        updatePlayPauseButton();
    }

    private void updatePlayPauseButton() {
        boolean playing = isPlaying();
        playPauseButton.setText(playing ? "Pause" : "Play");
    }

    private boolean isPlaying() {
        return MultiMixer.get().isPlaying(id);
    }
}
