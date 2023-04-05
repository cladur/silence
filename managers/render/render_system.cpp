#include <render/debug/debug_drawing.h>
#include "render_system.h"

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

        DebugDraw::draw_box(transform.get_position(), transform.get_scale() * 1.01f);

        float radius = transform.get_scale().x * 0.5f * (float)sqrt(3);

        DebugDraw::draw_sphere(transform.get_position(), radius);

//        DebugDraw::draw_box(transform.get_position(), glm::vec3(1.0f));
//        DebugDraw::draw_sphere(transform.get_position(), 0.5f);

		render_manager.renderables.push_back(render_object);
	}
}
