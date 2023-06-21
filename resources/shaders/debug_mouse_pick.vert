#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in uint aEntity;

flat out uint Entity;

uniform mat4 projection;
uniform mat4 view;

void main() {
	Entity = aEntity;
	gl_Position = projection * view * vec4(aPos, 1.0);
}
