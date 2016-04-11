// This proprietary software may be used only as
// authorised by a licensing agreement from ARM Limited
// (C) COPYRIGHT 2015 ARM Limited
// ALL RIGHTS RESERVED
// The entire notice above must be reproduced on all authorised
// copies and copies may only be made to the extent permitted
// by a licensing agreement from ARM Limited.
#ifndef _armvr_h_
#define _armvr_h_
#include <stdio.h>
#include "matrix.h"
#define Meter(x) (x)
#define Centimeter(x) (Meter(x) / 100.0f)
#define Millimeter(x) (Meter(x) / 1000.0f)

// The following compile-time constants are used to calibrate the
// VR experience for each device and each user. For the optimal
// experience, you will need to perform measurements on your own
// device, head-mounted-display and eyes. The default values are
// calibrated for a Samsung Note 4 for a Gear VR SM-R320 HMD.

#define Num_Eyes 2
#define Num_Views Num_Eyes * 2

// The dimensions of the device screen in pixels and meters.
#define Screen_Resolution_X 2560
#define Screen_Resolution_Y 1440
#define Screen_Size_X       Meter(0.125f)
#define Screen_Size_Y       Meter(0.072f)

// The dimensions of the framebuffers used for both eyes when
// rendering the scene. The values for these will balance visual
// quality and performance. The framebuffers will be scaled down
// or up to fit inside the viewports.
#define Eye_Fb_Resolution_X 1280
#define Eye_Fb_Resolution_Y 1440

// If multisampling is available on the device the framebuffers
// will be rendered to using multisampling.
#define Multisample_Samples 4

// The interpupillary distance (IPD) is the distance between
// your pupils when looking straight ahead. The human average
// is about 64mm, which is the same as the distance between the
// lenses of the Gear VR headset. The user should set these to
// their own measured IPD for the most comfortable experience.
#define Eye_IPD Millimeter(61.0f)

// This should be set equal to the distance between the lens
// centres in the head-mounted display.
#define Lens_IPD Millimeter(64.0f)

// This should be set equal to the distance between the display
// and the center point of the viewer's eye.
#define Eye_Display_Distance Centimeter(8.0f)

// Defining border color enums in case the headers are not up to date for using the android extension pack.
#ifndef GL_TEXTURE_BORDER_COLOR_EXT
#define GL_TEXTURE_BORDER_COLOR_EXT 0x1004
#endif

#ifndef GL_CLAMP_TO_BORDER_EXT
#define GL_CLAMP_TO_BORDER_EXT 0x812D
#endif

struct Framebuffer
{
    int width;
    int height;
    GLuint framebuffer;
    GLuint depthbuffer;
    GLuint colorbuffer;
};

// These coefficients control the degree of distortion that
// is applied on the rendertargets, per channel. The notation
// is the same as in the original Brown-Conray distortion
// correction model.
struct DistortionCoefficients
{
    // Radial distortion coefficients
    float k1; // Central
    float k2; // Edge
    float k3; // Fine

    // Tangential distortion coefficients
    float p1; // Horizontal
    float p2; // Vertical
};

struct LensConfig
{
    // One set for each channel, to handle chromatic aberration.
    DistortionCoefficients coefficients_red;
    DistortionCoefficients coefficients_green;
    DistortionCoefficients coefficients_blue;

    // The viewer may not look through the lens centre. This means
    // that we need to perform an asymmetrical barrel distortion,
    // centered at an offset given by the difference between the
    // viewer's eye seperation and the HMD lens seperation.
    vec2 distort_centre;

    // Each eye should be looking at the centre of each produced
    // framebuffer image. To do this we must shift the result of
    // each framebuffer by an appropriate amount, such that when
    // the viewer looks straight ahead, the left pupil is in the
    // centre of the left image and the right pupil vice versa.
    vec2 image_centre;

    // The distorted image will appear smaller on the screen,
    // depending on the values of the distortion coefficients.
    // We apply a shape-preserving upscale to make the distorted
    // image fit into the edges of the screen. The value of this
    // scale is somewhat arbitrarily chosen.
    float fill_scale;
};

