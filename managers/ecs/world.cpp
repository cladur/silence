#include "world.h"

#include "assets/material_asset.h"
#include "assets/model_asset.h"
#include "assets/skinned_model_asset.h"
#include "component_visitor.h"
#include "components/children_component.h"
#include "components/parent_component.h"
#include "ecs/component_visitor.h"
#include "resource/resource_manager.h"
#include "serialization.h"
#include <spdlog/spdlog.h>

#include <common/TracyColor.hpp>
#include <common/TracyQueue.hpp>
#include <future>
#include <tracy/tracy.hpp>
#include <vector>

void World::startup() {
	// Create pointers to each manager
	component_manager = std::make_unique<ComponentManager>();
	entity_manager = std::make_unique<EntityManager>();
	entity_manager->startup();
	system_manager = std::make_unique<SystemManager>();
}

Entity World::create_entity() {
	return entity_manager->create_entity();
}

Entity World::create_entity(Entity entity) {
	return entity_manager->create_entity(entity);
}

void World::destroy_entity(Entity entity) {
	entity_manager->destroy_entity(entity);

	component_manager->entity_destroyed(entity);

	system_manager->entity_destroyed(entity);
}

bool World::add_child(Entity parent, Entity child, bool keep_transform) {
	if (child == parent) {
		SPDLOG_WARN("Parent and children are the same entity");
		return false;
	}

	if (!has_component<Children>(parent)) {
		add_component<Children>(parent, Children{});
		// SPDLOG_INFO("Added children component to {}", parent);
	} else {
		if (has_child(parent, child)) {
			SPDLOG_WARN("Child {} already exists on parent {}", child, parent);
			return false;
		}
	}

	if (!has_component<Parent>(child)) {
		add_component(child, Parent{ parent });
		// SPDLOG_INFO("Added parent component to {}", child);
	}

	// SPDLOG_INFO("Added child {} to parent {}", child, parent);

	if (keep_transform && has_component<Transform>(parent) && has_component<Transform>(child)) {
		auto &child_transform = get_component<Transform>(child);
		auto &parent_transform = get_component<Transform>(parent);
		child_transform.reparent_to(parent_transform);
	}

	return get_component<Children>(parent).add_child(child);
}

bool World::remove_child(Entity parent, Entity child, bool keep_transform) {
	if (!has_component<Children>(parent)) {
		SPDLOG_WARN("No children component found on parent");
		return false;
	}

	if (keep_transform && has_component<Transform>(parent) && has_component<Transform>(child)) {
		auto &child_transform = get_component<Transform>(child);
		auto &parent_transform = get_component<Transform>(parent);
		Transform zero = Transform();
		child_transform.reparent_to(zero);
	}

	bool found_child = get_component<Children>(parent).remove_child(child);
	if (!found_child) {
		SPDLOG_WARN("Children {} not found on parent", child);
		return false;
	} else {
		// SPDLOG_INFO("Removed child {} from parent {}", child, parent);
	}

	remove_component<Parent>(child);
	// SPDLOG_INFO("Removed parent component from child {}", child);

	if (get_component<Children>(parent).children_count == 0) {
		remove_component<Children>(parent);
		// SPDLOG_INFO("Removed children component from {}", parent);
	}

	return true;
}

bool World::has_child(Entity parent, Entity child) {
	for (auto &c : get_component<Children>(parent).children) {
		if (c == child) {
			return true;
		}
	}
	return false;
}

bool World::reparent(Entity new_parent, Entity child, bool keep_transform) {
	if (has_component<Parent>(child)) {
		remove_child(get_component<Parent>(child).parent, child, keep_transform);
	}

	if (has_component<Transform>(new_parent) && has_component<Transform>(child)) {
		auto &child_transform = get_component<Transform>(child);
		auto &parent_transform = get_component<Transform>(new_parent);
		child_transform.reparent_to(parent_transform);
	}

	return add_child(new_parent, child);
}

void World::serialize_entity_json(nlohmann::json &json, Entity entity, bool is_archetype) {
	Entity entity_to_serialize = is_archetype ? 0 : entity;
	json["entity"] = entity_to_serialize;
	json["signature"] = entity_manager->get_signature(entity).to_string();
	json["components"] = nlohmann::json::array();
	component_manager->serialize_entity(json["components"], entity);
}

