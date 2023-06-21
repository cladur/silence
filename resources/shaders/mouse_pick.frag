#version 330 core
layout (location = 0) out uint FragColor;

in vec2 TexCoords;
in vec3 ViewPos;
in vec3 Normal;

uniform uint entity_id;


void main()
{
    FragColor = entity_id;
}
