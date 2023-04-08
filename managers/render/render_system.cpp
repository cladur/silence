#include "render_system.h"
#include <components/mesh_instance_component.h>

#include "core/components/transform_component.h"
#include "ecs/ecs_manager.h"
#include "render_manager.h"

extern ECSManager ecs_manager;

void RenderSystem::startup() {
	Signature signature;
	signature.set(ecs_manager.get_component_type<Transform>());
	signature.set(ecs_manager.get_component_type<MeshInstance>());
	ecs_manager.set_system_component_whitelist<RenderSystem>(signature);
}

void RenderSystem::update(RenderManager &render_manager) {
	render_manager.renderables.clear();
	for (auto const &entity : entities) {
		auto &transform = ecs_manager.get_component<Transform>(entity);
		auto &mesh_instance = ecs_manager.get_component<MeshInstance>(entity);

		RenderObject render_object = {};
		render_object.mesh = mesh_instance.mesh;
		render_object.material = mesh_instance.material;
		render_object.transform_matrix = transform.get_global_model_matrix();

		render_manager.renderables.push_back(render_object);
	}
}
