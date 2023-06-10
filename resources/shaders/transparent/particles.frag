#version 430 core

out vec4 FragColor;

in vec2 TexCoords;
in vec4 Color;
in float viewZ;
flat in int ID;

uniform sampler2D particle_tex;
uniform sampler2D depth;
uniform vec2 screen_size;
uniform float smooth_size;
uniform float far_sub_near;
uniform float far;
uniform float near;
uniform int is_textured;

layout (std430, binding = 3) buffer ssbo_data {
    vec4 positions[256]; // X, Y, Z, SIZE
    mat4 rotations[256];
    vec4 colors[256];
    vec4 up[256];       // vec3 + 1*float padding
    vec4 right[256];    // vec3 + 1*float padding
};

float calc_depth( in float z )
{
    return (z - near) / (far - near);
}

void main()
{
    vec4 sampled = vec4(1.0);
    if (is_textured == 1) {
        sampled = texture(particle_tex, TexCoords);
    }
    vec2 depth_uv = vec2(gl_FragCoord.x / screen_size.x, gl_FragCoord.y / screen_size.y);
    vec4 depth_sampled = texture(depth, depth_uv);
    float scene_depth = calc_depth(depth_sampled.x);
    if (scene_depth < 0.0001) {
        scene_depth = 1;
    }
    float particle_depth = calc_depth(viewZ);
    float diff = clamp((scene_depth - particle_depth) / (smooth_size / 100.0), 0.0, 1.0);

    FragColor = Color * sampled * vec4(1.0, 1.0, 1.0, diff);
}