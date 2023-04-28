#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Color;

uniform sampler2D sprite_texture;
uniform int textured;

void main()
{
    vec4 sampled = texture(sprite_texture, TexCoords);
//    if (sampled.a == 0) {
//        discard;
//    }
    // for flat-colored sprites
    if (textured == 0) {
        sampled = vec4(1.0, 1.0, 1.0, 1.0);
    }

    FragColor = vec4(Color, 1.0) * sampled;
}