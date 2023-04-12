#include "skinned_mesh.h"

#include "tiny_gltf.h"

SkinnedMesh::SkinnedMesh(const std::vector<SkinnedVertex> &vertices, const std::vector<uint32_t> &indices) :
		vertices(vertices), indices(indices) {
}
