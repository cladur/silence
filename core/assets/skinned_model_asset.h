#ifndef SILENCE_SKINNED_MODEL_ASSET_H
#define SILENCE_SKINNED_MODEL_ASSET_H

#include "asset_loader.h"
namespace assets {

struct BoneData {
	std::vector<std::array<float, 4>> rotation;
	std::vector<std::array<float, 3>> translation;
};

struct JointData {
	std::vector<std::array<float, 4>> rotation;
	std::vector<std::array<float, 3>> translation;
	std::vector<uint32_t> id;
};

struct SkinnedModelInfo {
	std::vector<int64_t> bone_parents;
	std::vector<std::string> bone_names;
	std::vector<std::string> joint_names;
	std::vector<std::vector<int32_t>> bone_children;
	int32_t root;

	struct NodeMesh {
		std::string material_path;
		std::string mesh_path;
	};
	uint64_t bone_translation_buffer_size;
	uint64_t bone_rotation_buffer_size;
	uint64_t joint_translation_buffer_size;
	uint64_t joint_rotation_buffer_size;
	uint64_t joint_id_buffer_size;
	std::unordered_map<uint64_t, NodeMesh> node_meshes;
};

SkinnedModelInfo read_skinned_model_info(AssetFile *file);
AssetFile pack_model(const SkinnedModelInfo &info, BoneData &bone_data, JointData &joint_data);

} //namespace assets

#endif //SILENCE_SKINNED_MODEL_ASSET_H
