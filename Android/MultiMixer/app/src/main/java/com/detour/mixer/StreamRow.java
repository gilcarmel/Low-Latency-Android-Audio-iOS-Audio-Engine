package com.detour.mixer;

import android.annotation.TargetApi;
import android.content.Context;
import android.os.Build;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

public class StreamRow extends RelativeLayout {

    public StreamRowDelegate delegate;

    public interface StreamRowDelegate {
        void closeStream(int id);
    }

    private int id = -1;
    private Button playPauseButton;
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

    public void setId(int id) {
        this.id = id;
        TextView streamIdView = (TextView) findViewById(R.id.stream_id);
        streamIdView.setText(((Number) id).toString());
        setUpPlayPauseButton();
        setUpFadeOutButton();
        setUpDuckButton();
        setUpSeekBar();
        setUpLoopCheckbox();
        setUpCloseButton();
        updateUi();
    }

    private void setUpFadeOutButton() {
        Button fadeOutButton = (Button) findViewById(R.id.row_fade_out);
        fadeOutButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (isPlaying()) {
                    double currentTime = Mixer.get().getPosition(id);
                    Mixer.get().fadeOut(id, currentTime, 0.5, Mixer.FadeShape.Linear, new Runnable() {

                        @Override
                        public void run() {

                        }
                    });
                }
            }
        });
    }

    private void setUpDuckButton() {
        Button duckButton = (Button) findViewById(R.id.row_duck);
        duckButton.setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                double currentTime = Mixer.get().getPosition(id);
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        Mixer.get().beginDuckingAtStartTime(id, currentTime, 0.5, Mixer.FadeShape.Linear);
                        return true;
                    case MotionEvent.ACTION_UP:
                    case MotionEvent.ACTION_CANCEL:
                        Mixer.get().endDuckingAtStartTime(id, currentTime, 0.5, Mixer.FadeShape.Linear);
                        return true;
                }
                return false;
            }
        });
    }

    public void updateUi() {
        if (id > 0) {
            updateSeekBar();
            updatePlayPauseButton();
        }
    }

    private void setUpLoopCheckbox() {
        CheckBox loop = (CheckBox) findViewById(R.id.stream_loop);
        loop.setChecked(Mixer.get().isLooping(id));
        loop.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                Mixer.get().setLooping(id, isChecked);
            }
        });
    }

    private void setUpSeekBar() {
        seekBar = (SeekBar) findViewById(R.id.seekBar);
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (trackingSeek) {
                    double durationSeconds = Mixer.get().getDuration(id);
                    if (durationSeconds > 0) {
                        Mixer.get().seek(id, progress / 100.0 * durationSeconds);
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

    private void updateSeekBar() {
        if (!trackingSeek) {
            double durationSeconds = Mixer.get().getDuration(id);
            double positionSeconds = Mixer.get().getPosition(id);
            seekBar.setProgress((int) (100.0 * positionSeconds / durationSeconds));
        }
    }

    private void setUpPlayPauseButton() {
        playPauseButton = (Button) findViewById(R.id.row_play_pause);
        playPauseButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (isPlaying()) {
                    Mixer.get().pause(id);
                } else {
                    Mixer.get().play(id);
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

    private void setUpCloseButton() {
        Button closeButton = (Button) findViewById(R.id.row_close_button);
        closeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                delegate.closeStream(id);
            }
        });
        updatePlayPauseButton();
    }


    private boolean isPlaying() {
        return Mixer.get().isPlaying(id);
    }
}
