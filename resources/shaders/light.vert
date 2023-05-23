#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
	vec3 WorldPos = vec3(model * vec4(aPos, 1.0));

	gl_Position = projection * view * vec4(WorldPos, 1.0);
}
