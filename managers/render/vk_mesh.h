#ifndef SILENCE_VK_MESH_H
#define SILENCE_VK_MESH_H

#include "vk_types.h"

struct VertexInputDescription {
	std::vector<vk::VertexInputBindingDescription> bindings;
	std::vector<vk::VertexInputAttributeDescription> attributes;

	vk::PipelineVertexInputStateCreateFlags flags = {};
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;

	static VertexInputDescription get_vertex_description();
};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	AllocatedBuffer vertex_buffer;
	AllocatedBuffer index_buffer;

	bool load_from_asset(const char *filename);
};

#endif //SILENCE_VK_MESH_H
