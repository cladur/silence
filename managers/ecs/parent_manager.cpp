#include "parent_manager.h"
#include "components/children_component.h"
#include "ecs_manager.h"
#include <spdlog/spdlog.h>

extern ECSManager ecs_manager;

bool ParentManager::add_children(Entity parent, Entity children) {
	if (children == parent) {
		SPDLOG_ERROR("Parent and children are the same entity");
		return false;
	}

	if (!ecs_manager.has_component<Children>(parent)) {
		ecs_manager.add_component<Children>(parent, Children{});
	}

	return ecs_manager.get_component<Children>(parent).add_children(children);
}

bool ParentManager::remove_children(Entity parent, Entity children) {
	if (!ecs_manager.has_component<Children>(parent)) {
		SPDLOG_ERROR("No children component found on parent");
		return false;
	}

	bool found_children = ecs_manager.get_component<Children>(parent).remove_children(children);
	if (!found_children) {
		SPDLOG_ERROR("Children {} not found on parent", children);
		return false;
	}

	if (ecs_manager.get_component<Children>(parent).children_count == 0) {
		ecs_manager.remove_component<Children>(parent);
	}

	return true;
}
