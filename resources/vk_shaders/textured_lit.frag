#version 450

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_texcoord;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec3 in_position;

//output write
layout (location = 0) out vec4 out_frag_color;

layout(set = 0, binding = 1) uniform SceneData {
    vec4 fog_color;// w is for exponent
    vec4 fog_distances;//x for min, y for max, zw unused.
    vec4 ambient_color;
    vec4 sunlight_direction;//w for sun power
    vec4 sunlight_color;
} sceneData;

layout(set = 2, binding = 0) uniform sampler2D albedo_map;
layout(set = 2, binding = 1) uniform sampler2D ao_map;
layout(set = 2, binding = 2) uniform sampler2D normal_map;
layout(set = 2, binding = 3) uniform sampler2D metallic_roughness_map;
layout(set = 2, binding = 4) uniform sampler2D emissive_map;

vec3 get_normal_from_normal_map()
{
    vec3 tangentNormal = texture(normal_map, in_texcoord).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(in_position);
    vec3 Q2 = dFdy(in_position);
    vec2 st1 = dFdx(in_texcoord);
    vec2 st2 = dFdy(in_texcoord);

    vec3 N = normalize(in_normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{
    vec3 color = texture(albedo_map, in_texcoord).rgb;
    vec3 ao = texture(ao_map, in_texcoord).rgb;
    vec3 emissive = texture(emissive_map, in_texcoord).rgb;
    vec3 normal = get_normal_from_normal_map();
    float metallic = texture(metallic_roughness_map, in_texcoord).b;
    float roughness = texture(metallic_roughness_map, in_texcoord).g;

    color = pow(color, vec3(2.2));

    color *= ao;

    float light = max(dot(sceneData.sunlight_direction.xyz, normal), 0.0);
    color *= light;

    color = max(color, emissive);

    out_frag_color = vec4(color, 1.0f);
}
