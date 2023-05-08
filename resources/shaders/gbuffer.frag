#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gAoRoughMetal;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D ao_metallic_roughness_map;
uniform sampler2D emissive_map;

uniform bool has_ao_map;
uniform bool has_emissive_map;

uniform vec3 camPos;


// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anyways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normal_map, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// ----------------------------------------------------------------------------
void main()
{
    vec3 albedo = pow(texture(albedo_map, TexCoords).rgb, vec3(2.2));
    vec3 ao_metallic_roughness = texture(ao_metallic_roughness_map, TexCoords).rgb;
    float roughness = ao_metallic_roughness.g;
    float metallic = ao_metallic_roughness.b;

    float ao = 1.0;
    if (has_ao_map) {
        ao = ao_metallic_roughness.r;
    }

    gPosition = vec4(WorldPos, 1.0);
    gNormal = vec4(getNormalFromMap(), 1.0);
    gAlbedo = vec4(albedo, 1.0);
    gAoRoughMetal = vec4(ao, roughness, metallic, 1.0);
}
