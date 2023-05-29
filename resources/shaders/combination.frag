#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// material parameters
uniform sampler2D Albedo;
uniform sampler2D Diffuse;
uniform sampler2D Specular;
uniform sampler2D SSAO;
uniform sampler2D AoRoughMetal;
uniform sampler2D ViewPos;
uniform sampler2D Skybox;
uniform sampler2D clut;

uniform int use_fog;
uniform float fog_min;
uniform float fog_max;

uniform bool use_ao;
uniform int use_clut = 1;

void main()
{
    vec3 albedo = texture(Albedo, TexCoords).rgb;
    vec3 diffuse = texture(Diffuse, TexCoords).rgb;
    vec3 specular = texture(Specular, TexCoords).rgb;
    float ssao = texture(SSAO, TexCoords).r;
    float ao = texture(AoRoughMetal, TexCoords).r;
    vec3 skybox = texture(Skybox, TexCoords).rgb;
    vec4 view_pos = texture(ViewPos, TexCoords);

    if (
    albedo == vec3(0.0, 0.0, 0.0) && diffuse == vec3(0.0, 0.0, 0.0) && view_pos.xyz == vec3(0.0, 0.0, 0.0)
    ) {
        FragColor = vec4(skybox, 1.0);
        return;
    }

    float final_ao = min(ssao, ao);
    if (!use_ao) {
        final_ao = 1.0;
    }

    vec3 color = (albedo * diffuse + specular) * final_ao;
    // color = specular;

    if (use_fog == 1) {
        vec4 view_pos = view_pos;
        float distance = distance(vec3(0.0, 0.0, 0.0), view_pos.xyz);
        float fog_value = clamp((distance - fog_min) / fog_max, 0.0, 1.0);

        color = mix(color, vec3(0.3, 0.3, 0.3), fog_value);
    }

    if (use_clut == 1)
    {
        // LUT consists of 32 boxes of 32 x 32 colors
        float x = floor(color.b * 31.0) / 31.0 * 992.0; // quantized blue
        x += (floor(color.g * 31.0) / 31.0 * 31.0); // quantized green offset
        x /= 1023.0;

        //color = vec3(x);
        float y = 1.0 - (ceil(color.r * 31.0) / 31.0);

        vec3 left_lut = texture(clut, vec2(x, y)).rgb;


        x = ceil(color.b * 31.0) / 31.0 * 992.0; // quantized blue
        x += (ceil(color.g * 31.0) / 31.0 * 31.0);
        x /= 1023.0;

        y = 1.0 - (ceil(color.r * 31.0) / 31.0);

        vec3 right_lut = texture(clut, vec2(x, y)).rgb;
        color = right_lut;
//        color.r = mix(left_lut.r, right_lut.r, fract(color.r * 31.0));
//        color.g = mix(left_lut.g, right_lut.g, fract(color.g * 31.0));
//        color.b = mix(left_lut.b, right_lut.b, fract(color.b * 31.0));
    }

    FragColor = vec4(color, 1.0);
}