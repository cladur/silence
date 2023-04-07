#version 460

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_color;
layout(location = 3) in vec2 v_texcoord;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_texcoord;

layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
}
CameraData;

struct ObjectData {
    mat4 model;
};

//all object matrices
layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
    ObjectData objects[];
}
objectBuffer;

void main()
{
    //gl_Position = CameraData.viewproj * ObjectData.model * vec4(v_position, 1.0);
    mat4 model_matrix = objectBuffer.objects[gl_BaseInstance].model;
    mat4 transform_matrix = (CameraData.viewproj * model_matrix);
    gl_Position = transform_matrix * vec4(v_position, 1.0f);
    out_texcoord = v_texcoord;
}