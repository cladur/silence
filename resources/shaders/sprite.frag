#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Color;
flat in int Textured;

uniform sampler2D sprite_texture;

void main()
{
    vec4 sampled = texture(sprite_texture, TexCoords);
    // for flat-colored sprites
    if (Textured == 0) {
        sampled = vec4(1.0, 1.0, 1.0, 1.0);
    }
    FragColor = vec4(Color, 1.0) * sampled;
}