#include "skinned_model.h"

#include "assets/material_asset.h"
#include "assets/skinned_model_asset.h"


//TODO: Remove rig
void SkinnedModel::load_from_asset(const char *path) {
	name = path;

	assets::AssetFile file;
	bool loaded = assets::load_binary_file(path, file);
	if (!loaded) {
		SPDLOG_ERROR("Error when loading info file at path {}", path);
		assert(false);
	} else {
		SPDLOG_INFO("Model {} loaded to cache", path);
	}

	assets::SkinnedModelInfo info = assets::read_skinned_model_info(&file);

	rig.children = info.bone_children;

	rig.parents = info.bone_parents;
	rig.names = info.bone_names;

	std::vector<glm::vec3> translations(info.bone_translation_buffer_size / sizeof(glm::vec3));
	std::vector<glm::quat> rotations(info.bone_rotation_buffer_size / sizeof(glm::quat));

	memcpy(translations.data(), file.binary_blob.data(), info.bone_translation_buffer_size);
	memcpy(rotations.data(), file.binary_blob.data() + info.bone_translation_buffer_size,
			info.bone_rotation_buffer_size);
	rig.positions = translations;
	rig.rotations = rotations;

	std::vector<glm::vec3> joint_translations(info.joint_translation_buffer_size / sizeof(glm::vec3));
	std::vector<glm::quat> joint_rotations(info.joint_rotation_buffer_size / sizeof(glm::quat));
	std::vector<int32_t> joint_ids(info.joint_id_buffer_size / sizeof(int32_t));

	size_t rig_size = info.bone_translation_buffer_size + info.bone_rotation_buffer_size;
	memcpy(joint_translations.data(), file.binary_blob.data() + rig_size, info.joint_translation_buffer_size);
	memcpy(joint_rotations.data(), file.binary_blob.data() + rig_size + info.joint_translation_buffer_size,
			info.joint_rotation_buffer_size);
	memcpy(joint_ids.data(),
			file.binary_blob.data() + rig_size + info.joint_translation_buffer_size + info.joint_rotation_buffer_size,
			info.joint_id_buffer_size);

	for (int32_t i = 0; i < info.joint_names.size(); ++i) {
		joint_map[info.joint_names[i]].rotation = joint_rotations[i];
		joint_map[info.joint_names[i]].translation = joint_translations[i];
		joint_map[info.joint_names[i]].id = joint_ids[i];
	}

	root = Bone{};
	process_bone(info.root, root);

	std::vector<SkinnedMesh> model_renderables;
	model_renderables.reserve(info.node_meshes.size());

	for (auto &[k, v] : info.node_meshes) {
		//load mesh

		SkinnedMesh mesh{};
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
	rig.children.clear();
	rig.rotations.clear();
	rig.positions.clear();
	rig.names.clear();
	rig.parents.clear();
}

void SkinnedModel::process_bone(int32_t bone_index, Bone &bone) {
	bone.name = rig.names[bone_index];
	bone.translation = rig.positions[bone_index];
	bone.rotation = rig.rotations[bone_index];
	bone.children.reserve(rig.children[bone_index].size());
	for (int32_t i = 0; i < rig.children[bone_index].size(); ++i) {
		bone.children.emplace_back();
		process_bone(rig.children[bone_index][i], bone.children[i]);
	}
}
