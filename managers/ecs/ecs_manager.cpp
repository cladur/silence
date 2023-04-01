#include "ecs_manager.h"

#include "components/children_component.h"
#include "components/parent_component.h"

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

void ECSManager::destroy_entity(Entity entity) {
	entity_manager->destroy_entity(entity);

	component_manager->entity_destroyed(entity);

	system_manager->entity_destroyed(entity);
}

bool ECSManager::add_child(Entity parent, Entity child) {
	if (child == parent) {
		SPDLOG_WARN("Parent and children are the same entity");
		return false;
	}

	if (!has_component<Children>(parent)) {
		add_component<Children>(parent, Children{});
		SPDLOG_INFO("Added children component to {}", parent);
	}

	if (!has_component<Parent>(child)) {
		add_component(child, Parent{ parent });
		SPDLOG_INFO("Added parent component to {}", child);
	}

	SPDLOG_INFO("Added child {} to parent {}", child, parent);
	return get_component<Children>(parent).add_child(child);
}

bool ECSManager::remove_child(Entity parent, Entity child) {
	if (!has_component<Children>(parent)) {
		SPDLOG_WARN("No children component found on parent");
		return false;
	}

	bool found_child = get_component<Children>(parent).remove_child(child);
	if (!found_child) {
		SPDLOG_WARN("Children {} not found on parent", child);
		return false;
	} else {
		SPDLOG_INFO("Removed child {} from parent {}", child, parent);
	}

	remove_component<Parent>(child);
	SPDLOG_INFO("Removed parent component from child {}", child);

	if (get_component<Children>(parent).children_count == 0) {
		remove_component<Children>(parent);
		SPDLOG_INFO("Removed children component from {}", parent);
	}

	return true;
}
void ECSManager::call_all_on_starts() {
	component_manager->run_starts();
}
