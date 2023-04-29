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

struct RenderBounds {
	glm::vec3 origin;
	float radius;
	glm::vec3 extents;
	bool valid;
};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	AllocatedBufferUntyped vertex_buffer;
	AllocatedBufferUntyped index_buffer;

	RenderBounds bounds;

	bool load_from_asset(const char *filename);
};

#endif //SILENCE_VK_MESH_H
