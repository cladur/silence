#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloom_tex;

uniform int use_bloom;
uniform float bloom_strength = 0.04;

void main()
{
    vec4 result = texture(scene, TexCoords);
    if (use_bloom == 1)
    {
        vec4 bloom = texture(bloom_tex, TexCoords);
        result = mix(result, bloom, bloom_strength);
    }
    FragColor = result;
}