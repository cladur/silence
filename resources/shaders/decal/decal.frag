#version 330 core
layout (location = 2) out vec4 gAlbedo;

in vec4 clip_pos;

uniform sampler2D gPosition;
uniform sampler2D gDepth;
uniform sampler2D decal_albedo;

uniform mat4 inv_view_proj;
uniform mat4 decal_view_proj;
uniform vec2 aspect_ratio;
uniform vec2 pixel_size;

uniform mat4 view;

void main()
{
    vec2 screen_pos = clip_pos.xy / clip_pos.w;
    vec2 depth_uv = screen_pos * 0.5 + 0.5;

    float depth = texture(gDepth, depth_uv).x;

    // Calculate WorldPos from ViewPos
    vec3 ViewPos = texture(gPosition, depth_uv).rgb;
    vec4 clipPos = vec4(ViewPos, 1.0);
    vec4 ndcPos = clipPos / clipPos.w;
    vec3 world_pos = (inverse(view) * ndcPos).xyz;

    vec4 ndc_pos = decal_view_proj * vec4(world_pos, 1.0);
    ndc_pos.xyz /= ndc_pos.w;

    ndc_pos.xy *= aspect_ratio;

    if (ndc_pos.x < -1.0 || ndc_pos.x > 1.0 || ndc_pos.y < -1.0 || ndc_pos.y > 1.0)
    {
        discard;
    }

    vec2 decal_uv = ndc_pos.xy * 0.5 + 0.5;
    decal_uv.x = 1.0 - decal_uv.x;

    vec4 albedo = texture(decal_albedo, decal_uv);

    if (albedo.a < 0.1)
    {
        discard;
    }

    gAlbedo = albedo;
}