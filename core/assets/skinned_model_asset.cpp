#include "skinned_model_asset.h"
#include <nlohmann/json.hpp>

assets::SkinnedModelInfo assets::read_skinned_model_info(AssetFile *file) {
	SkinnedModelInfo info;
	nlohmann::json model_metadata = nlohmann::json::parse(file->json);

	for (auto &[key, value] : model_metadata["bone_names"].items()) {
		info.bone_names.push_back(value[1]);
	}

	for (auto &[key, value] : model_metadata["bone_parents"].items()) {
		info.bone_parents.push_back(value[1]);
	}
	info.bone_rotation_buffer_size = model_metadata["bone_rotation_buffer_size"];
	info.bone_translation_buffer_size = model_metadata["bone_translation_buffer_size"];

	std::unordered_map<uint64_t, nlohmann::json> mesh_nodes = model_metadata["node_meshes"];

	for (auto pair : mesh_nodes) {
		assets::SkinnedModelInfo::NodeMesh node;

		node.mesh_path = pair.second["mesh_path"];
		node.material_path = pair.second["material_path"];

		info.node_meshes[pair.first] = node;
	}

	return info;
}

assets::AssetFile assets::pack_model(const SkinnedModelInfo &info, char *bone_data) {
	nlohmann::json model_metadata;
	model_metadata["bone_names"] = info.bone_names;
	model_metadata["bone_parents"] = info.bone_parents;
	model_metadata["bone_rotation_buffer_size"] = info.bone_rotation_buffer_size;
	model_metadata["bone_translation_buffer_size"] = info.bone_translation_buffer_size;

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

	size_t full_size = info.bone_translation_buffer_size + info.bone_rotation_buffer_size;
	file.binary_blob.resize(full_size);
	memcpy(file.binary_blob.data(), bone_data, full_size);

	std::string stringified = model_metadata.dump();
	file.json = stringified;

	return file;
}
