#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D lut;

// from https://lettier.github.io/3d-game-shaders-for-beginners/lookup-table.html

void main()
{
    vec3 color = texture(scene, TexCoords).rgb;

    float u  =  floor(color.b * 31.0) / 31.0 * 993.0;
    u  = (floor(color.r * 31.0) / 31.0 *  31.0) + u;
    u /= 1023.0;
    float v  = (floor(color.g * 31.0) / 31.0);

    vec3 left = texture(lut, vec2(u, v)).rgb;

    u  =  ceil(color.b * 31.0) / 31.0 * 993.0;
    u  = (ceil(color.r * 31.0) / 31.0 *  31.0) + u;
    u /= 1023.0;
    v  = (ceil(color.g * 31.0) / 31.0);

    vec3 right = texture(lut, vec2(u, v)).rgb;

    color.r = mix(left.r, right.r, fract(color.r * 31.0));
    color.g = mix(left.g, right.g, fract(color.g * 31.0));
    color.b = mix(left.b, right.b, fract(color.b * 31.0));

    FragColor = vec4(color, 1.0);
}