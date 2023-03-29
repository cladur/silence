#include "parent_system.h"
#include "../../core/components/children_component.h"
#include "../../core/components/parent_component.h"
#include "ecs/ecs_manager.h"
#include "isolated_entities_system.h"
#include "root_parent_system.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <iostream>

extern ECSManager ecs_manager;

void ParentSystem::startup() {
	isolated_entities_system = ecs_manager.register_system<IsolatedEntitiesSystem>();
	Signature blacklist;
	blacklist.set(ecs_manager.get_component_type<Children>());
	blacklist.set(ecs_manager.get_component_type<Parent>());
	ecs_manager.set_system_component_blacklist<IsolatedEntitiesSystem>(blacklist);
	isolated_entities_system->startup();
	blacklist.reset();

	root_parent_system = ecs_manager.register_system<RootParentSystem>();
	blacklist.set(ecs_manager.get_component_type<Parent>());
	Signature whitelist;
	whitelist.set(ecs_manager.get_component_type<Children>());
	ecs_manager.set_system_component_whitelist<RootParentSystem>(whitelist);
	ecs_manager.set_system_component_blacklist<RootParentSystem>(blacklist);
	root_parent_system->startup();
}

void ParentSystem::update() {
	isolated_entities_system->update();
	root_parent_system->update();
}
