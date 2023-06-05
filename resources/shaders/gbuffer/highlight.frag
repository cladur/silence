#version 330 core

layout (location = 0) out vec4 output_color;
layout (location = 1) out vec3 depth;

in vec2 TexCoords;
in vec3 ViewPos;
in vec3 Normal;

uniform int is_highlighted;
uniform vec3 highlight_color;
uniform int agent_hacker_pov; // 0 - agent, 1 - hacker
uniform int highlight_target; // 0 - agent, 1 - hacker, 2 - enemy, 3 - other

uniform float near;
uniform float far;

uniform int is_xray;

float calc_depth( in float z )
{
    return (z - near) / (far - near);
}

void main() {
    vec4 color = vec4(0,0,0,0);
    if (is_highlighted == 1) {
        switch (highlight_target) {
            case 0:
                if (agent_hacker_pov != 0) {
                    color = vec4(highlight_color, 0.0f);
                }
                break;
            case 1:
                if (agent_hacker_pov != 1) {
                    color = vec4(highlight_color, 0.0f);
                }
                break;
            case 2:
                color = vec4(highlight_color, 0.0f);
                break;
            case 3:
                color = vec4(highlight_color, 0.0f);
                break;
        }
        if (is_xray == 1) {
            color.a = 1.0;
        }
    } else {
        color = vec4(0.0f);
    }
    depth = vec3(-ViewPos.z);
    output_color = color;
}