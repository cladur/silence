#version 450

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec2 v_uv;
layout(location = 2) in vec3 v_color;
layout(location = 3) in int v_screen_space;

layout(location = 0) out vec2 out_texcoord;
layout(location = 1) out vec3 out_color;

layout(set = 0, binding = 0) uniform CameraBuffer {
	mat4 view;
	mat4 proj;
	mat4 viewproj;
}
cameraData;

layout(set = 0, binding = 1) uniform SceneData {
	vec4 fog_color; // w is for exponent
	vec4 fog_distances; //x for min, y for max, zw unused.
	vec4 ambient_color;
	vec4 sunlight_direction; //w for sun power
	vec4 sunlight_color;
}
sceneData;

void main() {
	if (v_screen_space == 1) {
		gl_Position = vec4(v_position, 1.0);
	} else {
		gl_Position = cameraData.viewproj * vec4(v_position, 1.0);
	}
	out_texcoord = v_uv;
	out_color = v_color;
}