// This proprietary software may be used only as
// authorised by a licensing agreement from ARM Limited
// (C) COPYRIGHT 2015 ARM Limited
// ALL RIGHTS RESERVED
// The entire notice above must be reproduced on all authorised
// copies and copies may only be made to the extent permitted
// by a licensing agreement from ARM Limited.
package org.babysource.vrdroid;

import android.content.Context;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

class VRDroidView extends GLSurfaceView {

    public VRDroidView(Context context) {
        super(context);
        setEGLConfigChooser(8, 8, 8, 0, 16, 0);
        setEGLContextClientVersion(2);
        setRenderer(new Renderer());
    }

    private static class Renderer implements GLSurfaceView.Renderer {
        public void onDrawFrame(GL10 gl) {
            VRDroid.step();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            VRDroid.resize(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            VRDroid.init();
        }
    }

}
