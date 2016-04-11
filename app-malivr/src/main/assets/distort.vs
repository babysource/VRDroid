#version 300 es
// This proprietary software may be used only as
// authorised by a licensing agreement from ARM Limited
// (C) COPYRIGHT 2015 ARM Limited
// ALL RIGHTS RESERVED
// The entire notice above must be reproduced on all authorised
// copies and copies may only be made to the extent permitted
// by a licensing agreement from ARM Limited.

in vec2 position;
in vec2 uv_red_low_res;
in vec2 uv_green_low_res;
in vec2 uv_blue_low_res;
in vec2 uv_red_high_res;
in vec2 uv_green_high_res;
in vec2 uv_blue_high_res;
out vec2 texel_r_low_res;
out vec2 texel_g_low_res;
out vec2 texel_b_low_res;
out vec2 texel_r_high_res;
out vec2 texel_g_high_res;
out vec2 texel_b_high_res;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    texel_r_low_res = uv_red_low_res;
    texel_g_low_res = uv_green_low_res;
    texel_b_low_res = uv_blue_low_res;
    texel_r_high_res = uv_red_high_res;
    texel_g_high_res = uv_green_high_res;
    texel_b_high_res = uv_blue_high_res;
}
