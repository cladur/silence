#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloom_tex;

uniform int use_bloom;
uniform float bloom_strength = 0.04;
uniform float gamma = 2.2;

void main()
{
    vec3 color = texture(scene, TexCoords).rgb;
    if (use_bloom == 1)
    {
        vec3 bloom = texture(bloom_tex, TexCoords).rgb;
        color = mix(color, bloom, bloom_strength);
    }

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/gamma));

    FragColor = vec4(color, 1.0);
}