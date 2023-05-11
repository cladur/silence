#version 330 core
out vec4 FragColor;

struct s_material {
    float shininess;
};

struct s_dir_light {
    vec3 direction;
    float ambient;
    float intensity;
    vec3 color;
    bool enabled;
};

in vec2 text_coord;
in vec3 out_normal;
in vec3 frag_pos;

uniform sampler2D our_texture;

#define SLC 2 // spot light count

uniform vec3 view_pos;
uniform s_dir_light dir_light;
uniform s_material material;

vec3 calculate_directional_lights(s_dir_light light, vec3 normal, vec3 view_dir);

void main()
{
    vec3 norm = normalize(out_normal);
    vec3 view_dir = normalize(view_pos - frag_pos);

    vec3 result = vec3(0.0f);

    if (dir_light.enabled)
    {
        result += calculate_directional_lights(dir_light, norm, view_dir);
    }

    FragColor = vec4(result, 1.0) * vec4(0.8, 0.2, 0.3, 1.0);
}

vec3 calculate_directional_lights(s_dir_light light, vec3 normal, vec3 view_dir) {
    vec3 light_dir = normalize(-light.direction);
    vec3 halfway_dir = normalize(light_dir + view_dir);

    float diff = max(dot(normal, light_dir), 0.0);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), material.shininess);

    vec3 ambient = light.ambient * light.color;
    vec3 diffuse = light.intensity * diff * light.color;
    vec3 specular = spec * light.color;
    return (ambient + diffuse + specular);
}
