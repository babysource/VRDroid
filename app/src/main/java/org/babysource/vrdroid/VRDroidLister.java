/**
 * Created on 2016/3/31
 */
package org.babysource.vrdroid;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Bundle;

import com.google.vrtoolkit.cardboard.CardboardActivity;
import com.google.vrtoolkit.cardboard.CardboardView;
import com.google.vrtoolkit.cardboard.Eye;
import com.google.vrtoolkit.cardboard.HeadTransform;
import com.google.vrtoolkit.cardboard.Viewport;

import javax.microedition.khronos.egl.EGLConfig;

/**
 * VR资源列表
 * <p/>
 *
 * @author Wythe
 */
public class VRDroidLister extends CardboardActivity implements CardboardView.StereoRenderer {

    private CardboardView mCardboardView;

    private GLSurfaceView mGLSurfaceView;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.droid_lister);
        this.mCardboardView = (CardboardView) findViewById(R.id.cardboard_view);
        if (this.mCardboardView != null) {
            this.setCardboardView(this.mCardboardView);
            // 设置视图
            this.mCardboardView.setRenderer(this);
            this.mCardboardView.setVRModeEnabled(true);
            this.mCardboardView.setTransitionViewEnabled(true);
            this.mCardboardView.setSettingsButtonEnabled(false);
            this.mGLSurfaceView = this.mCardboardView.getGLSurfaceView();
        }
    }

    @Override
    public void onNewFrame(final HeadTransform headTransform) {

    }

    @Override
    public void onDrawEye(final Eye eye) {

    }

    @Override
    public void onRendererShutdown() {

    }

    @Override
    public void onFinishFrame(final Viewport viewport) {

    }

    @Override
    public void onSurfaceChanged(final int i, final int i1) {

    }

    @Override
    public void onSurfaceCreated(final EGLConfig eglConfig) {
        GLES20.glClearColor(0.1f, 0.1f, 0.1f, 0.5f);
    }

}
