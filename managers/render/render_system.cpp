#include "render_system.h"

#include "core/components/transform_component.h"
#include "ecs/ecs_manager.h"
#include "render_manager.h"

extern ECSManager ecs_manager;

void RenderSystem::startup() {
	Signature signature;
	signature.set(ecs_manager.get_component_type<Transform>());
	signature.set(ecs_manager.get_component_type<PrefabInstance>());
	ecs_manager.set_system_component_whitelist<RenderSystem>(signature);
}

void RenderSystem::update(RenderManager &render_manager) {
	for (auto const &entity : entities) {
		auto &transform = ecs_manager.get_component<Transform>(entity);
		auto &mesh_instance = ecs_manager.get_component<PrefabInstance>(entity);

		if (transform.is_changed_this_frame()) {
			for (auto &object_id : mesh_instance.object_ids) {
				render_manager.render_scene.update_transform(object_id, transform.get_global_model_matrix());
			}
		}
	}
}
