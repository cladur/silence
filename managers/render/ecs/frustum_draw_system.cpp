#include "frustum_draw_system.h"

#include "core/components/transform_component.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include "managers/render/render_manager.h"

AutoCVarInt cvar_frustum_draw_system_enabled(
		"debug_draw.frustum.draw", "enable frustum draw system", 1, CVarFlags::EditCheckbox);

AutoCVarInt cvar_frustum_draw_system_real_far_value(
		"debug_draw.frustum.real_far_value", "use actual far plane instead of fixed one", 0, CVarFlags::EditCheckbox);

void FrustumDrawSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<Transform>());
	signature.set(world.get_component_type<Camera>());
	world.set_system_component_whitelist<FrustumDrawSystem>(signature);
}

void FrustumDrawSystem::update(World &world, float dt) {
	if (!cvar_frustum_draw_system_enabled.get()) {
		return;
	}

	RenderManager &render_manager = RenderManager::get();
	for (auto const &entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &camera = world.get_component<Camera>(entity);

		float far = 4.0f;
		if (cvar_frustum_draw_system_real_far_value.get()) {
			far = *CVarSystem::get()->get_float_cvar("render.draw_distance.far");
		}
		float near = *CVarSystem::get()->get_float_cvar("render.draw_distance.near");
		float aspect_ratio = world.get_parent_scene()->get_render_scene().aspect_ratio;

		world.get_parent_scene()->get_render_scene().debug_draw.draw_frustum(transform.get_global_position(),
				transform.get_global_orientation(), camera.fov, aspect_ratio, near, far, glm::vec3(1.0f, 0.0f, 0.0f),
				entity);
	}
}