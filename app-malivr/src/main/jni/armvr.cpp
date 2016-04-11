// This proprietary software may be used only as
// authorised by a licensing agreement from ARM Limited
// (C) COPYRIGHT 2015 ARM Limited
// ALL RIGHTS RESERVED
// The entire notice above must be reproduced on all authorised
// copies and copies may only be made to the extent permitted
// by a licensing agreement from ARM Limited.
#include "armvr.h"

// These are the dimensions of the viewports (in pixels) used
// when rendering each eye's framebuffer.
#define View_Resolution_X (Screen_Resolution_X / 2)
#define View_Resolution_Y Screen_Resolution_Y

// These are used to ensure that the distortion appears
// circular, even when the quad is stretched across a
// non-square region of the device. Furthermore, we use
// them to fit the resulting distorted image such that
// the rendered scene fully fits into the viewport.
#define View_Aspect_Ratio ((float)View_Resolution_X / (float)View_Resolution_Y)
#define Eye_Fb_Aspect_Ratio ((float)Eye_Fb_Resolution_X / (float)Eye_Fb_Resolution_Y)

// The near-clipping plane does not need to be equal to the
// projection plane (the hmd). It can be set larger as long
// as you can ensure that no geometry gets clipped (since
// that is really jarring for users).
#define Z_Near (Eye_Display_Distance)
#define Z_Far  Meter(12.0f)

// Instead of recomputing the distortion per frame, we store
// the distorted texel lookup coordinates in the attributes of
// a tessellated quad. The coordinates are linearly interpolated
// between each vertex. This gives acceptable results given a
// high enough resolution of the mesh, even though the distortion
// equations are nonlinear.
#define Warp_Mesh_Resolution_X 64
#define Warp_Mesh_Resolution_Y 64

#include <EGL/egl.h>
#include <EGL/eglext.h>

typedef void (GL_APIENTRY* PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVR)(GLenum, GLenum, GLuint, GLint, GLint, GLsizei);
typedef void (GL_APIENTRY* PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVR) (GLenum,  GLenum, GLuint, GLint, GLsizei, GLint, GLsizei);

Framebuffer make_eye_framebuffer(int width, int height, int num_views)
{
    Framebuffer result = {};
    result.width = width;
    result.height = height;

    PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVR glFramebufferTextureMultiviewOVR =
        (PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVR)eglGetProcAddress ("glFramebufferTextureMultiviewOVR");
    PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVR glFramebufferTextureMultisampleMultiviewOVR =
        (PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVR)eglGetProcAddress ("glFramebufferTextureMultisampleMultiviewOVR");

    if (!glFramebufferTextureMultiviewOVR)
    {
        LOGE("Did not have glFramebufferTextureMultiviewOVR\n");
        exit(EXIT_FAILURE);
    }
    if (!glFramebufferTextureMultisampleMultiviewOVR)
    {
        LOGE("Did not have glFramebufferTextureMultisampleMultiviewOVR\n");
    }

    bool have_multisampled_ext = glFramebufferTextureMultisampleMultiviewOVR != 0;

    GL_CHECK(glGenFramebuffers(1, &result.framebuffer));
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, result.framebuffer));

    GL_CHECK(glGenTextures(1, &result.depthbuffer));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, result.depthbuffer));
    GL_CHECK(glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT16, width, height, num_views));

    if (have_multisampled_ext)
    {
        GL_CHECK(glFramebufferTextureMultisampleMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, result.depthbuffer, 0, Multisample_Samples, 0, num_views));
    }
    else
    {
        GL_CHECK(glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, result.depthbuffer, 0, 0, num_views));
    }

    GL_CHECK(glGenTextures(1, &result.colorbuffer));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, result.colorbuffer));
    GL_CHECK(glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width, height, num_views));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_EXT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_EXT));
    GLint border_color[4] = {0, 0, 0, 0};
    GL_CHECK(glTexParameteriv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR_EXT, border_color));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));

    if (have_multisampled_ext)
    {
        GL_CHECK(glFramebufferTextureMultisampleMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, result.colorbuffer, 0, Multisample_Samples, 0, num_views));
    }
    else
    {
        GL_CHECK(glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, result.colorbuffer, 0, 0, num_views));
    }

    GLenum status = GL_CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE)
        LOGE("Framebuffer not complete\n");
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    return result;
}

