#include "model.h"

#include "assets/material_asset.h"
#include "assets/prefab_asset.h"

#include "managers/render/render_manager.h"

void Model::load_from_asset(const char *path) {
	name = path;

	RenderManager *render_manager = RenderManager::get();

	assets::AssetFile file;
	bool loaded = assets::load_binary_file(path, file);
	if (!loaded) {
		SPDLOG_ERROR("Error When loading prefab file at path {}", path);
		assert(false);
	} else {
		SPDLOG_INFO("Prefab {} loaded to cache", path);
	}

	assets::PrefabInfo prefab = assets::read_prefab_info(&file);

	std::unordered_map<uint64_t, glm::mat4> node_worldmats;

	std::vector<std::pair<uint64_t, glm::mat4>> pending_nodes;
	for (auto &[k, v] : prefab.node_matrices) {
		glm::mat4 nodematrix{ 1.f };

		auto nm = prefab.matrices[v];
		memcpy(&nodematrix, &nm, sizeof(glm::mat4));

		//check if it has parents
		auto matrix_it = prefab.node_parents.find(k);
		if (matrix_it == prefab.node_parents.end()) {
			//add to worldmats
			node_worldmats[k] = root * nodematrix;
		} else {
			//enqueue
			pending_nodes.emplace_back(k, nodematrix);
		}
	}

	//process pending nodes list until it empties
	while (!pending_nodes.empty()) {
		for (int i = 0; i < pending_nodes.size(); i++) {
			uint64_t node = pending_nodes[i].first;
			uint64_t parent = prefab.node_parents[node];

			//try to find parent in cache
			auto matrix_it = node_worldmats.find(parent);
			if (matrix_it != node_worldmats.end()) {
				//transform with the parent
				glm::mat4 nodematrix = (matrix_it)->second * pending_nodes[i].second;

				node_worldmats[node] = nodematrix;

				//remove from queue, pop last
				pending_nodes[i] = pending_nodes.back();
				pending_nodes.pop_back();
				i--;
			}
		}
	}

	std::vector<Mesh> prefab_renderables;
	prefab_renderables.reserve(prefab.node_meshes.size());

	for (auto &[k, v] : prefab.node_meshes) {
		//load mesh

		Mesh mesh{};
		mesh.load_from_asset(asset_path(v.mesh_path).c_str());

		//load material
		auto material_name = v.material_path.c_str();

		assets::AssetFile material_file;
		loaded = assets::load_binary_file(asset_path(material_name).c_str(), material_file);

		if (!loaded) {
			SPDLOG_ERROR("Error When loading material file at path {}", material_name);
			assert(false);
		}

		assets::MaterialInfo material = assets::read_material_info(&material_file);

		// BASE COLOR
		if (material.textures.contains("baseColor")) {
			Texture albedo = {};
			albedo.load_from_asset(asset_path(material.textures["baseColor"]));
			mesh.textures[0] = albedo;
			mesh.textures_present[0] = true;
		}

		// NORMAL
		if (material.textures.contains("normals")) {
			Texture normal = {};
			normal.load_from_asset(asset_path(material.textures["normals"]));
			mesh.textures[1] = normal;
			mesh.textures_present[1] = true;
		}

		// METALLIC ROUGHNESS
		if (material.textures.contains("metallicRoughness")) {
			Texture met_rough = {};
			met_rough.load_from_asset(asset_path(material.textures["metallicRoughness"]));
			mesh.textures[2] = met_rough;
			mesh.textures_present[2] = true;

			// AMBIENT OCCLUSION
			if (material.textures.contains("occlusion")) {
				// We assume that the occlusion map is bundled alongside the metallic roughness map, and don't load it
				// in here
				mesh.has_ao_map = true;
			}
		}

		// EMISSIVE
		if (material.textures.contains("emissive")) {
			Texture emissive = {};
			emissive.load_from_asset(asset_path(material.textures["emissive"]));
			mesh.textures[3] = emissive;
			mesh.textures_present[3] = true;
		}

		meshes.push_back(mesh);
	}
}