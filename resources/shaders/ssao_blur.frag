#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D ssao_texture;
uniform vec2 offset_step;

const float kernel[9] = float[](
    0.1, 0.1, 0.1,
    0.1, 0.2, 0.1,
    0.1, 0.1, 0.1
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
    vec3 result = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < 9; i++)
    {
        result += texture(ssao_texture, TexCoords + offsets[i]).rgb * kernel[i];
    }
    FragColor = vec4(vec3(result), 1.0);
}