GLuint make_cube()
{
    float v[] = {
        // front
        -1.0f, -1.0f, +1.0f, 0.0f, 0.0f, +1.0f,
        +1.0f, -1.0f, +1.0f, 0.0f, 0.0f, +1.0f,
        +1.0f, +1.0f, +1.0f, 0.0f, 0.0f, +1.0f,
        +1.0f, +1.0f, +1.0f, 0.0f, 0.0f, +1.0f,
        -1.0f, +1.0f, +1.0f, 0.0f, 0.0f, +1.0f,
        -1.0f, -1.0f, +1.0f, 0.0f, 0.0f, +1.0f,

        // back
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        -1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        +1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        +1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        +1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,

        // left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, +1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, +1.0f, +1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, +1.0f, +1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, +1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,

        // right
        +1.0f, -1.0f, -1.0f, +1.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, -1.0f, +1.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, +1.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, +1.0f, 0.0f, 0.0f,
        +1.0f, -1.0f, +1.0f, +1.0f, 0.0f, 0.0f,
        +1.0f, -1.0f, -1.0f, +1.0f, 0.0f, 0.0f,

        // up
        -1.0f, +1.0f, -1.0f, 0.0f, +1.0f, 0.0f,
        -1.0f, +1.0f, +1.0f, 0.0f, +1.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, 0.0f, +1.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, 0.0f, +1.0f, 0.0f,
        +1.0f, +1.0f, -1.0f, 0.0f, +1.0f, 0.0f,
        -1.0f, +1.0f, -1.0f, 0.0f, +1.0f, 0.0f,

        // down
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
        +1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
        +1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f,
        +1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f
    };

    GLuint result = 0;
    GL_CHECK(glGenBuffers(1, &result));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, result));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW));
    return result;
}

// Computes the distorted texel coordinate given the
// position on the image plane.
vec2 compute_distortion(float x, float y,
                        vec2 distort_centre,
                        DistortionCoefficients coefficients,
                        float tex_coord_factor)
{
    float k1 = coefficients.k1;
    float k2 = coefficients.k2;
    float k3 = coefficients.k3;
    float p1 = coefficients.p1;
    float p2 = coefficients.p2;

    // We need to correct for aspect ratio to ensure that
    // the distortion appears circular on the device.
    y /= View_Aspect_Ratio;

    float dx = x - distort_centre.x;
    float dy = y - distort_centre.y;
    float r2 = dx * dx + dy * dy;
    float r4 = r2 * r2;
    float r6 = r4 * r2;

    float radial_x = x * (k1 * r2 + k2 * r4 + k3 * r6);
    float radial_y = y * (k1 * r2 + k2 * r4 + k3 * r6);

    float tangential_x = p1 * (r2 + 2.0f*x*x) + 2.0f*p2*x*y;
    float tangential_y = p2 * (r2 + 2.0f*y*y) + 2.0f*p1*x*y;

    float distorted_x = x + radial_x + tangential_x;
    float distorted_y = y + radial_y + tangential_y;

    float result_x = 0.5f + tex_coord_factor * distorted_x;
    float result_y = 0.5f + tex_coord_factor * distorted_y * View_Aspect_Ratio;

    return vec2(result_x, result_y);
}

