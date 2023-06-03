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
uniform sampler2D Particles;

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
    vec3 skybox = texture(Skybox, TexCoords).rgb;
    vec4 view_pos = texture(ViewPos, TexCoords);
    vec4 particles = texture(Particles, TexCoords);

    if (
    albedo == vec3(0.0, 0.0, 0.0) && diffuse == vec3(0.0, 0.0, 0.0) && view_pos.xyz == vec3(0.0, 0.0, 0.0)
    ) {
        FragColor = vec4(mix(skybox, particles.rgb, particles.a), 1.0);
        return;
    }

    float final_ao = min(ssao, ao);
    if (!use_ao) {
        final_ao = 1.0;
    }

    vec4 color = vec4((albedo * diffuse + specular) * final_ao, 1.0);

    if (use_fog == 1) {
        vec4 view_pos = view_pos;
        float distance = distance(vec3(0.0, 0.0, 0.0), view_pos.xyz);
        float fog_value = clamp((distance - fog_min) / fog_max, 0.0, 1.0);

        color = mix(color, vec4(0.3, 0.3, 0.3, color.a), fog_value);
    }

    // blending particles based on alpha
    color = mix(color, particles, particles.a);

    FragColor = vec4(color.rgb, 1.0);
}