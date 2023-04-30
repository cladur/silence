#include "inspector_gui.h"
void Inspector::show_components(Entity entity) {
	ECSManager &ecs_manager = ECSManager::get();
	std::string entity_name;
	if (ecs_manager.has_component<Name>(entity)) {
		Name &name = ecs_manager.get_component<Name>(entity);
		entity_name = name.name;
	} else {
		entity_name = fmt::format("Entity {}", entity);
	}
	if (ImGui::CollapsingHeader(entity_name.c_str())) {
		if (ImGui::TreeNode("Components")) {
			Signature signature = ecs_manager.get_entity_signature(entity);
			for (int i = 0; i < signature.size(); i++) {
				if (signature[i] == true) {
					show_component(entity, i);
				}
			}
			ImGui::TreePop();
		}
	}
}
void Inspector::show_component(Entity entity, int signature_index) {
	switch (signature_index) {
		case 1:
			show_transform(entity);
			break;
		case 2:
			show_rigidbody(entity);
			break;
		case 3:
			show_gravity(entity);
			break;
		case 4:
			show_parent(entity);
			break;
		case 5:
			show_children(entity);
			break;
		case 6:
			show_modelinstance(entity);
			break;
		case 7:
			show_fmodlistener(entity);
			break;
		case 8:
			show_collidertag(entity);
			break;
		case 9:
			show_collidersphere(entity);
			break;
		case 10:
			show_collideraabb(entity);
			break;
		case 11:
			show_colliderobb(entity);
			break;
		default:
			SPDLOG_WARN("Invalid signature index {}", signature_index);
			break;
	}
}
void Inspector::show_transform(Entity entity) {
	auto &transform = ecs_manager.get_component<Transform>(entity);
	bool changed = false;
	if (ImGui::TreeNode("Transform")) {
		changed |= ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
		changed |= ImGui::DragFloat3("Rotation", &transform.euler_rot.x, 0.1f);
		changed |= ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);

		if (changed) {
			transform.set_changed(true);
		}
		ImGui::TreePop();
	}
}
void Inspector::show_rigidbody(Entity entity) {
}
void Inspector::show_gravity(Entity entity) {
}
void Inspector::show_parent(Entity entity) {
}
void Inspector::show_children(Entity entity) {
}
void Inspector::show_modelinstance(Entity entity) {
}
void Inspector::show_fmodlistener(Entity entity) {
}
void Inspector::show_collidertag(Entity entity) {
}
void Inspector::show_collidersphere(Entity entity) {
}
void Inspector::show_collideraabb(Entity entity) {
}
void Inspector::show_colliderobb(Entity entity) {
}