GLuint make_warp_mesh(LensConfig config)
{
    struct Vertex
    {
        vec2 position;
        vec2 uv_red_low_res;
        vec2 uv_green_low_res;
        vec2 uv_blue_low_res;
        vec2 uv_red_high_res;
        vec2 uv_green_high_res;
        vec2 uv_blue_high_res;
    };
    static Vertex v[(Warp_Mesh_Resolution_X + 1) * (Warp_Mesh_Resolution_Y + 1)];

    // Compute vertices
    int vi = 0;
    for (int yi = 0; yi <= Warp_Mesh_Resolution_Y; yi++)
    for (int xi = 0; xi <= Warp_Mesh_Resolution_X; xi++)
    {
        float x = -1.0f + 2.0f * xi / Warp_Mesh_Resolution_X;
        float y = -1.0f + 2.0f * yi / Warp_Mesh_Resolution_Y;
        v[vi].position = vec2(x, y) * config.fill_scale + config.image_centre;
        v[vi].uv_red_low_res   = compute_distortion(x, y, config.distort_centre, config.coefficients_red, 0.5f);
        v[vi].uv_green_low_res = compute_distortion(x, y, config.distort_centre, config.coefficients_green, 0.5f);
        v[vi].uv_blue_low_res  = compute_distortion(x, y, config.distort_centre, config.coefficients_blue, 0.5f);
        // The texture coordinates for the higher resolution texture go from -0.5 to 1.5 so
        // that only the center of the screen samples the high resolution texture.
        v[vi].uv_red_high_res   = compute_distortion(x, y, config.distort_centre, config.coefficients_red, 1.0f);
        v[vi].uv_green_high_res = compute_distortion(x, y, config.distort_centre, config.coefficients_green, 1.0f);
        v[vi].uv_blue_high_res  = compute_distortion(x, y, config.distort_centre, config.coefficients_blue, 1.0f);
        vi++;
    }

    // Generate faces from vertices
    static Vertex f[Warp_Mesh_Resolution_X * Warp_Mesh_Resolution_Y * 6];
    int fi = 0;
    for (int yi = 0; yi < Warp_Mesh_Resolution_Y; yi++)
    for (int xi = 0; xi < Warp_Mesh_Resolution_X; xi++)
    {
        Vertex v0 = v[(yi    ) * (Warp_Mesh_Resolution_X + 1) + xi    ];
        Vertex v1 = v[(yi    ) * (Warp_Mesh_Resolution_X + 1) + xi + 1];
        Vertex v2 = v[(yi + 1) * (Warp_Mesh_Resolution_X + 1) + xi + 1];
        Vertex v3 = v[(yi + 1) * (Warp_Mesh_Resolution_X + 1) + xi    ];
        f[fi++] = v0;
        f[fi++] = v1;
        f[fi++] = v2;
        f[fi++] = v2;
        f[fi++] = v3;
        f[fi++] = v0;
    }

    GLuint result = 0;
    GL_CHECK(glGenBuffers(1, &result));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, result));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(f), f, GL_STATIC_DRAW));
    return result;
}

// This computes a general frustum given four field-of-view (FOV)
// angles and the near and far clipping planes. The FOV angle is
// defined as the angle between the optical axis (i.e. the eye's
// forward direction) and the line from the eye to the respective
// edge of the screen.
mat4 make_frustum(float left_fov,
                  float right_fov,
                  float bottom_fov,
                  float top_fov,
                  float z_near,
                  float z_far)
{
    mat4 result(0.0f);
    float tt = tan(top_fov);
    float tb = tan(bottom_fov);
    float tl = tan(left_fov);
    float tr = tan(right_fov);
    result.x.x = 2.0f / (tl + tr);
    result.y.y = 2.0f / (tt + tb);
    result.z.x = (tl - tr) / (tl + tr);
    result.z.y = (tt - tb) / (tt + tb);
    result.z.z = (z_near + z_far) / (z_near - z_far);
    result.w.w = -1.0f;
    result.w.z = 2.0f * z_near * z_far / (z_near - z_far);
    return result;
}

// This computes a general frustum given the distance
// from the viewer's eye to the display, and the corners
// of that eye's screen half relative to the eye.
// z_near and z_far decide the near and far clipping planes.
mat4 make_frustum_screen_viewer(float eye_display_distance,
                                float left,
                                float right,
                                float bottom,
                                float top,
                                float z_near,
                                float z_far)
{
    mat4 result(0.0f);
    result.x.x = 2.0f * eye_display_distance / (right - left);
    result.y.y = 2.0f * eye_display_distance / (top - bottom);
    result.z.x = (right + left) / (right - left);
    result.z.y = (top + bottom) / (top - bottom);
    result.z.z = (z_near + z_far) / (z_near - z_far);
    result.z.w = -1.0f;
    result.w.z = 2.0f * z_near * z_far / (z_near - z_far);
    return result;
}

