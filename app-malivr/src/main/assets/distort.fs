#version 300 es
// This proprietary software may be used only as
// authorised by a licensing agreement from ARM Limited
// (C) COPYRIGHT 2015 ARM Limited
// ALL RIGHTS RESERVED
// The entire notice above must be reproduced on all authorised
// copies and copies may only be made to the extent permitted
// by a licensing agreement from ARM Limited.
precision highp float;
precision mediump int;
precision mediump sampler2DArray;

in vec2 texel_r_low_res;
in vec2 texel_g_low_res;
in vec2 texel_b_low_res;
in vec2 texel_r_high_res;
in vec2 texel_g_high_res;
in vec2 texel_b_high_res;
uniform sampler2DArray framebuffer;
uniform int layer_index;
out vec4 f_color;

vec3 sample_per_channel(vec2 tex_coord_r, vec2 tex_coord_g, vec2 tex_coord_b, int layer)
{
    vec3 sampled_color;
    sampled_color.r = texture(framebuffer, vec3(tex_coord_r, layer)).r;
    sampled_color.g = texture(framebuffer, vec3(tex_coord_g, layer)).g;
    sampled_color.b = texture(framebuffer, vec3(tex_coord_b, layer)).b;

    return sampled_color;
}

float interpolate_color(vec2 tex_coord, float low_res_color_val, float high_res_color_val)
{
    // Using squared distance to middle of screen for interpolating.
    vec2 dist_vec = vec2(0.5) - tex_coord;
    float squared_dist = dot(dist_vec, dist_vec);
    // Using the high res texture when distance from center is less than 0.5 in texture coordinates (0.25 is 0.5 squared).
    // When the distance is less than 0.2 (0.04 is 0.2 squared), only the high res texture will be used.
    float lerp_val = smoothstep(-0.25, -0.4, -squared_dist);
    return mix(low_res_color_val, high_res_color_val, lerp_val);
}

void main()
{
    vec3 low_res_color = sample_per_channel(texel_r_low_res, texel_g_low_res, texel_b_low_res, layer_index);
    vec3 high_res_color = sample_per_channel(texel_r_high_res, texel_g_high_res, texel_b_high_res, layer_index + 2);

    f_color.r = interpolate_color(texel_r_high_res, low_res_color.r, high_res_color.r);
    f_color.g = interpolate_color(texel_g_high_res, low_res_color.g, high_res_color.g);
    f_color.b = interpolate_color(texel_b_high_res, low_res_color.b, high_res_color.b);
    f_color.a = 1.0;
}
