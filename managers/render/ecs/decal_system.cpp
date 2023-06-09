#include "decal_system.h"

#include "ecs/world.h"
#include "engine/scene.h"

AutoCVarInt decal_debug_draw("debug_draw.decals", "debug draw decal cube", 0, CVarFlags::EditCheckbox);

void DecalSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<Transform>());
	signature.set(world.get_component_type<Decal>());
	world.set_system_component_whitelist<DecalSystem>(signature);
}

void DecalSystem::update(World &world, float dt) {
	RenderScene &render_scene = world.get_parent_scene()->get_render_scene();

	for (Entity entity : entities) {
		Decal &decal = world.get_component<Decal>(entity);
		Transform &transform = world.get_component<Transform>(entity);

		render_scene.queue_decal_draw(&decal, &transform);
		if (decal_debug_draw.get()) {
			const glm::vec3 &scale = transform.get_global_scale();
			const glm::vec3 &position = transform.get_global_position();
			const glm::quat &rotation = transform.get_global_orientation();
			render_scene.debug_draw.draw_box(position, rotation, scale, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), entity);
			const glm::vec3 &offset = rotation * glm::vec3(0.0f, scale.y * 0.5f, 0.0f);
			render_scene.debug_draw.draw_arrow(
					position + offset, position - offset, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), entity);
		}
	}
}
