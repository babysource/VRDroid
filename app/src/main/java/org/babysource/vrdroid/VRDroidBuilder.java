/**
 * Created on 2016/4/21
 */
package org.babysource.vrdroid;

import android.opengl.GLES20;

import javax.microedition.khronos.opengles.GL10;

/**
 * <p/>
 *
 * @author Wythe
 */
public final class VRDroidBuilder {

    final static int GL_CAMERA_OES = 0x8D65;

    final static int MAX_PREVIEW_WIDE = 1920;

    final static int MAX_PREVIEW_HIGH = 1080;

    final static String ATTR_POSITION = "a_Position";

    final static String ATTR_TEXCOORD = "a_Texcoord";

    // 顶点坐标个数
    final static int COORDS_PER_VERTEX = 2;

    // 顶点绘制顺序
    final static short ORDER_VERTICES[] = {
            0, 3, 2, 0, 2, 1
    };

    // 定义平面顶点
    final static float PLANE_VERTICES[] = {
            -1.0F, 1.0F,  // 0：左上角
            1.0F, 1.0F,   // 1：右上角
            1.0F, -1.0F,  // 2：右下角
            -1.0F, -1.0F, // 3：左下角
    };

    // 定义纹理顶点
    final static float FRAME_VERTICES[] = {
            0.0f, 0.0f, // 0：左上角
            1.0f, 0.0f, // 1：右上角
            1.0f, 1.0f, // 2：右下角
            0.0f, 1.0f  // 3：左下角
    };

    // 顶点绘制程序
    final static String VERTEX_SHADER_CODE =
            "attribute vec4 " + ATTR_POSITION + ";" +
                    "attribute vec2 " + ATTR_TEXCOORD + ";" +
                    "varying vec2 v_Texcoord;" +
                    "void main() {" +
                    "  v_Texcoord = " + ATTR_TEXCOORD + ";" +
                    "  gl_Position = " + ATTR_POSITION + ";" +
                    "}";

    // 外观绘制程序
    final static String FACADE_SHADER_CODE =
            "#extension GL_OES_EGL_image_external : require\n" +
                    "precision mediump float;" +
                    "varying vec2 v_Texcoord;" +
                    "uniform samplerExternalOES s_Texture;" +
                    "void main() {" +
                    "  gl_FragColor = texture2D(s_Texture, v_Texcoord);" +
                    "}";

    final static int createTexture() {
        final int[] textures = new int[1];
        GLES20.glGenTextures(1, textures, 0);
        GLES20.glBindTexture(VRDroidBuilder.GL_CAMERA_OES, textures[0]);
        GLES20.glTexParameterf(VRDroidBuilder.GL_CAMERA_OES, GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_LINEAR);
        GLES20.glTexParameterf(VRDroidBuilder.GL_CAMERA_OES, GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_LINEAR);
        GLES20.glTexParameteri(VRDroidBuilder.GL_CAMERA_OES, GL10.GL_TEXTURE_WRAP_S, GL10.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(VRDroidBuilder.GL_CAMERA_OES, GL10.GL_TEXTURE_WRAP_T, GL10.GL_CLAMP_TO_EDGE);
        return textures[0];
    }

    final static int fitPreviewWide(final int wide) {
        if (wide < 0) {
            return 0;
        }
        return wide > MAX_PREVIEW_WIDE ? MAX_PREVIEW_WIDE : wide;
    }

    final static int fitPreviewHigh(final int high) {
        if (high < 0) {
            return 0;
        }
        return high > MAX_PREVIEW_HIGH ? MAX_PREVIEW_HIGH : high;
    }

    final static int loadGLShader(final int type, final String code) {
        int shader = GLES20.glCreateShader(type);
        GLES20.glShaderSource(shader, code);
        GLES20.glCompileShader(shader);
        // 校验编译状态
        final int[] compileStatus = new int[1];
        GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compileStatus, 0);
        if (compileStatus[0] == 0) {
            GLES20.glDeleteShader(shader);
            shader = 0;
        }
        if (shader == 0) {
            throw new RuntimeException("Creating shader error.");
        }
        return shader;
    }

}
