// This proprietary software may be used only as
// authorised by a licensing agreement from ARM Limited
// (C) COPYRIGHT 2015 ARM Limited
// ALL RIGHTS RESERVED
// The entire notice above must be reproduced on all authorised
// copies and copies may only be made to the extent permitted
// by a licensing agreement from ARM Limited.
package org.babysource.vrdroid;

import android.content.res.AssetManager;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import java.io.File;
import java.io.InputStream;
import java.io.RandomAccessFile;

public class VRDroid extends Activity {

    VRDroidView mView;
    private static android.content.Context applicationContext = null;
    private static String assetsDirectory = null;
    private static String LOGTAG = "VRDroid";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mView = new VRDroidView(getApplication());
        setContentView(mView);

        applicationContext = getApplicationContext();
        assetsDirectory = applicationContext.getFilesDir().getPath() + "/";

        extractAsset("distort.vs");
        extractAsset("distort.fs");

        extractAsset("cube.vs");
        extractAsset("cube.fs");
    }

    @Override
    protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mView.onResume();
    }

    private void extractAsset(String assetName) {
        File file = new File(assetsDirectory + assetName);

        if (file.exists()) {
            Log.d(LOGTAG, assetName + " already exists. No extraction needed.\n");
        } else {
            Log.d(LOGTAG, assetName + " doesn't exist. Extraction needed. \n");

            try {
                RandomAccessFile randomAccessFile = new RandomAccessFile(assetsDirectory + assetName, "rw");
                AssetManager assetManager = applicationContext.getResources().getAssets();
                InputStream inputStream = assetManager.open(assetName);

                byte buffer[] = new byte[1024];
                int count = inputStream.read(buffer, 0, 1024);

                while (count > 0) {
                    randomAccessFile.write(buffer, 0, count);

                    count = inputStream.read(buffer, 0, 1024);
                }

                randomAccessFile.close();
                inputStream.close();
            } catch (Exception e) {
                Log.e(LOGTAG, "Failure in extractAssets(): " + e.toString() + " " + assetsDirectory + assetName);
            }

            if (file.exists()) {
                Log.d(LOGTAG, "File extracted successfully");
            }
        }
    }

    public static native void init();

    public static native void resize(int width, int height);

    public static native void step();

    static {
        System.loadLibrary("Native");
    }

}
