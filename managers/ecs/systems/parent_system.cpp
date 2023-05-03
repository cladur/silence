#include "parent_system.h"
#include "components/children_component.h"
#include "components/parent_component.h"
#include "components/transform_component.h"
#include "ecs/world.h"
#include "isolated_entities_system.h"
#include "root_parent_system.h"

void ParentSystem::startup(World &world) {
	world.register_system<IsolatedEntitiesSystem>();
	world.register_system<RootParentSystem>();
}

void ParentSystem::update(World &world, float dt) {
	// ZoneScopedN("ParentSystem::update");
	// isolated_entities_system->update();
	// root_parent_system->update();
}
