#include "attachment_system.h"
#include "animation/animation_manager.h"
#include "components/attachment_component.h"
#include "components/transform_component.h"
#include "ecs/world.h"
#include "render/ecs/skinned_model_instance.h"

void AttachmentSystem::startup(World &world) {
	Signature whitelist;
	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<Attachment>());
	world.set_system_component_whitelist<AttachmentSystem>(whitelist);
}

void AttachmentSystem::update(World &world, float dt) {
	AnimationManager &animation_manager = AnimationManager::get();

	for (Entity entity : entities) {
		Transform &transform = world.get_component<Transform>(entity);
		Attachment &attachment = world.get_component<Attachment>(entity);
		if (attachment.holder == -1 || attachment.bone_name.empty()) {
			continue;
		}
		Transform &holder_transform = world.get_component<Transform>(attachment.holder);
		const glm::mat4 &bone_matrix = animation_manager.get_bone_transform(attachment.holder, attachment.bone_name);
		transform.update_global_model_matrix(holder_transform.get_global_model_matrix() * bone_matrix);
	}
}
