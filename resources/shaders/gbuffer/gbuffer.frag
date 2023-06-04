#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gAoRoughMetal;
layout (location = 4) out vec4 gDepth;
layout (location = 5) out vec3 gHighlight;

in vec2 TexCoords;
in vec3 ViewPos;
in vec3 Normal;

// material parameters
uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D ao_metallic_roughness_map;
uniform sampler2D emissive_map;

uniform bool has_ao_map;
uniform bool has_normal_map;
uniform bool has_emissive_map;
uniform bool is_highlighted;
uniform vec3 highlight_color;

uniform vec2 uv_scale;

// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anyways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap(vec2 texutre_coords)
{

    vec3 tangentNormal = texture(normal_map, texutre_coords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(ViewPos);
    vec3 Q2 = dFdy(ViewPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// ----------------------------------------------------------------------------
void main()
{
    vec2 texture_coords = vec2(TexCoords.x * uv_scale.x, TexCoords.y * uv_scale.y);
    vec4 color = texture(albedo_map, texture_coords);
    vec3 albedo = pow(color.rgb, vec3(2.2));
    if (color.a < 0.05f)
    {
        discard;
    }
    vec3 ao_metallic_roughness = texture(ao_metallic_roughness_map, texture_coords).rgb;
    float roughness = ao_metallic_roughness.g;
    float metallic = ao_metallic_roughness.b;
    vec3 normal = Normal;
    if (has_normal_map) {
        normal = getNormalFromMap(texture_coords);
    }

    float ao = 1.0;
    if (has_ao_map) {
        ao = ao_metallic_roughness.r;
    }

    vec3 highlight = vec3(0.0);

    if (is_highlighted) {
        highlight = highlight_color;
    }

    gPosition = vec4(ViewPos, 1.0);
    gNormal = vec4(normal, 1.0);
    gAoRoughMetal = vec4(ao, roughness, metallic, 1.0);
    gAlbedo = vec4(albedo, 1.0);
    gDepth = vec4(-vec3(ViewPos.z), 1.0);
    gHighlight = highlight;
}
