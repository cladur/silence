#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Color;

uniform sampler2D font_atlas_map;
uniform float alpha;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(font_atlas_map, TexCoords).r);
    FragColor = vec4(Color, alpha) * sampled;
}