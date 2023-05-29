#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloom_tex;
uniform sampler2D dirt;

uniform int use_bloom;
uniform float bloom_strength = 0.04;
uniform float gamma = 2.2;
uniform float dirt_strength = 0.05;
uniform float aspect_ratio = 1.0;
uniform float dirt_offset = 0.0;

void main()
{
    vec3 color = texture(scene, TexCoords).rgb;
    if (use_bloom == 1)
    {
        // create texcoords that scale with the screen
        vec2 texcoords = TexCoords * vec2(aspect_ratio, 1.0);
        texcoords.x += dirt_offset;
        vec3 bloom = texture(bloom_tex, TexCoords).rgb;
        //vec3 dirt = texture(dirt, texcoords).rgb;
        color = mix(color, bloom, bloom_strength);
        //color = mix(color, bloom * dirt, dirt_strength);
    }

    // HDR tonemapping
    color = color / (color + vec3(1.0));

    // gamma correct
    color = pow(color, vec3(1.0/gamma));

    FragColor = vec4(color, 1.0);
}