#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in int is_screen_space;

out vec2 TexCoords;
out vec3 Color;

uniform mat4 projection;
uniform mat4 view;

uniform vec3 camera_up;
uniform vec3 camera_right;
uniform vec3 camera_look;
uniform vec3 camera_pos;

uniform vec3 billboard_center;
uniform int is_billboard;
uniform int use_camera_right;

uniform float billboard_z_offset;

void main() {
    if (is_screen_space == 1) {
        gl_Position = projection * vec4(aPos, 1.0);
    } else {
        mat4 vp = projection * view;
        vec3 right = camera_right;
        vec3 look = normalize(billboard_center - camera_pos);

        if (use_camera_right == 0) {
            right = normalize(
                cross(
                    normalize(
                        camera_pos - billboard_center
                    ),
                    vec3(0, 1, 0)
                )
            );
        }


        if (is_billboard == 1) {
            gl_Position = vp * vec4(billboard_center + right * aPos.x + camera_up * aPos.y + look * (aPos.z + billboard_z_offset), 1.0);
        } else {
            gl_Position = vp * vec4(aPos, 1.0);
        }
    }

    TexCoords = aTexCoords;
    Color = aColor;
}