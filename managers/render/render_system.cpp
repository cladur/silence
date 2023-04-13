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
	bool new_instance = false;

	for (auto const &entity : entities) {
		auto &transform = ecs_manager.get_component<Transform>(entity);
		auto &mesh_instance = ecs_manager.get_component<MeshInstance>(entity);

		if (!mesh_instance.registered) {
			new_instance = true;

			MeshObject render_object = {};
			render_object.mesh = mesh_instance.mesh;
			render_object.material = mesh_instance.material;
			render_object.transform_matrix = transform.get_global_model_matrix();
			render_object.b_draw_forward_pass = true;

			mesh_instance.object_id = render_manager.render_scene.register_object(&render_object);

			mesh_instance.registered = true;
			continue;
		}

		if (transform.is_changed_this_frame()) {
			render_manager.render_scene.update_transform(mesh_instance.object_id, transform.get_global_model_matrix());
		}
	}

	if (new_instance) {
		render_manager.render_scene.build_batches();
		render_manager.render_scene.merge_meshes(&render_manager);
	}
}