struct HMDConfig
{
    LensConfig left;
    LensConfig right;
};

struct WarpMesh
{
    GLuint vbo;
    GLuint ibo;
    int index_count;
};

struct App
{
    int window_width;
    int window_height;
    float frame_time;
    float elapsed_time;

    // Distortion shader
    GLuint program_distort;
    GLuint a_distort_position;
    GLuint a_distort_uv_red_low_res;
    GLuint a_distort_uv_green_low_res;
    GLuint a_distort_uv_blue_low_res;
    GLuint a_distort_uv_red_high_res;
    GLuint a_distort_uv_green_high_res;
    GLuint a_distort_uv_blue_high_res;
    GLuint u_distort_layer_index;
    GLuint u_distort_framebuffer;

    // Cube shader
    GLuint program_cube;
    GLuint a_cube_position;
    GLuint a_cube_normal;
    GLuint u_cube_projection;
    GLuint u_cube_view;
    GLuint u_cube_model;

    // Geometry
    GLuint vao;
    GLuint vbo_cube;
    GLuint warp_mesh[Num_Eyes];

    HMDConfig hmd;
    Framebuffer fb;
};

void gl_check(const char *msg);
void app_initialize(App *app);
void app_update_and_render(App *app);

/////////////////////////////////////
// Convenience macros

#define GL_CHECK(x)                                                                              \
    x;                                                                                           \
    {                                                                                            \
        GLenum glError = glGetError();                                                           \
        if(glError != GL_NO_ERROR) {                                                             \
            LOGE("glGetError() = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__); \
            exit(1);                                                                             \
        }                                                                                        \
    }

#define get_attrib_location(prog, name) \
    app->a_##prog##_##name = GL_CHECK(glGetAttribLocation(app->program_##prog, #name)); \
    if (app->a_##prog##_##name < 0) { \
        LOGE("Invalid or unused attribute %s\n", #name); \
    }

#define get_uniform_location(prog, name) \
    app->u_##prog##_##name = GL_CHECK(glGetUniformLocation(app->program_##prog, #name)); \
    if (app->u_##prog##_##name < 0) { \
        LOGE("Invalid or unused uniform %s\n", #name); \
    }

#define attribfv(prog, name, n, stride, offset) \
    GL_CHECK(glEnableVertexAttribArray(app->a_##prog##_##name)); \
    GL_CHECK(glVertexAttribPointer(app->a_##prog##_##name, n, GL_FLOAT, GL_FALSE, \
                          stride * sizeof(GLfloat), (void*)(offset * sizeof(GLfloat))));

#define attribiv(prog, name, n, stride, offset) \
    GL_CHECK(glEnableVertexAttribArray(app->a_##prog##_##name)); \
    GL_CHECK(glVertexAttribPointer(app->a_##prog##_##name, n, GL_INT, GL_FALSE, \
                          stride * sizeof(GLint), (void*)(offset * sizeof(GLint)) ));

#define uniform1f(prog, name, value)  GL_CHECK(glUniform1f(app->u_##prog##_##name, value));
#define uniform2f(prog, name, x, y)   GL_CHECK(glUniform2f(app->u_##prog##_##name, x, y));
#define uniform2fv(prog, name, value) GL_CHECK(glUniform2fv(app->u_##prog##_##name, 1, &value[0]));
#define uniform3fv(prog, name, value) GL_CHECK(glUniform3fv(app->u_##prog##_##name, 1, &value[0]));
#define uniform1i(prog, name, value)  GL_CHECK(glUniform1i(app->u_##prog##_##name, value));
#define uniformm4(prog, name, value)  GL_CHECK(glUniformMatrix4fv(app->u_##prog##_##name, 1, GL_FALSE, value.value_ptr()));
#define uniformm4array(prog, name, value, arraySize)  GL_CHECK(glUniformMatrix4fv(app->u_##prog##_##name, arraySize, GL_FALSE, value.value_ptr()));

#endif
