#include "skinned_model_asset.h"
#include <nlohmann/json.hpp>

assets::SkinnedModelInfo assets::read_skinned_model_info(AssetFile *file) {
	SkinnedModelInfo info;
	nlohmann::json model_metadata = nlohmann::json::parse(file->json);

	info.bone_names = model_metadata["bone_names"].get<std::vector<std::string>>();
	info.joint_names = model_metadata["joint_names"].get<std::vector<std::string>>();
	info.bone_parents = model_metadata["bone_parents"].get<std::vector<int64_t>>();
	info.root = model_metadata["root"].get<int32_t>();

	info.bone_rotation_buffer_size = model_metadata["bone_rotation_buffer_size"];
	info.bone_translation_buffer_size = model_metadata["bone_translation_buffer_size"];

	info.joint_rotation_buffer_size = model_metadata["joint_rotation_buffer_size"];
	info.joint_translation_buffer_size = model_metadata["joint_translation_buffer_size"];
	info.joint_id_buffer_size = model_metadata["joint_id_buffer_size"];

	std::unordered_map<uint64_t, nlohmann::json> mesh_nodes = model_metadata["node_meshes"];

	for (auto pair : mesh_nodes) {
		assets::SkinnedModelInfo::NodeMesh node;

		node.mesh_path = pair.second["mesh_path"];
		node.material_path = pair.second["material_path"];

		info.node_meshes[pair.first] = node;
	}

	return info;
}

assets::AssetFile assets::pack_model(const SkinnedModelInfo &info, BoneData &bone_data, JointData &joint_data) {
	nlohmann::json model_metadata;
	model_metadata["bone_names"] = info.bone_names;
	model_metadata["joint_names"] = info.joint_names;
	model_metadata["bone_parents"] = info.bone_parents;

	model_metadata["root"] = info.root;

	model_metadata["bone_rotation_buffer_size"] = info.bone_rotation_buffer_size;
	model_metadata["bone_translation_buffer_size"] = info.bone_translation_buffer_size;

	model_metadata["joint_rotation_buffer_size"] = info.joint_rotation_buffer_size;
	model_metadata["joint_translation_buffer_size"] = info.joint_translation_buffer_size;
	model_metadata["joint_id_buffer_size"] = info.joint_id_buffer_size;

	std::unordered_map<uint64_t, nlohmann::json> meshindex;
	for (const auto &pair : info.node_meshes) {
		nlohmann::json meshnode;
		meshnode["mesh_path"] = pair.second.mesh_path;
		meshnode["material_path"] = pair.second.material_path;
		meshindex[pair.first] = meshnode;
	}
	model_metadata["node_meshes"] = meshindex;

	//core file header
	AssetFile file;
	file.type[0] = 'M';
	file.type[1] = 'O';
	file.type[2] = 'D';
	file.type[3] = 'L';
	file.version = 1;

	size_t bone_data_size = info.bone_translation_buffer_size + info.bone_rotation_buffer_size;
	size_t joint_data_size =
			info.joint_translation_buffer_size + info.joint_rotation_buffer_size + info.joint_id_buffer_size;

	size_t full_size = bone_data_size + joint_data_size;
	file.binary_blob.resize(full_size);
	size_t actual_offset = 0;
	memcpy(file.binary_blob.data(), bone_data.translation.data(), info.bone_translation_buffer_size);
	actual_offset += info.bone_translation_buffer_size;

	memcpy(file.binary_blob.data() + actual_offset, bone_data.rotation.data(), info.bone_rotation_buffer_size);
	actual_offset += info.bone_rotation_buffer_size;

	memcpy(file.binary_blob.data() + actual_offset, joint_data.translation.data(), info.joint_translation_buffer_size);
	actual_offset += info.joint_translation_buffer_size;

	memcpy(file.binary_blob.data() + actual_offset, joint_data.rotation.data(), info.joint_rotation_buffer_size);
	actual_offset += info.joint_rotation_buffer_size;

	memcpy(file.binary_blob.data() + actual_offset, joint_data.id.data(), info.joint_id_buffer_size);

	std::string stringified = model_metadata.dump();
	file.json = stringified;

	return file;
}
