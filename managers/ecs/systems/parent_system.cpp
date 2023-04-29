#include "parent_system.h"
#include "components/children_component.h"
#include "components/parent_component.h"
#include "components/transform_component.h"
#include "ecs/ecs_manager.h"
#include "isolated_entities_system.h"
#include "root_parent_system.h"

void ParentSystem::startup() {
	ECSManager &ecs_manager = ECSManager::get();
	isolated_entities_system = ecs_manager.register_system<IsolatedEntitiesSystem>();
	isolated_entities_system->startup();

	root_parent_system = ecs_manager.register_system<RootParentSystem>();
	root_parent_system->startup();
}

void ParentSystem::update() {
	ECSManager &ecs_manager = ECSManager::get();
	ZoneScopedN("ParentSystem::update");
	isolated_entities_system->update();
	root_parent_system->update();
}
