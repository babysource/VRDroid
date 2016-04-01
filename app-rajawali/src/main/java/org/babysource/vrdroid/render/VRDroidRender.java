/**
 * Created on 2016/3/31
 */
package org.babysource.vrdroid.render;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Environment;
import android.view.MotionEvent;

import org.rajawali3d.materials.Material;
import org.rajawali3d.materials.textures.ATexture;
import org.rajawali3d.materials.textures.StreamingTexture;
import org.rajawali3d.primitives.Sphere;
import org.rajawali3d.vr.renderer.VRRenderer;

import java.io.File;

/**
 * VR资源渲染
 * <p/>
 *
 * @author Wythe
 */
public class VRDroidRender extends VRRenderer {

    private MediaPlayer mMediaPlayer;

    private StreamingTexture mMediaTexture;

    public VRDroidRender(Context context) {
        super(context);
        this.setFrameRate(60.0D);
    }

    @Override
    protected void initScene() {
        if (this.mMediaPlayer == null) {
            this.mMediaPlayer = MediaPlayer.create(getContext(), Uri.fromFile(
                    new File(Environment.getExternalStorageDirectory() + File.separator + "360Videos/360.mp4")
            ));
        }
        if (this.mMediaTexture == null) {
            this.mMediaTexture = new StreamingTexture("video", this.mMediaPlayer);
        }
        final Material material = new Material();
        try {
            material.addTexture(this.mMediaTexture);
        } catch (ATexture.TextureException e) {
            e.printStackTrace();
            return;
        }
        final Sphere sphere = new Sphere(1, 24, 24);
        sphere.setMaterial(material);
        sphere.setPosition(0, 0, 0);
        sphere.setScaleX(100);
        sphere.setScaleY(100);
        sphere.setScaleZ(-100);
        material.setColorInfluence(0F);
        getCurrentScene().addChild(sphere);
        this.mMediaPlayer.setLooping(true);
        this.mMediaPlayer.start();
    }

    @Override
    public void onRenderSurfaceDestroyed(final SurfaceTexture surface) {
        super.onRenderSurfaceDestroyed(surface);
        if (this.mMediaTexture != null) {
            this.mMediaTexture = null;
        }
        if (this.mMediaPlayer != null) {
            try {
                this.mMediaPlayer.stop();
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                this.mMediaPlayer.release();
            }
            this.mMediaPlayer = null;
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        if (this.mMediaPlayer != null) {
            this.mMediaPlayer.pause();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if (this.mMediaPlayer != null) {
            this.mMediaPlayer.start();
        }
    }

    @Override
    protected void onRender(final long ellapsedRealtime, final double deltaTime) {
        super.onRender(ellapsedRealtime, deltaTime);
        if (this.mMediaTexture != null) {
            this.mMediaTexture.update();
        }
    }

    @Override
    public void onTouchEvent(final MotionEvent event) {

    }

    @Override
    public void onOffsetsChanged(final float xOffset, final float yOffset, final float xOffsetStep, final float yOffsetStep, final int xPixelOffset, final int yPixelOffset) {

    }

}
