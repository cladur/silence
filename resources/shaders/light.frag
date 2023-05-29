#version 330 core
layout (location = 0) out vec4 Diffuse;
layout (location = 1) out vec4 Specular;

// material parameters
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gAoRoughMetal;

// lights
uniform vec3 light_position;
uniform vec3 light_direction;
uniform vec3 light_color;
uniform float light_intensity;
uniform float cutoff;
uniform float outer_cutoff;
uniform int type;

uniform vec2 screen_dimensions;

uniform mat4 view;
uniform vec3 camPos;

const float PI = 3.14159265359;
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
void CalcDirLight(vec3 normal, vec3 view_pos, vec3 F0, float roughness, float metalness, vec3 albedo)
{
    vec3 light_dir = normalize(light_direction);
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

    vec3 diffuse_light = (kD * albedo / PI) * light_color * NdotL;
    vec3 specular_light = (specular * light_color * NdotL);

    //    float shadow = ShadowCalculation(worldPosLightSpace, normal,  lightDir);

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
    vec3 radiance = light_intensity * light_color * attenuation;

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

    vec3 diffuse_light = (kD * albedo / PI) * radiance * NdotL;
    vec3 specular_light = (specular * radiance * NdotL);

    Diffuse = vec4(diffuse_light, 0.0);
    Specular = vec4(specular_light, 0.0);
}

// ----------------------------------------------------------------------------
void CalcSpotLight(vec3 world_pos, vec3 normal, vec3 view_pos, vec3 F0, float roughness, float metalness, vec3 albedo)
{
    vec3 light_dir = normalize(light_position - world_pos);
    vec3 halfway_dir = normalize(view_pos + light_dir);

    float theta = dot(light_dir, normalize(-light_direction));
    float epsilon = cutoff - outer_cutoff;
    float intensity = clamp((theta - outer_cutoff) / epsilon, 0.0, 1.0);

    float distance = length(light_position - world_pos);
    float attenuation = 1.0;// / (distance * distance);
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

    vec3 diffuse_light = (kD * albedo / PI) * radiance * NdotL;
    vec3 specular_light = (specular * radiance * NdotL);

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

    if (type == 0)
    {
        CalcPointLight(WorldPos, N, V, F0, roughness, metallic, albedo);
    } else if (type == 1) {
        CalcDirLight(N, V, F0, roughness, metallic, albedo);
    } else if (type == 2) {
        CalcSpotLight(WorldPos, N, V, F0, roughness, metallic, albedo);
    }
}