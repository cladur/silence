#include "world.h"

#include "component_visitor.h"
#include "components/children_component.h"
#include "components/parent_component.h"
#include "resource/resource_manager.h"
#include "serialization.h"
#include <spdlog/spdlog.h>

#include <future>
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

void World::deserialize_entity_json(nlohmann::json &json, std::vector<Entity> &entities) {
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
}

void World::deserialize_entities_json(nlohmann::json &json, std::vector<Entity> &entities) {
	ResourceManager &resource_manager = ResourceManager::get();
	std::set<std::string> assets_to_load;
	for (auto &array_entity : json) {
		for (auto &component : array_entity["components"]) {
			std::string component_name = component["component_name"];
			int component_id = component_ids[component_name];
			if (component_id == component_ids["ModelInstance"]) {
				std::string model_path = component["component_data"]["model_name"];
				assets_to_load.insert(model_path);
			}
		}
	}

	std::vector<std::future<void>> futures = {};
	futures.reserve(assets_to_load.size());
	for (auto &asset : assets_to_load) {
		futures.emplace_back(std::async(std::launch::deferred,
				[&resource_manager, &asset] { resource_manager.load_model(asset_path(asset).c_str()); }));
	}
	for (auto &f : futures) {
		f.wait();
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