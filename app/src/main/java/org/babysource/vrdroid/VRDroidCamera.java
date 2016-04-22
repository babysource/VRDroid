/**
 * Created on 2016/3/31
 */
package org.babysource.vrdroid;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.widget.Toast;

import com.google.vrtoolkit.cardboard.CardboardActivity;
import com.google.vrtoolkit.cardboard.CardboardView;
import com.google.vrtoolkit.cardboard.Eye;
import com.google.vrtoolkit.cardboard.HeadTransform;
import com.google.vrtoolkit.cardboard.Viewport;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

import javax.microedition.khronos.egl.EGLConfig;

/**
 * VR资源列表
 * <p/>
 *
 * @author Wythe
 */
public class VRDroidCamera extends CardboardActivity implements CardboardView.StereoRenderer, SurfaceTexture.OnFrameAvailableListener, Camera.FaceDetectionListener {

    private final float[] mRes = new float[16];

    private final float[] mRhs = new float[16];

    private long mBackPressedTime;

    private int mTarget;

    private int mProgram;

    private Camera mCamera;

    private ShortBuffer mOrderBuffer;

    private FloatBuffer mPlaneBuffer;

    private FloatBuffer mFrameBuffer;

    private CardboardView mCardboardView;

    private GLSurfaceView mGLSurfaceView;

    private SurfaceTexture mCameraTexture;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.droid_lister);
        this.mCardboardView = (CardboardView) this.findViewById(R.id.cardboard_view);
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
    protected void onPause() {
        super.onPause();
        if (this.mCardboardView != null) {
            this.mCardboardView.onPause();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (this.mCardboardView != null) {
            this.mCardboardView.onResume();
        }
    }

    @Override
    protected void onDestroy() {
        this.shutCamera();
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
    public void onNewFrame(final HeadTransform headTransform) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
        if (this.mCameraTexture != null) {
            this.mCameraTexture.updateTexImage();
        }
    }

    @Override
    public void onDrawEye(final Eye eye) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
        // 初始绘制
        GLES20.glUseProgram(this.mProgram);
        GLES20.glActiveTexture(VRDroidBuilder.GL_CAMERA_OES);
        GLES20.glBindTexture(VRDroidBuilder.GL_CAMERA_OES, this.mTarget);
        // 绘制平面
        final int mPositionHandle = GLES20.glGetAttribLocation(this.mProgram, VRDroidBuilder.ATTR_POSITION);
        GLES20.glEnableVertexAttribArray(mPositionHandle);
        GLES20.glVertexAttribPointer(mPositionHandle, VRDroidBuilder.COORDS_PER_VERTEX, GLES20.GL_FLOAT, false, VRDroidBuilder.COORDS_PER_VERTEX * 4, this.mPlaneBuffer);
        // 绘制纹理
        final int mTexCoordHandle = GLES20.glGetAttribLocation(this.mProgram, VRDroidBuilder.ATTR_TEXCOORD);
        GLES20.glEnableVertexAttribArray(mTexCoordHandle);
        GLES20.glVertexAttribPointer(mTexCoordHandle, VRDroidBuilder.COORDS_PER_VERTEX, GLES20.GL_FLOAT, false, VRDroidBuilder.COORDS_PER_VERTEX * 4, this.mFrameBuffer);
        // 顺序绘制
        GLES20.glDrawElements(GLES20.GL_TRIANGLES, VRDroidBuilder.ORDER_VERTICES.length, GLES20.GL_UNSIGNED_SHORT, this.mOrderBuffer);
        GLES20.glDisableVertexAttribArray(mPositionHandle);
        GLES20.glDisableVertexAttribArray(mTexCoordHandle);
        android.opengl.Matrix.multiplyMM(this.mRes, 0, eye.getEyeView(), 0, this.mRhs, 0);
    }

    @Override
    public void onRendererShutdown() {
        this.shutCamera();
    }

    @Override
    public void onFinishFrame(final Viewport viewport) {

    }

    @Override
    public void onSurfaceChanged(final int i, final int i1) {

    }

    @Override
    public void onSurfaceCreated(final EGLConfig eglConfig) {
        GLES20.glClearColor(0.1F, 0.1F, 0.1F, 0.5F);
        // 初始绘制顺序缓冲
        final ByteBuffer orderBuffer = ByteBuffer.allocateDirect(VRDroidBuilder.ORDER_VERTICES.length * 2).order(ByteOrder.nativeOrder());
        this.mOrderBuffer = orderBuffer.asShortBuffer();
        this.mOrderBuffer.put(VRDroidBuilder.ORDER_VERTICES);
        this.mOrderBuffer.position(0);
        // 初始平面顶点缓冲
        final ByteBuffer planeBuffer = ByteBuffer.allocateDirect(VRDroidBuilder.PLANE_VERTICES.length * 4).order(ByteOrder.nativeOrder());
        this.mPlaneBuffer = planeBuffer.asFloatBuffer();
        this.mPlaneBuffer.put(VRDroidBuilder.PLANE_VERTICES);
        this.mPlaneBuffer.position(0);
        // 初始纹理顶点缓冲
        final ByteBuffer frameBuffer = ByteBuffer.allocateDirect(VRDroidBuilder.FRAME_VERTICES.length * 4).order(ByteOrder.nativeOrder());
        this.mFrameBuffer = frameBuffer.asFloatBuffer();
        this.mFrameBuffer.put(VRDroidBuilder.FRAME_VERTICES);
        this.mFrameBuffer.position(0);
        // 创建绘制应用程序
        this.mProgram = GLES20.glCreateProgram();
        GLES20.glAttachShader(this.mProgram, VRDroidBuilder.loadGLShader(GLES20.GL_VERTEX_SHADER, VRDroidBuilder.VERTEX_SHADER_CODE));
        GLES20.glAttachShader(this.mProgram, VRDroidBuilder.loadGLShader(GLES20.GL_FRAGMENT_SHADER, VRDroidBuilder.FACADE_SHADER_CODE));
        GLES20.glLinkProgram(this.mProgram);
        // 打开摄像头
        this.openCamera();
    }

    @Override
    public void onFrameAvailable(final SurfaceTexture surfaceTexture) {
        if (this.mGLSurfaceView != null) {
            this.mGLSurfaceView.requestRender();
        }
    }

    @Override
    public void onFaceDetection(final Camera.Face[] faces, final Camera camera) {
        final int wide = this.mCardboardView.getWidth() / 2, high = this.mCardboardView.getHeight();
        if (wide > 0 && high > 0) {
            final RectF rectf = new RectF();
            for (final Camera.Face face : faces) {
                final Matrix matrix = new Matrix();
                matrix.postScale(wide / 2000F, high / 2000F);
                matrix.postTranslate(wide / 2F, high / 2F);
                rectf.set(face.rect);
                matrix.mapRect(rectf);
            }
        }
    }

    private void openCamera() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
            this.mCamera = Camera.open();
            if (this.mCamera != null) {
                this.mCameraTexture = new SurfaceTexture(
                        this.mTarget = VRDroidBuilder.createTexture()
                );
                this.mCameraTexture.setOnFrameAvailableListener(this);
                try {
                    this.mCamera.setPreviewTexture(this.mCameraTexture);
                } catch (IOException e) {
                    e.printStackTrace();
                    this.mCamera.release();
                    return;
                }
                // 开始偷窥
                this.mCamera.setFaceDetectionListener(this);
                this.mCamera.startPreview();
            }
        }
    }

    private void shutCamera() {
        if (this.mCamera != null) {
            this.mCamera.release();
            this.mCamera = null;
        }
    }

}
