// This proprietary software may be used only as
// authorised by a licensing agreement from ARM Limited
// (C) COPYRIGHT 2015 ARM Limited
// ALL RIGHTS RESERVED
// The entire notice above must be reproduced on all authorised
// copies and copies may only be made to the extent permitted
// by a licensing agreement from ARM Limited.
#include <jni.h>
#include <stdlib.h>
#include <time.h>

#include <android/log.h>
#define LOG_TAG "ARMVR"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#include <GLES3/gl3.h>
#include "armvr.cpp"

#define BASE_ASSET_PATH    "/data/data/com.arm.malideveloper.vrsdk.armvr/files/"
#define SHADER_PATH(name)  BASE_ASSET_PATH name
#include "loader.cpp"

#include <sys/time.h>
static timeval start_time;
static App app;

const char *get_gl_error_msg(GLenum code)
{
    switch (code)
    {
    case 0: return "NO_ERROR";
    case 0x0500: return "INVALID_ENUM";
    case 0x0501: return "INVALID_VALUE";
    case 0x0502: return "INVALID_OPERATION";
    case 0x0503: return "STACK_OVERFLOW";
    case 0x0504: return "STACK_UNDERFLOW";
    case 0x0505: return "OUT_OF_MEMORY";
    case 0x0506: return "INVALID_FRAMEBUFFER_OPERATION";
    default: return "UNKNOWN";
    }
}

void gl_check()
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        LOGD("An OpenGL error occurred %s", get_gl_error_msg(error));
        exit(1);
    }
}

extern "C"
{
    JNIEXPORT void JNICALL Java_org_babysource_vrdroid_VRDroid_init(JNIEnv* env, jobject obj)
    {
        LOGD("Init\n");
        start_time.tv_sec = 0;
        start_time.tv_usec = 0;
        gettimeofday(&start_time, NULL);

        LOGD("Load assets\n");
        load_assets(&app);
        app_initialize(&app);
    }

    JNIEXPORT void JNICALL Java_org_babysource_vrdroid_VRDroid_resize(JNIEnv* env, jobject obj, jint width, jint height)
    {
        app.window_width = width;
        app.window_height = height;
        app.elapsed_time = 0.0f;
        glViewport(0, 0, width, height);
        LOGD("Resizing %d %d\n", width, height);
    }

    JNIEXPORT void JNICALL Java_org_babysource_vrdroid_VRDroid_step(JNIEnv* env, jobject obj)
    {
        timeval now;
        gettimeofday(&now, NULL);
        float seconds  = (now.tv_sec - start_time.tv_sec);
        float milliseconds = (float(now.tv_usec - start_time.tv_usec)) / 1000000.0f;
        float elapsed_time = seconds + milliseconds;
        app.frame_time = elapsed_time - app.elapsed_time;
        app.elapsed_time = elapsed_time;

        app_update_and_render(&app);
        gl_check();
    }
};
