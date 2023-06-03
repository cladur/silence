#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 light_space[6];
uniform int type; // 0 - point, 1 - directional, 2 - spot

out vec4 FragPos;

void main()
{
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        if (type == 0) {
            for (int i = 0; i < 3; ++i)
            {
                FragPos = gl_in[i].gl_Position;
                gl_Position = light_space[face] * FragPos;
                EmitVertex();
            }
        } else if (type == 1 || type == 2) {
            for (int i = 0; i < 3; ++i)
            {
                gl_Position = light_space[0] * gl_in[i].gl_Position;
                EmitVertex();
            }
        }
        EndPrimitive();
    }
}