#include "isolated_entities_system.h"
#include "components/transform_component.h"
#include "ecs/ecs_manager.h"

extern ECSManager ecs_manager;

void IsolatedEntitiesSystem::startup() {
}
void IsolatedEntitiesSystem::update() {
	for (auto const &entity : entities) {
		ecs_manager.get_component<Transform>(entity).update_global_model_matrix();
	}
}
