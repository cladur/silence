#include "isolated_entities_system.h"
#include "components/transform_component.h"
#include "ecs/ecs_manager.h"
#include <components/children_component.h>
#include <components/parent_component.h>

void IsolatedEntitiesSystem::startup() {
	ECSManager &ecs_manager = ECSManager::get();
	Signature blacklist;
	Signature whitelist;
	blacklist.set(ecs_manager.get_component_type<Children>());
	blacklist.set(ecs_manager.get_component_type<Parent>());
	whitelist.set(ecs_manager.get_component_type<Transform>());

	ecs_manager.set_system_component_blacklist<IsolatedEntitiesSystem>(blacklist);
	ecs_manager.set_system_component_whitelist<IsolatedEntitiesSystem>(whitelist);
}
void IsolatedEntitiesSystem::update() {
	ECSManager &ecs_manager = ECSManager::get();
	for (auto const &entity : entities) {
		ecs_manager.get_component<Transform>(entity).update_global_model_matrix();
	}
}
