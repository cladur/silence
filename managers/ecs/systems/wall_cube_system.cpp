#include "ecs/systems/wall_cube_system.h"
#include "ecs/world.h"

void WallCubeSystem::startup(World &world) {
	Signature whitelist;
	whitelist.set(world.get_component_type<WallCube>());
	whitelist.set(world.get_component_type<Transform>());

	world.set_system_component_whitelist<WallCubeSystem>(whitelist);
}

void WallCubeSystem::update_face_uv(ModelInstance &face_model_instance, int i, glm::vec3 global_scale) {
	auto uv_scale = glm::vec2(1.0f);

	//faces L R U D F B

	switch (i) {
		case 0:
		case 1:
			uv_scale = glm::vec2(global_scale.z, global_scale.y);
			break;
		case 2:
		case 3:
			uv_scale = glm::vec2(global_scale.x, global_scale.z);
			break;
		case 4:
		case 5:
			uv_scale = glm::vec2(global_scale.x, global_scale.y);
			break;
		default:
			break;
	}

	face_model_instance.uv_scale = uv_scale;
}

void WallCubeSystem::update(World &world, float dt) {
	ZoneScopedN("WallCube::update");
	for (auto const &entity : entities) {
		auto &wall_cube = world.get_component<WallCube>(entity);

		if (wall_cube.faces_parent == 0) {
			continue;
		}

		auto &transform = world.get_component<Transform>(entity);
		auto &faces = world.get_component<Children>(wall_cube.faces_parent);
		auto global_scale = transform.get_global_scale();
		auto scale_uv = wall_cube.scale_uv;

		if (!scale_uv) {
			for (int i = 0; i < 6; i++) {
				auto &face_model_instance = world.get_component<ModelInstance>(faces.children[i]);
				face_model_instance.scale_uv_with_transform = scale_uv;
			}
			continue;
		}

		//faces L R U D F B

		for (int i = 0; i < 6; i++) {
			auto &face_model_instance = world.get_component<ModelInstance>(faces.children[i]);
			if (face_model_instance.model_handle.id != wall_cube.model_handle.id) {
				face_model_instance.model_handle = wall_cube.model_handle;
			}

			face_model_instance.scale_uv_with_transform = scale_uv;

			update_face_uv(face_model_instance, i, global_scale);
		}
	}
}