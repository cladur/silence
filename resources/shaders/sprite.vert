#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in int is_screen_space;

out vec2 TexCoords;
out vec3 Color;

uniform mat4 projection;
uniform mat4 view;

void main() {
    if (is_screen_space == 1) {
        gl_Position = projection * vec4(aPos, 1.0);
    } else {
        gl_Position = projection * view * vec4(aPos, 1.0);
    }

    TexCoords = aTexCoords;
    Color = aColor;
}