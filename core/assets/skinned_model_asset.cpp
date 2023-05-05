#include "skinned_model_asset.h"
#include <nlohmann/json.hpp>

assets::SkinnedModelInfo assets::read_skinned_model_info(AssetFile *file) {
	SkinnedModelInfo info;
	nlohmann::json model_metadata = nlohmann::json::parse(file->json);

	//info.node_matrices = std::unordered_map<uint64_t,int>(model_metadata["node_matrices"]) ;
	for (const auto &pair : model_metadata["node_matrices"].items()) {
		auto value = pair.value();
		info.node_matrices[value[0]] = value[1];
	}

	//info.node_names = std::unordered_map<uint64_t, std::string>(model_metadata["node_names"]);
	//auto nodenames =;
	for (auto &[key, value] : model_metadata["node_names"].items()) {
		info.node_names[value[0]] = value[1];
	}

	//info.node_parents = std::unordered_map<uint64_t, uint64_t>(model_metadata["node_parents"]);

	for (auto &[key, value] : model_metadata["node_parents"].items()) {
		info.node_parents[value[0]] = value[1];
	}

	std::unordered_map<uint64_t, nlohmann::json> meshnodes = model_metadata["node_meshes"];

	for (auto pair : meshnodes) {
		assets::SkinnedModelInfo::NodeMesh node;

		node.mesh_path = pair.second["mesh_path"];
		node.material_path = pair.second["material_path"];

		info.node_meshes[pair.first] = node;
	}

	size_t nmatrices = file->binary_blob.size() / (sizeof(float) * 16);
	info.matrices.resize(nmatrices);

	memcpy(info.matrices.data(), file->binary_blob.data(), file->binary_blob.size());

	return info;
}

assets::AssetFile assets::pack_model(const SkinnedModelInfo &info) {
	nlohmann::json model_metadata;
	model_metadata["node_matrices"] = info.node_matrices;
	model_metadata["node_names"] = info.node_names;
	model_metadata["node_parents"] = info.node_parents;

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

	file.binary_blob.resize(info.matrices.size() * sizeof(float) * 16);
	memcpy(file.binary_blob.data(), info.matrices.data(), info.matrices.size() * sizeof(float) * 16);

	std::string stringified = model_metadata.dump();
	file.json = stringified;

	return file;
}