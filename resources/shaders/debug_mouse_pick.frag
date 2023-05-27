#version 330 core
out uint FragColor;
flat in uint Entity;

void main()
{
    FragColor = Entity;
}
