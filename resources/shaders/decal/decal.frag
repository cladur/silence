#version 330 core
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec4 view_pos;
in vec4 clip_pos;
in vec2 uv;
// source is gBuffer
uniform sampler2D source_position;
uniform sampler2D source_normal;
uniform sampler2D decal_normal;
uniform sampler2D decal_albedo;

uniform vec4 color;
uniform vec2 aspect_ratio;
uniform mat4 decal_view_proj;
uniform mat4 inv_view;


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

    vec3 ViewNorm = texture(source_normal, depth_uv).rgb;
    vec3 world_norm = (inv_view * vec4(ViewNorm, 1.0)).xyz;
    gNormal = vec4(getNormalFromMap(decal_uv, world_norm), 1.0);
}