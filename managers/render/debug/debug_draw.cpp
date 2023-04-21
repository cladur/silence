#include "debug_draw.h"
#include "render/vk_debug.h"
#include <render/render_manager.h>
#include <render/vk_initializers.h>
#include <vulkan/vulkan_handles.hpp>

VertexInputDescription DebugVertex::get_vertex_description() {
	VertexInputDescription description;

	//we will have just 1 vertex buffer binding, with a per-vertex rate
	vk::VertexInputBindingDescription main_binding = {};
	main_binding.binding = 0;
	main_binding.stride = sizeof(DebugVertex);
	main_binding.inputRate = vk::VertexInputRate::eVertex;

	description.bindings.push_back(main_binding);

	//Position will be stored at Location 0
	vk::VertexInputAttributeDescription position_attribute = {};
	position_attribute.binding = 0;
	position_attribute.location = 0;
	position_attribute.format = vk::Format::eR32G32B32Sfloat;
	position_attribute.offset = offsetof(DebugVertex, position);

	//Color will be stored at Location 1
	vk::VertexInputAttributeDescription color_attribute = {};
	color_attribute.binding = 0;
	color_attribute.location = 1;
	color_attribute.format = vk::Format::eR32G32B32Sfloat;
	color_attribute.offset = offsetof(DebugVertex, color);

	description.attributes.push_back(position_attribute);
	description.attributes.push_back(color_attribute);

	return description;
}

namespace debug_draw {

void draw_line(const glm::vec3 &from, const glm::vec3 &to, const glm::vec3 &color) {
	auto render_manager = RenderManager::get();

	render_manager->debug_vertices.push_back({ from, color });
	render_manager->debug_vertices.push_back({ to, color });
}

// Draw a box with center at "center" and scale "scale".
void draw_box(const glm::vec3 &center, const glm::vec3 &scale, const glm::vec3 &color) {
	// Generate a box mesh, using draw_line to draw the lines
	glm::vec3 vertices[8] = { glm::vec3(
									  center.x - scale.x / 2.0f, center.y - scale.y / 2.0f, center.z - scale.z / 2.0f),
		glm::vec3(center.x + scale.x / 2.0f, center.y - scale.y / 2.0f, center.z - scale.z / 2.0f),
		glm::vec3(center.x + scale.x / 2.0f, center.y + scale.y / 2.0f, center.z - scale.z / 2.0f),
		glm::vec3(center.x - scale.x / 2.0f, center.y + scale.y / 2.0f, center.z - scale.z / 2.0f),
		glm::vec3(center.x - scale.x / 2.0f, center.y - scale.y / 2.0f, center.z + scale.z / 2.0f),
		glm::vec3(center.x + scale.x / 2.0f, center.y - scale.y / 2.0f, center.z + scale.z / 2.0f),
		glm::vec3(center.x + scale.x / 2.0f, center.y + scale.y / 2.0f, center.z + scale.z / 2.0f),
		glm::vec3(center.x - scale.x / 2.0f, center.y + scale.y / 2.0f, center.z + scale.z / 2.0f) };

	draw_line(vertices[0], vertices[1], color);
	draw_line(vertices[1], vertices[2], color);
	draw_line(vertices[2], vertices[3], color);
	draw_line(vertices[3], vertices[0], color);
	draw_line(vertices[4], vertices[5], color);
	draw_line(vertices[5], vertices[6], color);
	draw_line(vertices[6], vertices[7], color);
	draw_line(vertices[7], vertices[4], color);
	draw_line(vertices[0], vertices[4], color);
	draw_line(vertices[1], vertices[5], color);
	draw_line(vertices[2], vertices[6], color);
	draw_line(vertices[3], vertices[7], color);
}

// Draw a sphere with center at "center" and radius "radius".
void draw_sphere(const glm::vec3 &center, float radius, const glm::vec3 &color) {
	// Generate a sphere mesh, using draw_line to draw the lines
	const int num_segments = 8; // number of horizontal segments
	const int num_rings = 4; // number of vertical rings
	const float pi = 3.14159265358979323846;
	const float delta_ring = pi / num_rings;
	const float delta_segment = 2.0f * pi / num_segments;

	// generate vertices
	float vertices[num_rings + 1][num_segments + 1][3];
	for (int i = 0; i <= num_rings; i++) {
		float radius = sin(i * delta_ring);
		for (int j = 0; j <= num_segments; j++) {
			vertices[i][j][0] = radius * cos(j * delta_segment) + center.x;
			vertices[i][j][1] = cos(i * delta_ring) + center.y;
			vertices[i][j][2] = radius * sin(j * delta_segment) + center.z;
		}
	}

	// generate line segments
	for (int i = 0; i < num_rings; i++) {
		for (int j = 0; j < num_segments; j++) {
			int i1 = i;
			int i2 = i + 1;
			int j1 = j;
			int j2 = (j + 1) % num_segments;
			draw_line(glm::vec3(vertices[i1][j1][0], vertices[i1][j1][1], vertices[i1][j1][2]),
					glm::vec3(vertices[i2][j1][0], vertices[i2][j1][1], vertices[i2][j1][2]), color);
			draw_line(glm::vec3(vertices[i2][j1][0], vertices[i2][j1][1], vertices[i2][j1][2]),
					glm::vec3(vertices[i2][j2][0], vertices[i2][j2][1], vertices[i2][j2][2]), color);
		}
	}
}

}; //namespace debug_draw