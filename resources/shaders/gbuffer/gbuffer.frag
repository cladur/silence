#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gAoRoughMetal;
layout (location = 4) out vec4 gDepth;

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

uniform mat4 view;
uniform mat4 inv_view;

uniform vec2 uv_scale;
uniform bool flip_uv_y;

uniform vec3 color_tint;
uniform float brightness_offset;

// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal
// mapping the usual way for performance anyways; I do plan make a note of this
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap(vec3 WorldPos, vec2 texture_coords)
{

    vec3 tangentNormal = texture(normal_map, texture_coords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(WorldPos);
    vec3 Q2 = dFdy(WorldPos);
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
    // Calculate WorldPos from ViewPos
    vec4 clipPos = vec4(ViewPos, 1.0);
    vec4 ndcPos = clipPos / clipPos.w;
    vec3 WorldPos = (inv_view * ndcPos).xyz;

    float uv_y = TexCoords.y;
    if (flip_uv_y) {
        uv_y = 1.0 - TexCoords.y;
    }
    vec2 texture_coords = vec2(TexCoords.x * uv_scale.x, uv_y * uv_scale.y);
    vec4 color = texture(albedo_map, texture_coords);
    vec3 albedo = pow((brightness_offset + color.rgb) * color_tint, vec3(2.2));
    if (color.a < 0.05f)
    {
        discard;
    }
    vec3 ao_metallic_roughness = texture(ao_metallic_roughness_map, texture_coords).rgb;
    float roughness = ao_metallic_roughness.g;
    float metallic = ao_metallic_roughness.b;
    vec3 normal = normalize(Normal);
    if (has_normal_map) {
        normal = getNormalFromMap(WorldPos, texture_coords);
    }

    float ao = 1.0;
    if (has_ao_map) {
        ao = ao_metallic_roughness.r;
    }

    gPosition = vec4(ViewPos, 1.0);
    gNormal = vec4(normal, 1.0);
    gAoRoughMetal = vec4(ao, roughness, metallic, 1.0);
    gAlbedo = vec4(albedo, 1.0);
    gDepth = vec4(-vec3(ViewPos.z), 1.0);
}
