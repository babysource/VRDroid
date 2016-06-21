/**
 * Created on 2016/3/31
 */
package org.babysource.vrdroid;

import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.text.TextUtils;
import android.widget.TextView;

import com.google.vr.sdk.widgets.video.VrVideoEventListener;
import com.google.vr.sdk.widgets.video.VrVideoView;

import java.io.File;
import java.io.IOException;

/**
 * VR资源播放
 * <p/>
 *
 * @author Wythe
 */
public class VRDroidPlayer extends FragmentActivity {

    final static String KEY_NAME = "_$KEY_NAME";
    final static String KEY_PATH = "_$KEY_PATH";

    private TextView mVrVideoName;

    private VrVideoView mVrVideoView;

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        this.finish();
    }

    @Override
    protected void onDestroy() {
        if (this.mVrVideoView != null) {
            this.mVrVideoView.pauseRendering();
            this.mVrVideoView.shutdown();
        }
        super.onDestroy();
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (this.mVrVideoView != null) {
            this.mVrVideoView.pauseVideo();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (this.mVrVideoView != null) {
            this.mVrVideoView.playVideo();
        }
    }

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.droid_player);
        this.mVrVideoName = (TextView) this.findViewById(R.id.tv_video_name);
        if (this.mVrVideoName != null) {
            final String name = this.getIntent().getStringExtra(KEY_NAME);
            if (!TextUtils.isEmpty(name)) {
                this.mVrVideoName.setText(name);
            }
        }
        this.mVrVideoView = (VrVideoView) this.findViewById(R.id.vr_video_view);
        if (this.mVrVideoView != null) {
            final String path = this.getIntent().getStringExtra(KEY_PATH);
            if (!TextUtils.isEmpty(path)) {
                this.mVrVideoView.setInfoButtonEnabled(false);
                this.mVrVideoView.setEventListener(new VrVideoEventListener() {

                });
                try {
                    this.mVrVideoView.loadVideo(
                            Uri.fromFile(new File(path)), new VrVideoView.Options()
                    );
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

}
