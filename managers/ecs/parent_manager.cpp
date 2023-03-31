#include "parent_manager.h"
#include "components/children_component.h"
#include "components/parent_component.h"
#include "ecs_manager.h"
#include <spdlog/spdlog.h>

extern ECSManager ecs_manager;

bool ParentManager::add_child(Entity parent, Entity child) {
	if (child == parent) {
		SPDLOG_ERROR("Parent and children are the same entity");
		return false;
	}

	if (!ecs_manager.has_component<Children>(parent)) {
		ecs_manager.add_component<Children>(parent, Children{});
		SPDLOG_INFO("Added children component to {}", parent);
	}

	if (!ecs_manager.has_component<Parent>(child)) {
		ecs_manager.add_component(child, Parent{ parent });
		SPDLOG_INFO("Added parent component to {}", child);
	}

	SPDLOG_INFO("Added child {} to parent {}", child, parent);
	return ecs_manager.get_component<Children>(parent).add_child(child);
}

bool ParentManager::remove_child(Entity parent, Entity child) {
	if (!ecs_manager.has_component<Children>(parent)) {
		SPDLOG_ERROR("No children component found on parent");
		return false;
	}

	bool found_child = ecs_manager.get_component<Children>(parent).remove_child(child);
	if (!found_child) {
		SPDLOG_ERROR("Children {} not found on parent", child);
		return false;
	} else {
		SPDLOG_INFO("Removed child {} from parent {}", child, parent);
	}

	ecs_manager.remove_component<Parent>(child);
	SPDLOG_INFO("Removed parent component from child {}", child);

	if (ecs_manager.get_component<Children>(parent).children_count == 0) {
		ecs_manager.remove_component<Children>(parent);
		SPDLOG_INFO("Removed children component from {}", parent);
	}

	return true;
}