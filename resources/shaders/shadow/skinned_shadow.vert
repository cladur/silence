#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 3) in vec4 aWeights;
layout (location = 4) in ivec4 aBoneIds;

uniform mat4 light_space;
uniform mat4 model;

const int MAX_BONES = 512;
const int MAX_BONE_INFLUENCE = 4;
layout (binding = 1) uniform SkinningBuffer {
    mat4 bones[MAX_BONES];
} skin;

void main()
{
    vec4 pos = vec4(aPos, 1.0f);
    vec4 skinned_pos = vec4(0.0f);

    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if (aWeights[i] > 0.0f && aBoneIds[i] < MAX_BONES) {
            mat4 bone = skin.bones[aBoneIds[i]];
            float weight = aWeights[i];

            skinned_pos += (bone * pos) * weight;
        }
    }
    gl_Position = light_space * model * skinned_pos;
}