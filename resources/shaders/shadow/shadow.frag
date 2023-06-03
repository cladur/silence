#version 330 core
in vec4 FragPos;

uniform vec3 light_pos;
uniform float far_plane;
uniform int type; // 0 - point, 1 - directional, 2 - spot

void main()
{
    if (type == 1 || type == 2) {
        gl_FragDepth = gl_FragCoord.z;
    } else if (type == 0) {
        float lightDistance = length(FragPos.xyz - light_pos) / far_plane;

        gl_FragDepth = lightDistance;
    }
}