#version 330 core
layout (location = 0) out vec4 Diffuse;
layout (location = 1) out vec4 Specular;

// material parameters
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gAoRoughMetal;
uniform sampler2D shadowMap;
uniform samplerCube depthMap;

uniform mat4 view;
uniform mat4 light_space;
// lights
uniform vec3 light_position;
uniform vec3 light_direction;
uniform vec3 light_color;
uniform vec3 camPos;
uniform vec2 screen_dimensions;
uniform vec2 spot_bias;
uniform vec2 volumetric_bias;
uniform float light_intensity;
uniform float light_blend_distance;
uniform float light_radius;
uniform float cutoff;
uniform float outer_cutoff;
uniform float far_plane;
uniform float scattering;
uniform int num_steps;
uniform int type;
uniform bool cast_shadow;
uniform bool cast_volumetric;

const mat4 DITHER_PATTERN = mat4
(vec4(0.0f, 0.5f, 0.125f, 0.625f),
 vec4(0.75f, 0.22f, 0.875f, 0.375f),
 vec4(0.1875f, 0.6875f, 0.0625f, 0.5625f),
 vec4(0.9375f, 0.4375f, 0.8125f, 0.3125f));
vec4 world_light_space;
const float PI = 3.14159265359;

float get_light_edge_blend(float distance) {
    float a = light_radius;
    float b = light_radius - light_blend_distance;

    // Value from 0 to 1, where 0 is at the edge of the light and 1 is at light_radius - blend_distance
    float blend_x = (1.0 / (b - a)) * (distance - a);

    blend_x = clamp(blend_x, 0.0, 1.0);

    float blend = smoothstep(0.0, 1.0, blend_x);

    return blend;
}

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------

vec2 CalcTexCoord()
{
    return gl_FragCoord.xy / screen_dimensions;
}
// ----------------------------------------------------------------------------
const vec3 gridSamplingDisk[20] = vec3[]
(
    vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, -1.0f, 1.0f), vec3(-1.0f, -1.0f, 1.0f), vec3(-1.0f, 1.0f, 1.0f),
    vec3(1.0f, 1.0f, -1.0f), vec3(1.0f, -1.0f, -1.0f), vec3(-1.0f, -1.0f, -1.0f), vec3(-1.0f, 1.0f, -1.0f),
    vec3(1.0f, 1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(-1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f),
    vec3(1.0f, 0.0f, 1.0f), vec3(-1.0f, 0.0f, 1.0f), vec3(1.0f, 0.0f, -1.0f), vec3(-1.0f, 0.0f, -1.0f),
    vec3(0.0f, 1.0f, 1.0f), vec3(0.0f, -1.0f, 1.0f), vec3(0.0f, -1.0f, -1.0f), vec3(0.0f, 1.0f, -1.0f)
);

float ShadowPointCalculation(vec3 world_pos)
{
    vec3 fragToLight = world_pos - light_position;
    float currentDepth = length(fragToLight);
    float shadow = 0.0f;
    float bias = 0.15f;
    int samples = 20;
    float viewDistance = length(camPos - world_pos);
    float diskRadius = (1.0f + (viewDistance / far_plane)) / 25.0f;
    for (int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if (currentDepth - bias > closestDepth)
        {
            shadow += 1.0f;
        }
    }
    shadow /= float(samples);

    return shadow;
}
// ----------------------------------------------------------------------------
float ShadowCalculation(vec3 normal, vec3 light_dir)
{
    // perform perspective divide
    vec3 projCoords = world_light_space.xyz / world_light_space.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5f + 0.5f;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow

    float shadow = 0.0f;
    float bias;
    if (type == 2) {
        bias = max(spot_bias.x * (1.0f - dot(normal, light_dir)), spot_bias.y);
    } else if (type == 1) {
        bias = max(0.025f * (1.0f - dot(normal, light_dir)), 0.005f);
    }
    vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
        }
    }
    shadow /= 9.0f;

    if (projCoords.z > 1.0f)
    {
        shadow = 0.0f;
    }

    return shadow;
}
// ----------------------------------------------------------------------------
float CalcScattering(float cosTheta)
{
    float scattering2 = scattering * scattering;
    return (1.0f - scattering2) / (4.0f * PI * pow(1.0f + scattering2 - 2.0f * scattering * cosTheta, 1.5f));
}
// ----------------------------------------------------------------------------
vec3 CalcVolume(vec3 world_pos, vec3 light_dir, vec2 tex_coords)
{
    vec3 ray_vector = world_pos - camPos;
    float ray_length = length(ray_vector);
    vec3 ray_dir = ray_vector / ray_length;
    float step_size = ray_length / float(num_steps);
    vec3 step = ray_dir * step_size;

    vec3 position = camPos;
    position += step * DITHER_PATTERN[int(tex_coords.x * screen_dimensions.x) % 4][int(tex_coords.y * screen_dimensions.y) % 4];
    vec3 volumetric = vec3(0.0f);
    for (int i = 0; i < num_steps; ++i)
    {
        vec4 lightSpacePos = light_space * vec4(position, 1.0f);
        vec3 lightSpacePosPostW = lightSpacePos.xyz / lightSpacePos.w * 0.5f + 0.5f;
        float depth = texture(shadowMap, lightSpacePosPostW.xy).r;
        float d = dot(ray_dir, light_dir);
        float bias = max(volumetric_bias.x * d, volumetric_bias.y);
        if (depth > lightSpacePosPostW.z + bias)
        {
            volumetric += CalcScattering(d);
        }
        position += step;
    }

    return volumetric / float(num_steps);
}

