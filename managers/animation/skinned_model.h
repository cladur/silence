#ifndef SILENCE_SKINNED_MODEL_H
#define SILENCE_SKINNED_MODEL_H

#include "joint.h"
#include "skinned_mesh.h"
#include <tiny_gltf.h>

class SkinnedModel {
public:
	std::vector<SkinnedMesh> meshes;
	std::unordered_map<std::string, Joint> joint_map;

	bool load_model(const char *filename);

	void process_node(const tinygltf::Node &node, int32_t node_id);

	void process_mesh(const tinygltf::Mesh &mesh);

	void process_skin(const tinygltf::Skin &skin);

private:
	tinygltf::Model model;
};

#endif //SILENCE_SKINNED_MODEL_H
