#version 300 es
#extension GL_OVR_multiview2 : enable
// This proprietary software may be used only as
// authorised by a licensing agreement from ARM Limited
// (C) COPYRIGHT 2015 ARM Limited
// ALL RIGHTS RESERVED
// The entire notice above must be reproduced on all authorised
// copies and copies may only be made to the extent permitted
// by a licensing agreement from ARM Limited.

layout(num_views = 4) in;

in vec3 position;
in vec3 normal;
uniform mat4 projection[4];
uniform mat4 view[4];
uniform mat4 model;
out float v_depth;
out vec3 v_position;
out vec3 v_normal;

void main()
{
    vec4 view_pos = view[gl_ViewID_OVR] * model * vec4(position, 1.0);
    gl_Position = projection[gl_ViewID_OVR] * view_pos;
    v_normal = (view[gl_ViewID_OVR] * model * vec4(normal, 0.0)).xyz;
    v_depth = -view_pos.z;
    v_position = position;
}
