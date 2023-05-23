#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec4 aWeights;
layout(location = 4) in ivec4 aBoneIds;

out vec2 TexCoords;
out vec3 ViewPos;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 512;
const int MAX_BONE_INFLUENCE = 4;
layout(binding = 1) uniform SkinningBuffer {
	mat4 bones[MAX_BONES];
}
skin;

void main() {
	vec4 pos = vec4(aPos, 1.0f);
	vec4 norm = vec4(aNormal, 0.0f);
	vec4 skinned_pos = vec4(0.0f);
	vec4 skinned_norm = vec4(0.0f);

	for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
		if (aWeights[i] > 0.0f && aBoneIds[i] < MAX_BONES) {
			mat4 bone = skin.bones[aBoneIds[i]];
			float weight = aWeights[i];

			skinned_pos += (bone * pos) * weight;
			skinned_norm += (bone * norm) * weight;
		}
	}

	TexCoords = aTexCoords;
	ViewPos = vec3(view * model * skinned_pos);
	Normal = vec3(model * skinned_norm);

	gl_Position = projection * vec4(ViewPos, 1.0);
}
