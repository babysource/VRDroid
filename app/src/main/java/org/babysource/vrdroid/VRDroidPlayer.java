/**
 * Created on 2016/3/31
 */
package org.babysource.vrdroid;

import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.app.FragmentActivity;

import com.google.vrtoolkit.cardboard.widgets.video.VrVideoEventListener;
import com.google.vrtoolkit.cardboard.widgets.video.VrVideoView;

import java.io.File;
import java.io.IOException;

/**
 * VR资源播放
 * <p/>
 *
 * @author Wythe
 */
public class VRDroidPlayer extends FragmentActivity {

    private VrVideoView mVrVideoView;

    @Override
    protected void onDestroy() {
        if (this.mVrVideoView != null) {
            this.mVrVideoView.shutdown();
        }
        super.onDestroy();
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (this.mVrVideoView != null) {
            this.mVrVideoView.pauseRendering();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (this.mVrVideoView != null) {
            this.mVrVideoView.resumeRendering();
        }
    }

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.droid_player);
        this.mVrVideoView = (VrVideoView) this.findViewById(R.id.vr_video_view);
        if (this.mVrVideoView != null) {
            this.mVrVideoView.setInfoButtonEnabled(false);
            this.mVrVideoView.setEventListener(new VrVideoEventListener() {

            });
            try {
                this.mVrVideoView.loadVideo(Uri.fromFile(
                        new File(Environment.getExternalStorageDirectory() + File.separator + "360Videos/362.czvr")
                ));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

}
