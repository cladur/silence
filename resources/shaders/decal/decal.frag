#version 330 core
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gAoRoughMetal;

in vec4 view_pos;
in vec4 clip_pos;
in vec2 uv;
// source is gBuffer
uniform sampler2D source_position;
uniform sampler2D source_normal;
uniform sampler2D source_ao_rough_metal;
uniform sampler2D decal_normal;
uniform sampler2D decal_albedo;
uniform sampler2D decal_ao_rough_metal;

uniform mat4 decal_view_proj;
uniform mat4 inv_view;
uniform vec4 color;
uniform vec3 face_normal;
uniform vec2 aspect_ratio;
uniform bool has_normal;
uniform bool use_face_normal;
uniform bool has_ao;
uniform bool has_roughness;
uniform bool has_metalness;


vec3 getNormalFromMap(vec2 texture_coords, vec3 surface_normal)
{
    vec3 tangentNormal = texture(decal_normal, texture_coords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(view_pos.xyz);
    vec3 Q2 = dFdy(view_pos.xyz);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 N = normalize(surface_normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{
    vec2 screen_pos = clip_pos.xy / clip_pos.w;
    vec2 depth_uv = screen_pos * 0.5 + 0.5;

    // Calculate WorldPos from ViewPos
    vec3 ViewPos = texture(source_position, depth_uv).rgb;
    vec3 world_pos = (inv_view * vec4(ViewPos, 1.0)).xyz;

    vec4 ndc_pos = decal_view_proj * vec4(world_pos, 1.0);
    ndc_pos.xyz /= ndc_pos.w;

    ndc_pos.xy *= aspect_ratio;

    if (ndc_pos.x < -1.0 || ndc_pos.x > 1.0 || ndc_pos.y < -1.0 || ndc_pos.y > 1.0 || ndc_pos.z < -1.0 || ndc_pos.z > 1.0)
    {
        discard;
    }

    vec2 decal_uv = ndc_pos.xy * 0.5 + 0.5;
    decal_uv.x = 1.0 - decal_uv.x;

    vec4 albedo = texture(decal_albedo, decal_uv) * color;

    if (albedo.a < 0.1)
    {
        discard;
    }

    gAlbedo = albedo;

    if (has_normal)
    {
        vec3 surface_normal;
        if (use_face_normal)
        {
            surface_normal = face_normal;
        }
        else
        {
            surface_normal = texture(source_normal, depth_uv).rgb;
        }
        gNormal = vec4(getNormalFromMap(decal_uv, surface_normal), 1.0);
    } else {
        vec3 ViewNorm = texture(source_normal, depth_uv).rgb;
        gNormal = vec4(ViewNorm, 1.0);
    }
    vec3 ao_metallic_roughness;
    bool has_any = has_ao || has_roughness || has_metalness;
    if (has_any)
    {
        ao_metallic_roughness = texture(decal_ao_rough_metal, decal_uv).rgb;
    }
    vec3 s_ao_metallic_roughness = texture(source_ao_rough_metal, depth_uv).rgb;
    float ao;
    if (has_ao) {
        ao = ao_metallic_roughness.r;
    } else {
        ao = s_ao_metallic_roughness.r;
    }
    float roughness;
    if (has_roughness) {
        roughness = ao_metallic_roughness.g;
    } else {
        roughness = s_ao_metallic_roughness.g;
    }
    float metalness;
    if (has_metalness) {
        metalness = ao_metallic_roughness.b;
    } else {
        metalness = s_ao_metallic_roughness.b;
    }
    gAoRoughMetal = vec4(ao, roughness, metalness, 1.0);
}