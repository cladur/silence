#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D ssao_texture;
uniform vec2 offset_step;
uniform int should_blur;

const float kernel[9] = float[](
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0
);

void main()
{
    vec2 offsets[9] = vec2[](
        vec2(-offset_step.x, -offset_step.y), 
        vec2(0, -offset_step.y), 
        vec2(offset_step.x, -offset_step.y),
        vec2(-offset_step.x, 0),
        vec2(0, 0),
        vec2(offset_step.x, 0),
        vec2(-offset_step.x, offset_step.y),
        vec2(0, offset_step.y),
        vec2(offset_step.x, offset_step.y)
    );
    if (should_blur == 0) {
        FragColor = vec4(vec3(texture(ssao_texture, TexCoords).rgb), 1.0);
    } else {
        vec3 result = vec3(0.0, 0.0, 0.0);
        for(int i = 0; i < 9; i++)
        {
            result += texture(ssao_texture, TexCoords + offsets[i]).rgb * kernel[i];
        }
        result /= 9.0;
        FragColor = vec4(vec3(result), 1.0);
    }
}