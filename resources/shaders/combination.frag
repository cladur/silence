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

uniform int use_fog;
uniform float fog_min;
uniform float fog_max;

uniform bool use_ao;

void main()
{
    vec3 albedo = texture(Albedo, TexCoords).rgb;
    vec3 diffuse = texture(Diffuse, TexCoords).rgb;
    vec3 specular = texture(Specular, TexCoords).rgb;
    float ssao = texture(SSAO, TexCoords).r;
    float ao = texture(AoRoughMetal, TexCoords).r;

    float final_ao = 1.0 - min((1.0 - ssao) + (1.0 - ao), 1.0);
    if (!use_ao) {
        final_ao = 1.0;
    }

    vec3 color = (albedo * diffuse + specular) * final_ao;
    // color = specular;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    if (use_fog == 1) {
        vec4 view_pos = texture(ViewPos, TexCoords);
        float distance = distance(vec3(0.0, 0.0, 0.0), view_pos.xyz);
        float fog_value = clamp((distance - fog_min) / fog_max, 0.0, 1.0);

        color = mix(color, vec3(0.3, 0.3, 0.3), fog_value);
    }

    FragColor = vec4(color, 1.0);
}