#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec4 view_pos;
out vec4 clip_pos;
out vec2 uv;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 decal_inv_view_proj;

void main()
{
    vec4 world_pos = decal_inv_view_proj * vec4(aPos, 1.0);
    view_pos = view * world_pos;
    clip_pos = projection * view_pos;
    gl_Position = clip_pos;
    uv = aTexCoord;
}