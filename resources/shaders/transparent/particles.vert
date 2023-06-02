#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec4 Color;
out int ID;
out float viewZ;

uniform mat4 projection;
uniform mat4 view;

uniform vec3 camera_look;
uniform vec3 camera_pos;
uniform vec3 camera_right;
uniform vec3 raw_position;

layout (std430, binding = 3) buffer ssbo_data {
    vec4 positions[256]; // X, Y, Z, SIZE
    mat4 rotations[256];
    vec4 colors[256];
};

void main() {

    int id = gl_InstanceID;
    mat4 vp = projection * view;
    vec3 pos = vec3(rotations[id] * (positions[id].w * vec4(aPos, 1.0)));
    vec3 look_x0z = normalize(vec3(camera_look.x, 0, camera_look.z));

    vec3 new_right = normalize ( cross(
    normalize(
    camera_pos - raw_position
    ),
    vec3(0, 1, 0)
    ) );

    vec4 world_space_pos = vec4(
        positions[id].xyz + // position
        camera_right * pos.x +              // right
        vec3(0, 1, 0) * pos.y +          // up
        look_x0z * pos.z, 1.0            // forward
    );
    gl_Position = vp * world_space_pos;

    viewZ = -(view * world_space_pos).z;

    TexCoords = aTexCoords;
    Color = colors[id];
    ID = id;
}