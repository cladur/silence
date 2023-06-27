#include "dialogue_collider_draw.h"
#include "engine/scene.h"

AutoCVarInt cvar_dialogue_collider_draw_system_enabled(
		"debug_draw.dialogue_colliders.draw", "enable dialogue collider draw system", 0, CVarFlags::EditCheckbox);

void DialogueColliderDrawSystem::startup(World &world) {
	Signature white_signature;
	white_signature.set(world.get_component_type<DialogueTrigger>());
	white_signature.set(world.get_component_type<ColliderOBB>());
	world.set_system_component_whitelist<DialogueColliderDrawSystem>(white_signature);
}

void DialogueColliderDrawSystem::update(World &world, float dt) {
	if (cvar_dialogue_collider_draw_system_enabled.get() == 0) {
		return;
	}
	ZoneScopedN("DialogueColliderDrawSystem::update");
	for (const Entity entity : entities) {
		const auto &transform = world.get_component<Transform>(entity);
		const auto &col = world.get_component<ColliderOBB>(entity);

		const glm::vec3 &position = transform.get_global_position();
		const glm::vec3 &scale = transform.get_global_scale();
		const glm::quat &orientation = transform.get_global_orientation();

		world.get_parent_scene()->get_render_scene().debug_draw.draw_box(position + orientation * (col.center * scale),
				orientation, col.range * 2.0f * scale, glm::vec3(1.0f, 1.0f, 0.0f), entity);
	}
}
