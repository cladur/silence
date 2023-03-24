#ifndef SILENCE_VK_MESH_H
#define SILENCE_VK_MESH_H

#include <vector>

#include "glm/vec3.hpp"
#include "vk_types.h"

struct VertexInputDescription {
	std::vector<vk::VertexInputBindingDescription> bindings;
	std::vector<vk::VertexInputAttributeDescription> attributes;

	vk::PipelineVertexInputStateCreateFlags flags = {};
};

struct Vertex {
	glm::vec3 position;
	[[maybe_unused]] float padding1;
	glm::vec3 normal;
	[[maybe_unused]] float padding2;
	glm::vec3 color;
	[[maybe_unused]] float padding3;

	static VertexInputDescription get_vertex_description();
};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	AllocatedBuffer vertex_buffer;
	AllocatedBuffer index_buffer;

	bool load_from_gltf(const char *filename);
};

#endif //SILENCE_VK_MESH_H
