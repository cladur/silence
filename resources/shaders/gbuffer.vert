#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 ViewPos;
out vec3 Normal;
out float Depth;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 camPos;

void main() {
	TexCoords = aTexCoords;
	ViewPos = vec3(view * model * vec4(aPos, 1.0));
	Normal = mat3(model) * aNormal;

	Depth = distance(aPos, camPos);

	gl_Position = projection * vec4(ViewPos, 1.0);
}
