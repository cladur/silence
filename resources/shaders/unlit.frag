#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform sampler2D albedo_map;

uniform vec3 camPos;

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
void main()
{
    // material properties
    vec4 albedo = texture(albedo_map, TexCoords).rgba;

    if (albedo.a < 0.02) {
        discard;
    }

    FragColor = vec4(albedo.rgb, 1.0);
}
