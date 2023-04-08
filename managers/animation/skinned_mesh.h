#ifndef SILENCE_SKINNED_MESH_H
#define SILENCE_SKINNED_MESH_H

#include "render/vk_mesh.h"

struct SkinnedVertex : public Vertex {
	glm::ivec4 bone_ids;
	glm::vec4 weights;
};

class SkinnedMesh {
public:
	std::vector<SkinnedVertex> vertices;
	std::vector<uint32_t> indices;

	SkinnedMesh(const std::vector<SkinnedVertex> &vertices, const std::vector<uint32_t> &indices);

	AllocatedBuffer vertex_buffer;
	AllocatedBuffer index_buffer;
};

#endif //SILENCE_SKINNED_MESH_H
