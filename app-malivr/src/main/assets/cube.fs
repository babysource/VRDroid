#version 300 es
// This proprietary software may be used only as
// authorised by a licensing agreement from ARM Limited
// (C) COPYRIGHT 2015 ARM Limited
// ALL RIGHTS RESERVED
// The entire notice above must be reproduced on all authorised
// copies and copies may only be made to the extent permitted
// by a licensing agreement from ARM Limited.
precision highp float;

in vec3 v_position;
in vec3 v_normal;
in float v_depth;
out vec4 f_color;

vec3 light(vec3 n, vec3 l, vec3 c)
{
    float ndotl = max(dot(n, l), 0.0);
    return ndotl * c;
}

void main()
{
    vec3 albedo = vec3(0.95, 0.84, 0.62);
    vec3 n = normalize(v_normal);
    f_color.rgb = vec3(0.0);
    f_color.rgb += light(n, normalize(vec3(1.0)), vec3(1.0));
    f_color.rgb += light(n, normalize(vec3(-1.0, -1.0, 0.0)), vec3(0.2, 0.23, 0.35));

    // Fog
    f_color.rgb *= 1.0 - smoothstep(0.9, 10.0, v_depth);

    // Gamma
    f_color.rgb = sqrt(f_color.rgb);

    f_color.a = 1.0;
}