Entity World::deserialize_entity_json(nlohmann::json &json, std::vector<Entity> &entities) {
	Entity serialized_entity = json["entity"];
	serialized_entity = serialized_entity == 0 ? create_entity() : create_entity(serialized_entity);
	entities.push_back(serialized_entity);
	SPDLOG_INFO("Entity {} created or loaded", serialized_entity);

	serialization::IdToClassConstructor map = SceneManager::get_class_map();

	for (auto &component : json["components"]) {
		std::string component_name = component["component_name"];
		int component_id = component_ids[component_name];
		auto component_data = map[component_id](component["component_data"]);
		ComponentVisitor::visit(*this, serialized_entity, component_data);
	}

	return serialized_entity;
}

void World::deserialize_entities_json(nlohmann::json &json, std::vector<Entity> &entities) {
	ZoneScopedN("World::deserialize_entities_json");
	ResourceManager &resource_manager = ResourceManager::get();
	std::set<std::string> assets_to_load;
	std::set<std::string> skinned_assets_to_load;
	std::set<std::string> ktx_paths;

	{
		ZoneScopedN("preloading stuff to load");
		for (auto &array_entity : json) {
			for (auto &component : array_entity["components"]) {
				std::string component_name = component["component_name"];
				int component_id = component_ids[component_name];
				if (component_id == component_ids["ModelInstance"]) {
					std::string model_path = component["component_data"]["model_name"];
					assets_to_load.insert(model_path);
				} else if (component_id == component_ids["SkinnedModelInstance"]) {
					std::string model_path = component["component_data"]["model_name"];
					skinned_assets_to_load.insert(model_path);
				}
			}
		}

		ktx_paths.insert("resources/assets_export/cubemaps/venice_sunset/environment_map.ktx2");
		ktx_paths.insert("resources/assets_export/cubemaps/venice_sunset/irradiance_map.ktx2");
		ktx_paths.insert("resources/assets_export/cubemaps/venice_sunset/prefilter_map.ktx2");
		ktx_paths.insert("resources/assets_export/cubemaps/venice_sunset/brdf_lut.ktx2");

		struct NodeMesh {
			std::string material_path;
			std::string mesh_path;
		};

		for (int i = 0; i < assets_to_load.size() + skinned_assets_to_load.size(); i++) {
			std::string asset;
			if (i >= assets_to_load.size()) {
				asset = *std::next(skinned_assets_to_load.begin(), i - assets_to_load.size());
			} else {
				asset = *std::next(assets_to_load.begin(), i);
			}
			//auto asset = *std::next(assets_to_load.begin(), i);
			assets::AssetFile file;
			bool loaded = assets::load_binary_file(asset_path(asset).c_str(), file);
			std::vector<std::string> paths;
			if (i >= assets_to_load.size()) {
				assets::SkinnedModelInfo model = assets::read_skinned_model_info(&file);
				for (const auto &pair : model.node_meshes) {
					paths.push_back(pair.second.material_path);
				}
			} else {
				assets::ModelInfo model = assets::read_model_info(&file);
				for (const auto &pair : model.node_meshes) {
					paths.push_back(pair.second.material_path);
				}
			}

			for (auto &path : paths) {
				auto material_name = path.c_str();
				assets::AssetFile material_file;
				loaded = assets::load_binary_file(asset_path(material_name).c_str(), material_file);
				assets::MaterialInfo material = assets::read_material_info(&material_file);
				ktx_paths.insert(asset_path(material.textures["baseColor"]));
				ktx_paths.insert(asset_path(material.textures["normals"]));
				ktx_paths.insert(asset_path(material.textures["metallicRoughness"]));
				ktx_paths.insert(asset_path(material.textures["emissive"]));
			}
		}
	}

	{
		ZoneScopedN("transcoding ktx textures");

		SPDLOG_INFO("Transcoding ktx textures");

		ktx_texture_transcode_fmt_e tf = KTX_TTF_RGBA32;
		GLenum format = Texture::get_supported_compressed_format();

		if (format == GL_COMPRESSED_RGBA_BPTC_UNORM) {
			tf = KTX_TTF_BC7_RGBA;
		} else if (format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) {
			tf = KTX_TTF_BC3_RGBA;
		}

//count time for loading ktx
#pragma omp parallel for
		for (int i = 0; i < ktx_paths.size(); i++) {
			int64_t thread = omp_get_thread_num();
			ktxTexture2 *ktx = nullptr;
			std::string ktx_path;
#pragma omp critical
			{ ktx_path = *std::next(ktx_paths.begin(), i); }
			ktxTexture2_CreateFromNamedFile(ktx_path.c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, &ktx);

			if (ktx) {
				ktxTexture2_TranscodeBasis(ktx, tf, 0);
			}
			std::pair<std::string, ktxTexture2 *> pair = std::make_pair(ktx_path, ktx);
#pragma omp critical
			{ Texture::ktx_textures.insert(pair); }
		}
	}

	for (auto &asset : assets_to_load) {
		resource_manager.load_model(asset_path(asset).c_str());
	}

	for (auto &array_entity : json) {
		deserialize_entity_json(array_entity, entities);
	}
}
Signature World::get_entity_signature(Entity entity) {
	return entity_manager->get_signature(entity);
}
void World::print_components() {
	int i = 0;
	for (auto name : component_names) {
		SPDLOG_INFO("ID: {}  Name: {}", i++, name);
	}
}
void World::add_component(Entity entity, int component_id) {
	add_component_map[component_id](entity);
}
void World::remove_component(Entity entity, int component_id) {
	remove_component_map[component_id](entity);
}
bool World::has_component(Entity entity, int component_id) {
	return has_component_map[component_id](entity);
}
std::vector<std::string> &World::get_component_names() {
	return component_names;
}
int World::get_registered_components() {
	return registered_components;
}

