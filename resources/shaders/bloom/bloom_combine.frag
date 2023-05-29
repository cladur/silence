#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloom_tex;
uniform sampler2D dirt;
uniform sampler2D clut;

uniform int use_bloom;
uniform float bloom_strength = 0.04;
uniform float gamma = 2.2;
uniform float dirt_strength = 0.05;
uniform float aspect_ratio = 1.0;
uniform float dirt_offset = 0.0;

uniform int use_clut = 1;

#define MAXCOLOR 31.0
#define COLORS 32.0
#define WIDTH 1024.0
#define HEIGHT 32.0

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

    if (use_clut == 1)
    {
        float half_pixel_x = 0.5 / WIDTH;
        float half_pixel_y = 0.5 / HEIGHT;

        float cell = color.b * MAXCOLOR;
        float x_floor = floor(cell);
        float x_ceil = ceil(cell);

        float g_offset = half_pixel_x + color.g / COLORS * (MAXCOLOR / COLORS);
        float r_offset = half_pixel_y + (1.0 - color.r) * (MAXCOLOR / COLORS);

        vec2 lut_pos_l = vec2(x_floor / COLORS + g_offset, r_offset);
        vec2 lut_pos_r = vec2(x_ceil / COLORS + g_offset, r_offset);

        //lut_pos_l = clamp(lut_pos_l, vec2(0.0), vec2(1.0));
        //lut_pos_r = clamp(lut_pos_r, vec2(0.0), vec2(1.0));

        vec3 left_lut = texture(clut, lut_pos_l).rgb;
        vec3 right_lut = texture(clut, lut_pos_r).rgb;

        color = mix(left_lut, right_lut, fract(cell));
    }

    FragColor = vec4(color, 1.0);
}