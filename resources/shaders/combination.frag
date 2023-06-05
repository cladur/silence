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

uniform sampler2D Highlights;
uniform sampler2D HighlightsDepth;
uniform sampler2D Depth;

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
    vec4 highlights = texture(Highlights, TexCoords);
    vec4 highlights_depth = texture(HighlightsDepth, TexCoords);
    vec4 depth = texture(Depth, TexCoords);

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

    if (length(highlights.rgb) > 0.0) {
        float highlight_power = clamp(((length(color.rgb) + 0.05) / 3.0), 0.0, 1.0);
        if (highlights.a < 0.9) {
            if (depth.r + 0.8 > highlights_depth.r) {
                highlight_power = 0.0;
            }
            highlight_power *= 4.0; // highlight xray entities a bit more
            clamp(highlight_power, 0.0, 1.0);
        }
        color = mix(color, highlights, highlight_power);
    }

    FragColor = vec4(color.rgb, 1.0);
}