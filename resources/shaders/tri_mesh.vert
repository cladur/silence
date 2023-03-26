#version 450

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec3 v_color;

layout (location = 0) out vec3 out_color;

layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
} CameraData;

layout(push_constant) uniform constants {
    vec4 data;
    mat4 render_matrix;
} PushConstants;

void main() {
    mat4 transform_matrix = (CameraData.viewproj * PushConstants.render_matrix);
    gl_Position = transform_matrix * vec4(v_position, 1.0f);
    out_color = v_color;
}