void app_initialize(App *app)
{
    // Make sure the required extensions are present.
    const GLubyte* extensions = glGetString(GL_EXTENSIONS);
    char * found_multiview2_extension = strstr ((const char*)extensions, "GL_OVR_multiview2");
    char * found_multisample_multiview_extension = strstr ((const char*)extensions, "GL_OVR_multiview_multisampled_render_to_texture");
    char * found_border_clamp_extension = strstr ((const char*)extensions, "GL_EXT_texture_border_clamp");

    if (found_multiview2_extension == NULL)
    {
        LOGI("OpenGL ES 3.0 implementation does not support GL_OVR_multiview2 extension.\n");
        exit(EXIT_FAILURE);
    }

    if (found_multisample_multiview_extension == NULL)
    {
        // If multisampled multiview is not supported, multisampling will not be used, so no need to exit here.
        LOGI("OpenGL ES 3.0 implementation does not support GL_OVR_multiview_multisampled_render_to_texture extension.\n");
    }

    if (found_border_clamp_extension == NULL)
    {
        LOGI("OpenGL ES 3.0 implementation does not support GL_EXT_texture_border_clamp extension.\n");
        exit(EXIT_FAILURE);
    }

    GL_CHECK(glGenVertexArrays(1, &app->vao));
    GL_CHECK(glBindVertexArray(app->vao));
    GL_CHECK(glViewport(0, 0, app->window_width, app->window_height));

    app->vbo_cube = make_cube();

    app->fb = make_eye_framebuffer(Eye_Fb_Resolution_X, Eye_Fb_Resolution_Y, Num_Views);

    // The coefficients below may be calibrated by photographing an
    // image containing straight lines, both vertical and horizontal,
    // through the lenses of the HMD, at the position where the viewer
    // would be looking through them.

    // Ideally, the user would be allowed to calibrate them for their
    // own eyes, through some calibration utility. The application should
    // then load a stored user-profile on runtime. For now, we hardcode
    // some values based on our calibration of the SM-R320 Gear VR
    // lenses.

    // Left lens
    app->hmd.left.coefficients_red.k1    = 0.19f;
    app->hmd.left.coefficients_red.k2    = 0.21f;
    app->hmd.left.coefficients_red.k3    = 0.0f;
    app->hmd.left.coefficients_red.p1    = 0.0f;
    app->hmd.left.coefficients_red.p2    = 0.0f;
    app->hmd.left.coefficients_green.k1  = 0.22f;
    app->hmd.left.coefficients_green.k2  = 0.24f;
    app->hmd.left.coefficients_green.k3  = 0.0f;
    app->hmd.left.coefficients_green.p1  = 0.0f;
    app->hmd.left.coefficients_green.p2  = 0.0f;
    app->hmd.left.coefficients_blue.k1   = 0.24f;
    app->hmd.left.coefficients_blue.k2   = 0.26f;
    app->hmd.left.coefficients_blue.k3   = 0.0f;
    app->hmd.left.coefficients_blue.p1   = 0.0f;
    app->hmd.left.coefficients_blue.p2   = 0.0f;

    // Right lens
    app->hmd.right.coefficients_red.k1   = 0.19f;
    app->hmd.right.coefficients_red.k2   = 0.21f;
    app->hmd.right.coefficients_red.k3   = 0.0f;
    app->hmd.right.coefficients_red.p1   = 0.0f;
    app->hmd.right.coefficients_red.p2   = 0.0f;
    app->hmd.right.coefficients_green.k1 = 0.22f;
    app->hmd.right.coefficients_green.k2 = 0.24f;
    app->hmd.right.coefficients_green.k3 = 0.0f;
    app->hmd.right.coefficients_green.p1 = 0.0f;
    app->hmd.right.coefficients_green.p2 = 0.0f;
    app->hmd.right.coefficients_blue.k1  = 0.24f;
    app->hmd.right.coefficients_blue.k2  = 0.26f;
    app->hmd.right.coefficients_blue.k3  = 0.0f;
    app->hmd.right.coefficients_blue.p1  = 0.0f;
    app->hmd.right.coefficients_blue.p2  = 0.0f;

    // These may be computed by measuring the distance between the top
    // of the unscaled distorted image and the top of the screen. Denote
    // this distance by Delta. The normalized view coordinate of the
    // distorted image top is
    //     Y = 1 - 2 Delta / Screen_Size_Y
    // We want to scale this coordinate such that it maps to the top of
    // the view. That is,
    //     Y * fill_scale = 1
    // Solving for fill_scale gives the equations below.
    float delta = Centimeter(0.7f);
    app->hmd.left.fill_scale  = 1.0f / (1.0f - 2.0f * delta / Screen_Size_Y);
    app->hmd.right.fill_scale = 1.0f / (1.0f - 2.0f * delta / Screen_Size_Y);

    // These are computed such that the centers of the displayed framebuffers
    // on the device are seperated by the viewer's IPD.
    app->hmd.left.image_centre    = vec2(+1.0f - Eye_IPD / (Screen_Size_X / 2.0f), 0.0f);
    app->hmd.right.image_centre   = vec2(-1.0f + Eye_IPD / (Screen_Size_X / 2.0f), 0.0f);

    // These are computed such that the distortion takes place around
    // an offset determined by the difference between lens seperation
    // and the viewer's eye IPD. If the difference is zero, the distortion
    // takes place around the image centre.
    app->hmd.left.distort_centre  = vec2((Lens_IPD - Eye_IPD) / (Screen_Size_X / 2.0f), 0.0f);
    app->hmd.right.distort_centre = vec2((Eye_IPD - Lens_IPD) / (Screen_Size_X / 2.0f), 0.0f);

    app->warp_mesh[0] = make_warp_mesh(app->hmd.left);
    app->warp_mesh[1] = make_warp_mesh(app->hmd.right);

    get_attrib_location( distort, position);
    get_attrib_location( distort, uv_red_low_res);
    get_attrib_location( distort, uv_green_low_res);
    get_attrib_location( distort, uv_blue_low_res);
    get_attrib_location( distort, uv_red_high_res);
    get_attrib_location( distort, uv_green_high_res);
    get_attrib_location( distort, uv_blue_high_res);
    get_uniform_location(distort, layer_index);
    get_uniform_location(distort, framebuffer);

    get_attrib_location (cube, position);
    get_attrib_location (cube, normal);
    get_uniform_location(cube, projection);
    get_uniform_location(cube, view);
    get_uniform_location(cube, model);
}