// ----------------------------------------------------------------------------
void CalcDirLight(vec3 world_pos, vec3 normal, vec3 view_pos, vec3 F0, float roughness, float metalness, vec3 albedo, vec2 tex_coords)
{
    vec3 light_dir = normalize(-light_direction);
    vec3 halfway_dir = normalize(light_dir + view_pos);

    float NDF = DistributionGGX(normal, halfway_dir, roughness);
    float G = GeometrySmith(normal, view_pos, light_dir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfway_dir, view_pos), 0.0), F0);

    float NdotL = max(dot(normal, light_dir), 0.0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, view_pos), 0.0) * NdotL + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;

    float shadow = cast_shadow ? ShadowCalculation(normal, light_dir) : 0.0f;
    vec3 diffuse_light = (kD * albedo / PI) * light_color * light_intensity * NdotL * (1.0f - shadow);

    vec3 volumetric = cast_volumetric ? CalcVolume(world_pos, light_dir, tex_coords) : vec3(0.0f);
    vec3 specular_light = specular * light_color * light_intensity * NdotL * (1.0f - shadow) + volumetric * light_color;

    Diffuse = vec4(diffuse_light, 0.0);
    Specular = vec4(specular_light, 0.0);
}
// ----------------------------------------------------------------------------
void CalcPointLight(vec3 world_pos, vec3 normal, vec3 view_pos, vec3 F0, float roughness, float metalness, vec3 albedo)
{
    vec3 light_dir = normalize(light_position - world_pos);
    vec3 halfway_dir = normalize(view_pos + light_dir);

    float distance = length(light_position - world_pos);
    float attenuation = 1.0 / (distance * distance);

    vec3 radiance = get_light_edge_blend(distance) * light_intensity * light_color * attenuation;

    float NDF = DistributionGGX(normal, halfway_dir, roughness);
    float G = GeometrySmith(normal, view_pos, light_dir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfway_dir, view_pos), 0.0), F0);

    float NdotL = max(dot(normal, light_dir), 0.0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, view_pos), 0.0) * NdotL + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;

    float shadow = cast_shadow ? ShadowPointCalculation(world_pos) : 0.0f;

    vec3 diffuse_light = (kD * albedo / PI) * radiance * NdotL * (1.0f - shadow);
    vec3 specular_light = (specular * radiance * NdotL) * (1.0f - shadow);

    Diffuse = vec4(diffuse_light, 0.0);
    Specular = vec4(specular_light, 0.0);
}
// ----------------------------------------------------------------------------
void CalcSpotLight(vec3 world_pos, vec3 normal, vec3 view_pos, vec3 F0, float roughness, float metalness, vec3 albedo, vec2 tex_coords)
{
    vec3 light_dir = normalize(light_position - world_pos);
    vec3 halfway_dir = normalize(view_pos + light_dir);

    float theta = dot(light_dir, normalize(-light_direction));
    float epsilon = cutoff - outer_cutoff;
    float intensity = clamp((theta - outer_cutoff) / epsilon, 0.0, 1.0);

    float distance = length(light_position - world_pos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = intensity * light_intensity * light_color * attenuation;

    float NDF = DistributionGGX(normal, halfway_dir, roughness);
    float G = GeometrySmith(normal, view_pos, light_dir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfway_dir, view_pos), 0.0), F0);

    float NdotL = max(dot(normal, light_dir), 0.0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, view_pos), 0.0) * NdotL + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;

    float shadow = cast_shadow ? ShadowCalculation(normal, light_dir) : 0.0f;
    vec3 diffuse_light = (kD * albedo / PI) * radiance * NdotL * (1.0f - shadow);

    vec3 volumetric = cast_volumetric ? CalcVolume(world_pos, light_dir, tex_coords) : vec3(0.0f);
    vec3 specular_light = (specular * radiance * NdotL) * (1.0f - shadow) + volumetric * radiance;

    Diffuse = vec4(diffuse_light, 0.0);
    Specular = vec4(specular_light, 0.0);
}

void main()
{
    vec2 TexCoords = CalcTexCoord();
    vec3 ViewPos = texture(gPosition, TexCoords).rgb;

    // Calculate WorldPos from ViewPos
    vec4 clipPos = vec4(ViewPos, 1.0);
    vec4 ndcPos = clipPos / clipPos.w;
    vec3 WorldPos = (inverse(view) * ndcPos).xyz;

    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    vec3 ao_metallic_roughness = texture(gAoRoughMetal, TexCoords).rgb;
    float roughness = ao_metallic_roughness.g;
    float metallic = ao_metallic_roughness.b;

    vec3 N = texture(gNormal, TexCoords).rgb;
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    if (type == 0) {
        CalcPointLight(WorldPos, N, V, F0, roughness, metallic, albedo);
    } else if (type == 1) {
        world_light_space = light_space * vec4(WorldPos, 1.0);
        CalcDirLight(WorldPos, N, V, F0, roughness, metallic, albedo, TexCoords);
    } else if (type == 2) {
        world_light_space = light_space * vec4(WorldPos, 1.0);
        CalcSpotLight(WorldPos, N, V, F0, roughness, metallic, albedo, TexCoords);
    }
}