/**
 * Created on 2016/3/31
 */
package org.babysource.vrdroid;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.opengl.GLES20;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.v4.app.ActivityCompat;
import android.view.Surface;
import android.widget.Toast;

import com.google.vr.sdk.base.Eye;
import com.google.vr.sdk.base.GvrActivity;
import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.HeadTransform;
import com.google.vr.sdk.base.Viewport;

import java.util.Arrays;
import java.util.concurrent.Semaphore;

import javax.microedition.khronos.egl.EGLConfig;

/**
 * VR资源列表
 * <p/>
 *
 * @author Wythe
 */
public class VRDroidCamera2 extends GvrActivity implements GvrView.StereoRenderer {

    private Semaphore mCameraLocker = new Semaphore(1);

    private long mBackPressedTime;

    private int mTarget;

    private int mProgram;

    private Handler mCameraHandle;

    private CameraDevice mCameraDevice;

    private HandlerThread mCameraThread;

    private CameraManager mCameraManager;

    private GvrView mGvrView;

    private SurfaceTexture mCameraTexture;

    private CaptureRequest.Builder mCameraBuilder;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.droid_lister);
        this.mGvrView = (GvrView) this.findViewById(R.id.gvr_view);
        if (this.mGvrView != null) {
            this.setGvrView(this.mGvrView);
            // 设置视图
            this.mGvrView.setRenderer(this);
            this.mGvrView.setVRModeEnabled(true);
            this.mGvrView.setTransitionViewEnabled(true);
            this.mGvrView.setSettingsButtonEnabled(false);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (this.mGvrView != null) {
            this.mGvrView.onPause();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (this.mGvrView != null) {
            this.mGvrView.onResume();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        final long mCurrentTime = System.currentTimeMillis();
        if (mCurrentTime - this.mBackPressedTime > 1000) {
            Toast.makeText(this, R.string.app_logout, Toast.LENGTH_SHORT).show();
            this.mBackPressedTime = mCurrentTime;
            return;
        }
        super.onBackPressed();
        System.exit(0);
    }

    @Override
    public void onRendererShutdown() {
        this.shutCamera();
        this.stopCameraThread();
    }

    @Override
    public void onDrawEye(final Eye eye) {

    }

    @Override
    public void onNewFrame(final HeadTransform headTransform) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
        if (this.mCameraTexture != null) {
            this.mCameraTexture.updateTexImage();
        }
    }

    @Override
    public void onFinishFrame(final Viewport viewport) {

    }

    @Override
    public void onSurfaceCreated(final EGLConfig eglConfig) {
        GLES20.glClearColor(1.0F, 1.0F, 1.0F, 1.0F);
        this.openCamera();
    }

    @Override
    public void onSurfaceChanged(final int i0, final int i1) {

    }

    private void openCamera() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
            this.mCameraManager = (CameraManager) this.getSystemService(Context.CAMERA_SERVICE);
            if (this.mCameraManager != null) {
                this.openCameraThread();
                try {
                    for (final String cameraId : this.mCameraManager.getCameraIdList()) {
                        CameraCharacteristics characteristics = this.mCameraManager.getCameraCharacteristics(cameraId);
                        if (characteristics.get(CameraCharacteristics.LENS_FACING) == CameraCharacteristics.LENS_FACING_BACK) {
                            this.mCameraManager.openCamera(cameraId, this.mCameraStateCallback, this.mCameraHandle);
                            return;
                        }
                    }
                } catch (CameraAccessException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void shutCamera() {
        try {
            this.mCameraLocker.acquire();
            if (this.mCameraDevice != null) {
                this.mCameraDevice.close();
                this.mCameraDevice = null;
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        } finally {
            this.mCameraLocker.release();
        }
    }

    private void openCameraThread() {
        this.mCameraThread = new HandlerThread("VRCamera");
        this.mCameraThread.start();
        this.mCameraHandle = new Handler(
                this.mCameraThread.getLooper()
        );
    }

    private void stopCameraThread() {
        this.mCameraThread.quitSafely();
        try {
            this.mCameraThread.join();
            this.mCameraThread = null;
            this.mCameraHandle = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private final CameraDevice.StateCallback mCameraStateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onError(final CameraDevice cameraDevice, final int error) {
            mCameraLocker.release();
            cameraDevice.close();
            mCameraDevice = null;
        }

        @Override
        public void onOpened(final CameraDevice cameraDevice) {
            mCameraLocker.release();
            mCameraDevice = cameraDevice;
            mCameraTexture = new SurfaceTexture(
                    mTarget = VRDroidBuilder.createTexture()
            );
            try {
                mCameraBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            } catch (CameraAccessException e) {
                e.printStackTrace();
                mCameraBuilder = null;
            }
            if (mCameraBuilder != null) {
                final Surface surface = new Surface(mCameraTexture);
                if (surface != null) {
                    mCameraBuilder.addTarget(surface);
                    try {
                        mCameraDevice.createCaptureSession(Arrays.asList(surface), mCameraCaptureSessionCallback, mCameraHandle);
                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

        @Override
        public void onDisconnected(final CameraDevice cameraDevice) {
            mCameraLocker.release();
            cameraDevice.close();
            mCameraDevice = null;
        }
    };

    private final CameraCaptureSession.StateCallback mCameraCaptureSessionCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(final CameraCaptureSession session) {
            mCameraBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
            try {
                session.setRepeatingRequest(mCameraBuilder.build(), null, mCameraHandle);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onConfigureFailed(final CameraCaptureSession session) {

        }
    };

}
