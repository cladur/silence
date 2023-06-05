#version 330 core

out vec4 output_color;

in vec2 TexCoords;
in vec3 ViewPos;
in vec3 Normal;

uniform vec3 cam_pos;

uniform int is_highlighted;
uniform vec3 highlight_color;
uniform int agent_hacker_pov; //            0 - agent, 1 - hacker
uniform int highlight_target; // 0 - other, 1 - agent, 2 - hacker

uniform int is_xray;

uniform sampler2D depth;

void main() {
    vec4 color = vec4(0.0f);
    if (is_highlighted == 1) {
        float diffuse = max(dot(normalize(cam_pos - ViewPos), Normal), 0.0);
        switch (highlight_target) {
            case 0:
                color = vec4(highlight_color, 1.0f) * diffuse;
                break;
            case 1:
                if (agent_hacker_pov != 0) {
                    color = vec4(highlight_color, 1.0f) * diffuse;
                }
                break;
            case 2:
                if (agent_hacker_pov != 1) {
                    color = vec4(highlight_color, 1.0f) * diffuse;
                }
                break;
        }

        // only render highlights that are OBSCURED by other objects.
        // otherwise dont check anything
        if (is_xray == 1) {
            float depth_v = texture(depth, TexCoords).r;
            if (depth_v > gl_FragCoord.z) {
                color = vec4(0.0f);
            }
        }
    } else {
        color = vec4(0.0f);
    }

    output_color = color;
}