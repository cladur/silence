#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Color;

uniform sampler2D _texture;
uniform int textured;
uniform int is_sprite;
uniform float alpha;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, 1.0);
    if (textured == 1) {
        if (is_sprite == 0) {
            // if the sprite is text, we just need the alpha and white color
            sampled = vec4(1.0, 1.0, 1.0, texture(_texture, TexCoords).r);
        } else {
            sampled = texture(_texture, TexCoords);
        }
    }

    FragColor = vec4(Color, alpha) * sampled;
}