Scene *World::get_parent_scene() {
	return parent_scene;
}

void World::update_name(std::vector<Entity> &entities, Entity &new_entity_id) {
	std::string entity_name;
	int name_counter = 0;

	if (has_component<Name>(new_entity_id)) {
		auto &name = get_component<Name>(new_entity_id);
		name.name = name.name + " " + std::to_string(new_entity_id);
	}
}
Entity World::deserialize_prefab(nlohmann::json &json, std::vector<Entity> &entities) {
	int number_of_entities = json.size();
	Entity root_entity = 0;

	if (number_of_entities == 0) {
		return 0;
	}

	if (number_of_entities == 1) {
		json.back()["entity"] = 0;
		deserialize_entity_json(json.back(), entities);
		root_entity = entities.back();
		return root_entity;
	}

	std::unordered_map<Entity, Entity> prefab_id_to_entity_id_map;
	std::vector<Entity> entity_ids;
	entity_ids.resize(number_of_entities);
	int i = 0;
	Entity new_entity_id = 0;

	static bool first = true;

	for (auto entity_json : json) {
		Entity serialized_entity = entity_json["entity"];
		entity_json["entity"] = 0;
		new_entity_id = deserialize_entity_json(entity_json, entities);

		if (first) {
			root_entity = new_entity_id;
			first = false;
		}

		entity_ids[i++] = new_entity_id;
		prefab_id_to_entity_id_map[serialized_entity] = new_entity_id;
		update_name(entities, new_entity_id);
	}

	for (auto entity_id : entity_ids) {
		update_parent(entity_id, prefab_id_to_entity_id_map);
		update_children(entity_id, prefab_id_to_entity_id_map);
		ComponentVisitor::update_ids(*this, entity_id, prefab_id_to_entity_id_map);
	}

	return root_entity;
}

void World::update_children(Entity entity, const std::unordered_map<Entity, Entity> &id_map) {
	if (!has_component<Children>(entity)) {
		return;
	}

	auto &children = get_component<Children>(entity);
	for (auto &child : children.children) {
		if (child == 0) {
			return;
		}
		Entity new_id = id_map.at(child);
		child = new_id;
	}
	SPDLOG_INFO("Children: {}", children.children[0]);
}
void World::update_parent(Entity entity, const std::unordered_map<Entity, Entity> &id_map) {
	if (!has_component<Parent>(entity)) {
		return;
	}

	auto &parent = get_component<Parent>(entity);
	if (parent.parent != 0) {
		Entity new_id = id_map.at(parent.parent);
		parent.parent = new_id;
	}
}