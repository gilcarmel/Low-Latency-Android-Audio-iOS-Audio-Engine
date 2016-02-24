package com.superpowered.multimixer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;

import java.io.FileOutputStream;
import java.io.InputStream;

import android.view.View;

public class MainActivity extends AppCompatActivity {
    MultiMixer mixer;
    private String lyckaPath;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mixer = new MultiMixer(this);
        setContentView(R.layout.activity_main);

        lyckaPath = getFilesDir().getAbsolutePath() + "/" + "lycka.mp3";
        copyRawResourceToExternalDir(R.raw.lycka, lyckaPath);
    }

    private void copyRawResourceToExternalDir(int id, String path) {
        InputStream in = getResources().openRawResource(id);
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(path);
            byte[] buff = new byte[1024];
            int read = 0;

            while ((read = in.read(buff)) > 0) {
                out.write(buff, 0, read);
            }
            out.close();
            in.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void SuperpoweredExample_PlayPause(View button) {  // Play/pause.
        long id = mixer.prepare(lyckaPath);
        mixer.play(id);
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
}
