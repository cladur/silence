#version 460

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_texcoord;
layout (location = 3) in vec3 v_color;
layout (location = 4) in ivec4 v_skin_indices;
layout (location = 5) in vec4 v_skin_weights;

layout (location = 0) out vec3 out_color;
layout (location = 1) out vec2 out_texcoord;

layout (set = 0, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
}
CameraData;

struct ObjectData {
    mat4 model;
};

//all object matrices
layout (std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
    ObjectData objects[];
}
objectBuffer;

//push constants block
layout (push_constant) uniform constants {
    vec4 data;
    mat4 render_matrix;
}
PushConstants;


const int MAX_BONE_COUNT = 512;
//buffer to load bone_matrices from animator
layout (std140, set = 2, binding = 0) readonly buffer SkinningBuffer {
    mat4 bones[MAX_BONE_COUNT];
}
skin;

void main() {

    const vec4 pos = vec4(v_position, 1.0f);
    const vec4 norm = vec4(v_normal, 0.0f);
    vec4 pos_skinned = vec4(0.0f);
    vec4 norm_skinned = vec4(0.0f);

    for (int i = 0; i < 4; ++i) {
        if (v_skin_weights[i] > 0 && v_skin_indices[i] < MAX_BONE_COUNT) {
            const mat4 bone = skin.bones[v_skin_indices[i]];
            const float weight = v_skin_weights[i];

            pos_skinned += (bone * pos) * weight;
            norm_skinned += (bone * norm) * weight;
        }
    }

    mat4 model_matrix = objectBuffer.objects[gl_BaseInstance].model;
    mat4 transform_matrix = (CameraData.viewproj * model_matrix);
    gl_Position = transform_matrix * pos_skinned;
    out_color = v_color;
    out_texcoord = v_texcoord;
}