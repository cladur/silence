#include "ecs_manager.h"

#include "component_visitor.h"
#include "components/children_component.h"
#include "components/parent_component.h"
#include "serialization.h"

ECSManager &ECSManager::get() {
	static ECSManager instance;
	return instance;
}

void ECSManager::startup() {
	// Create pointers to each manager
	component_manager = std::make_unique<ComponentManager>();
	entity_manager = std::make_unique<EntityManager>();
	entity_manager->startup();
	system_manager = std::make_unique<SystemManager>();
}

Entity ECSManager::create_entity() {
	return entity_manager->create_entity();
}

Entity ECSManager::create_entity(Entity entity) {
	return entity_manager->create_entity(entity);
}

void ECSManager::destroy_entity(Entity entity) {
	entity_manager->destroy_entity(entity);

	component_manager->entity_destroyed(entity);

	system_manager->entity_destroyed(entity);
}

bool ECSManager::add_child(Entity parent, Entity child, bool keep_transform) {
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

bool ECSManager::remove_child(Entity parent, Entity child, bool keep_transform) {
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

bool ECSManager::has_child(Entity parent, Entity child) {
	for (auto &c : get_component<Children>(parent).children) {
		if (c == child) {
			return true;
		}
	}
	return false;
}

bool ECSManager::reparent(Entity new_parent, Entity child, bool keep_transform) {
	remove_child(get_component<Parent>(child).parent, child, keep_transform);

	if (has_component<Transform>(new_parent) && has_component<Transform>(child)) {
		auto &child_transform = get_component<Transform>(child);
		auto &parent_transform = get_component<Transform>(new_parent);
		child_transform.reparent_to(parent_transform);
	}

	return add_child(new_parent, child);
}

void ECSManager::serialize_entity_json(nlohmann::json &json, Entity entity) {
	json["entity"] = entity;
	json["signature"] = entity_manager->get_signature(entity).to_string();
	json["components"] = nlohmann::json::array();
	component_manager->serialize_entity(json["components"], entity);
}

void ECSManager::deserialize_entities_json(nlohmann::json &json, std::vector<Entity> &entities) {
	serialization::IdToClassConstructor map = SceneManager::get_class_map();
	Entity entity{};
	Signature signature{};
	std::string string_signature{};
	for (auto &array_entity : json) {
		entity = array_entity["entity"];
		entities.push_back(entity);
		SPDLOG_INFO("Entity {} created or loaded", entity);
		create_entity(entity);

		string_signature = array_entity["signature"];
		std::reverse(string_signature.begin(), string_signature.end());

		for (int i = 0; i < string_signature.size(); i++) {
			int component_active = string_signature[i] - '0';
			if (component_active) {
				auto component = map[i](array_entity["components"]);

				ComponentVisitor::visit(entity, component);
			}
		}
	}
}
Signature ECSManager::get_entity_signature(Entity entity) {
	return entity_manager->get_signature(entity);
}
void ECSManager::print_components() {
	int i = 0;
	for (auto name : component_names) {
		SPDLOG_INFO("ID: {}  Name: {}", i++, name);
	}
}
void ECSManager::add_component(Entity entity, int component_id) {
	add_component_map[component_id](entity);
}
void ECSManager::remove_component(Entity entity, int component_id) {
	remove_component_map[component_id](entity);
}
bool ECSManager::has_component(Entity entity, int component_id) {
	return has_component_map[component_id](entity);
}
std::vector<std::string> &ECSManager::get_component_names() {
	return component_names;
}
int ECSManager::get_registered_components() {
	return registered_components;
}
