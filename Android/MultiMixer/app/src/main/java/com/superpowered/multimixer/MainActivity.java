package com.superpowered.multimixer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;

import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.ArrayList;

import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;

public class MainActivity extends AppCompatActivity {
    MultiMixer mixer;
    private String lyckaPath;
    private StreamListAdapter streamListAdapter;
    private String nuyoricaPath;
    private String jorgePath;
    private String chatterPath;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mixer = MultiMixer.create(this);
        setContentView(R.layout.activity_main);

        ListView listView = (ListView) findViewById(R.id.streamList);
        streamListAdapter = new StreamListAdapter();
        listView.setAdapter(streamListAdapter);

        lyckaPath = getFilesDir().getAbsolutePath() + "/" + "lycka.mp3";
        copyRawResourceToExternalDir(R.raw.lycka, lyckaPath);
        nuyoricaPath = getFilesDir().getAbsolutePath() + "/" + "nuyorica.m4a";
        copyRawResourceToExternalDir(R.raw.nuyorica, nuyoricaPath);
        jorgePath = getFilesDir().getAbsolutePath() + "/" + "jorge.m4a";
        copyRawResourceToExternalDir(R.raw.jorge, jorgePath);
        chatterPath = getFilesDir().getAbsolutePath() + "/" + "chatter.m4a";
        copyRawResourceToExternalDir(R.raw.chatter, chatterPath);
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

    public void addLycka(View button) {  // Play/pause.
        addStreamWithPath(lyckaPath);
    }

    public void addNuyorica(View button) {  // Play/pause.
        addStreamWithPath(nuyoricaPath);
    }

    public void addJorge(View button) {  // Play/pause.
        addStreamWithPath(jorgePath);
    }

    public void addChatter(View button) {  // Play/pause.
        addStreamWithPath(chatterPath);
    }

    private void addStreamWithPath(String path) {
        long id = mixer.prepare(path);
        mixer.play(id);
        streamListAdapter.addStream(id);
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

    private class StreamListAdapter extends BaseAdapter {
        ArrayList<Long> streams = new ArrayList<>();
        @Override
        public int getCount() {
            return streams.size();
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            StreamRow row = (StreamRow)convertView;
            if (row == null) {
                row = StreamRow.inflate(parent);
            }

            row.setId(streams.get(position));

            return row;
        }

        @Override
        public Object getItem(int position) {
            return streams.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        public void addStream(long streamId) {
            streams.add(streamId);
            notifyDataSetChanged();
        }
    }
}