void draw_scene(App *app)
{
    int n = 5;
    for (int zi = 0; zi <= n; zi++)
    for (int xi = 0; xi <= n; xi++)
    {
        float x = Centimeter(-100.0f) + 2.0f * Centimeter(100.0f * xi / n);
        float z = Centimeter(-100.0f * zi);
        int i = zi * n + xi;
        mat4 mat_model = translate(x, 0.0f, z) *
                         scale(Centimeter(5.0f)) *
                         rotateY(0.3f * app->elapsed_time + i * 1.57f) *
                         rotateX(0.2f * app->elapsed_time + i * 3.2f);
        uniformm4(cube, model, mat_model);
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 36));
    }

    mat4 mat_model = translate(0.0f, Centimeter(70.0f), Centimeter(400.0f)) *
                     scale(Centimeter(5.0f)) *
                     rotateY(0.3f * app->elapsed_time) *
                     rotateX(0.05f * app->elapsed_time);
    uniformm4(cube, model, mat_model);
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 36));
}

void app_update_and_render(App *app)
{
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glDepthFunc(GL_LEQUAL));
    GL_CHECK(glDepthMask(GL_TRUE));
    GL_CHECK(glDepthRangef(0.0, 1.0));

    GL_CHECK(glClearDepthf(1.0f));
    GL_CHECK(glClearColor(0.15f, 0.17f, 0.2f, 1.0f));

    ////////////////////////////
    // Cube shader

    GL_CHECK(glUseProgram(app->program_cube));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, app->vbo_cube));
    attribfv(cube, position, 3, 6, 0);
    attribfv(cube, normal, 3, 6, 3);

    float camera_z = Centimeter(500.0f);
    float camera_y = Centimeter(70.0f);

    mat4 mat_view[Num_Views];
    mat_view[0] = translate(+Eye_IPD / 2.0f, -camera_y, -camera_z);
    mat_view[1] = translate(-Eye_IPD / 2.0f, -camera_y, -camera_z);
    mat_view[2] = translate(+Eye_IPD / 2.0f, -camera_y, -camera_z);
    mat_view[3] = translate(-Eye_IPD / 2.0f, -camera_y, -camera_z);

    // The scene is rendered twice for each eye position, once with a wide field of view, and
    // once with a narrower field of view in order to render the midpoint of the screen with
    // a higher resolution than the rest. 
    mat4 mat_projection[Num_Views];
    mat_projection[0] = make_frustum_screen_viewer(
                        Eye_Display_Distance,
                        -(Screen_Size_X - Eye_IPD) / 2.0f,
                        +(Eye_IPD / 2.0f),
                        -Screen_Size_Y / 2.0f,
                        +Screen_Size_Y / 2.0f,
                        Z_Near, Z_Far);
    mat_projection[1] = make_frustum_screen_viewer(
                        Eye_Display_Distance,
                        -(Eye_IPD / 2.0f),
                        +(Screen_Size_X - Eye_IPD) / 2.0f,
                        -Screen_Size_Y / 2.0f,
                        +Screen_Size_Y / 2.0f,
                        Z_Near, Z_Far);

    float right_midpoint = -((Screen_Size_X/4.0f) - (Eye_IPD / 2.0f));
    float left_midpoint = (Screen_Size_X/4.0f) - (Eye_IPD / 2.0f);
    mat_projection[2] = make_frustum_screen_viewer(
                        Eye_Display_Distance,
                        right_midpoint - (Screen_Size_X/8.0f),
                        right_midpoint + (Screen_Size_X/8.0f),
                        -Screen_Size_Y / 4.0f,
                        +Screen_Size_Y / 4.0f,
                        Z_Near, Z_Far);
    mat_projection[3] = make_frustum_screen_viewer(
                        Eye_Display_Distance,
                        left_midpoint - (Screen_Size_X/8.0f),
                        left_midpoint + (Screen_Size_X/8.0f),
                        -Screen_Size_Y / 4.0f,
                        +Screen_Size_Y / 4.0f,
                        Z_Near, Z_Far);

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, app->fb.framebuffer));
    GL_CHECK(glViewport(0, 0, app->fb.width, app->fb.height));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    uniformm4array(cube, projection, mat_projection[0], Num_Views);
    uniformm4array(cube, view, mat_view[0], Num_Views);
    draw_scene(app);

    ////////////////////////////
    // Distortion shader

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    GL_CHECK(glDisable(GL_DEPTH_TEST));
    GL_CHECK(glDepthMask(GL_FALSE));
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    GL_CHECK(glUseProgram(app->program_distort));
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    uniform1i(distort, framebuffer, 0);

    // Left eye
    GL_CHECK(glViewport(0, 0, View_Resolution_X, View_Resolution_Y));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, app->fb.colorbuffer));
    uniform1i(distort, layer_index, 0);
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, app->warp_mesh[0]));
    attribfv(distort, position,          2, 14, 0);
    attribfv(distort, uv_red_low_res,    2, 14, 2);
    attribfv(distort, uv_green_low_res,  2, 14, 4);
    attribfv(distort, uv_blue_low_res,   2, 14, 6);
    attribfv(distort, uv_red_high_res,   2, 14, 8);
    attribfv(distort, uv_green_high_res, 2, 14, 10);
    attribfv(distort, uv_blue_high_res,  2, 14, 12);
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, Warp_Mesh_Resolution_X * Warp_Mesh_Resolution_Y * 6));

    // Right eye
    GL_CHECK(glViewport(View_Resolution_X, 0, View_Resolution_X, View_Resolution_Y));
    uniform1i(distort, layer_index, 1);
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, app->warp_mesh[1]));
    attribfv(distort, position,          2, 14, 0);
    attribfv(distort, uv_red_low_res,    2, 14, 2);
    attribfv(distort, uv_green_low_res,  2, 14, 4);
    attribfv(distort, uv_blue_low_res,   2, 14, 6);
    attribfv(distort, uv_red_high_res,   2, 14, 8);
    attribfv(distort, uv_green_high_res, 2, 14, 10);
    attribfv(distort, uv_blue_high_res,  2, 14, 12);
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, Warp_Mesh_Resolution_X * Warp_Mesh_Resolution_Y * 6));
}
