#include "opengl_system.h"

#include "core/components/transform_component.h"
#include "ecs/ecs_manager.h"

#include "opengl_manager.h"

extern ECSManager ecs_manager;

void OpenglSystem::startup() {
	Signature signature;
	signature.set(ecs_manager.get_component_type<Transform>());
	signature.set(ecs_manager.get_component_type<PrefabInstance>());
	ecs_manager.set_system_component_whitelist<OpenglSystem>(signature);
}

void OpenglSystem::update() {
	OpenglManager &opengl_manager = *OpenglManager::get();

	for (auto const &entity : entities) {
		auto &transform = ecs_manager.get_component<Transform>(entity);
		auto &prefab_instance = ecs_manager.get_component<PrefabInstance>(entity);

		if (transform.is_changed_this_frame()) {
			for (auto &object_id : prefab_instance.mesh_ids) {
				opengl_manager.update_transform(object_id, transform.get_global_model_matrix());
			}
		}
	}
}