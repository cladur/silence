#version 460

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_color;
layout(location = 3) in vec2 v_texcoord;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_texcoord;
layout(location = 2) out vec3 out_normal;
layout(location = 3) out vec3 out_position;

layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
}
cameraData;

layout(set = 0, binding = 1) uniform SceneData {
    vec4 fog_color;// w is for exponent
    vec4 fog_distances;//x for min, y for max, zw unused.
    vec4 ambient_color;
    vec4 sunlight_direction;//w for sun power
    vec4 sunlight_color;
}
sceneData;

struct ObjectData {
    mat4 model;
    vec4 spherebounds;
    vec4 extents;
};

//all object matrices
layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
    ObjectData objects[];
}
objectBuffer;

//all object indices
layout(set = 1, binding = 1) readonly buffer InstanceBuffer {
    uint IDs[];
}
instanceBuffer;

//push constants block
layout(push_constant) uniform constants {
    vec4 data;
    mat4 render_matrix;
}
PushConstants;

void main() {
    uint index = instanceBuffer.IDs[gl_InstanceIndex];

    mat4 model_matrix = objectBuffer.objects[index].model;
    mat4 transform_matrix = (cameraData.viewproj * model_matrix);
    gl_Position = transform_matrix * vec4(v_position, 1.0f);
    out_color = v_color;
    out_texcoord = v_texcoord;
    out_normal = (model_matrix * vec4(v_normal, 0.0f)).xyz;
    out_position = (model_matrix * vec4(v_position, 1.0f)).xyz;
}