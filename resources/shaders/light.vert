#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec4 world_light_space;

void main() {
	vec4 WorldPos = model * vec4(aPos, 1.0f);
	WorldPos.w = 1.0f;

	gl_Position = projection * view * WorldPos;
}
