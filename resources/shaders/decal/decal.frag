#version 330 core
layout (location = 0) out vec4 gAlbedo;

in vec4 clip_pos;

uniform sampler2D gDepth;
uniform sampler2D decal_albedo;

uniform mat4 inv_view_proj;
uniform mat4 decal_view_proj;
uniform vec2 aspect_ratio;

vec3 world_position_from_depth(vec2 screen_pos, float ndc_depth)
{
    // Remap depth to [-1.0, 1.0] range.
    float depth = ndc_depth * 2.0 - 1.0;

    // // Create NDC position.
    vec4 ndc_pos = vec4(screen_pos, depth, 1.0);

    // Transform back into world position.
    vec4 world_pos = inv_view_proj * ndc_pos;

    // Undo projection.
    world_pos = world_pos / world_pos.w;

    return world_pos.xyz;
}


void main()
{
    vec2 screen_pos = clip_pos.xy / clip_pos.w;
    vec2 tex_coords = screen_pos * 0.5 + 0.5;

    float depth = texture(gDepth, tex_coords).x;
    vec3 world_pos = world_position_from_depth(screen_pos, depth);

    vec4 ndc_pos = decal_view_proj * vec4(world_pos, 1.0);
    ndc_pos.xyz /= ndc_pos.w;

    ndc_pos.xy *= aspect_ratio;

    if (ndc_pos.x < -1.0 || ndc_pos.x > 1.0 || ndc_pos.y < -1.0 || ndc_pos.y > 1.0)
    {
        discard;
    }

    vec2 decal_tex_coord = ndc_pos.xy * 0.5 + 0.5;
    decal_tex_coord.x = 1.0 - decal_tex_coord.x;

    vec4 albedo = texture(decal_albedo, decal_tex_coord);

    if (albedo.a < 0.1)
    {
        discard;
    }

    gAlbedo = albedo;
}