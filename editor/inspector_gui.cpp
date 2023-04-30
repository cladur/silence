#include "inspector_gui.h"
#include <imgui.h>
void Inspector::show_components(Entity entity) {
	ECSManager &ecs_manager = ECSManager::get();
	std::string entity_name;
	if (ecs_manager.has_component<Name>(entity)) {
		Name &name = ecs_manager.get_component<Name>(entity);
		entity_name = name.name;
	} else {
		entity_name = fmt::format("Entity {}", entity);
	}
	if (ImGui::TreeNode(entity_name.c_str())) {
		if (ImGui::TreeNode("Components")) {
			Signature signature = ecs_manager.get_entity_signature(entity);
			for (int i = 0; i < signature.size(); i++) {
				if (signature[i] == true) {
					show_component(entity, i);
				}
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}
void Inspector::show_component(Entity entity, int signature_index) {
	switch (signature_index) {
		case 0:
			show_name(entity);
			break;
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
void Inspector::show_name(Entity entity) {
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
	auto &rigidbody = ecs_manager.get_component<RigidBody>(entity);
	if (ImGui::TreeNode("RigidBody")) {
		ImGui::DragFloat3("Velocity", &rigidbody.velocity.x, 0.1f);
		ImGui::DragFloat3("Acceleration", &rigidbody.acceleration.x, 0.1f);
		ImGui::TreePop();
	}
}
void Inspector::show_gravity(Entity entity) {
	auto &gravity = ecs_manager.get_component<Gravity>(entity);
	if (ImGui::TreeNode("Gravity")) {
		ImGui::DragFloat3("Gravity", &gravity.force.x, 0.1f);
		ImGui::TreePop();
	}
}
void Inspector::show_parent(Entity entity) {
	auto &parent = ecs_manager.get_component<Parent>(entity);
	if (ImGui::TreeNode("Parent")) {
		ImGui::Text("Parent: %d", parent.parent);
		ImGui::TreePop();
	}
}
void Inspector::show_children(Entity entity) {
	auto &children = ecs_manager.get_component<Children>(entity);
	if (ImGui::TreeNode("Children")) {
		ImGui::Text("Children: %d", children.children_count);
		if (ImGui::TreeNode("Children list")) {
			for (int i = 0; i < children.children_count; i++) {
				ImGui::Text("%d", children.children[i]);
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}
void Inspector::show_modelinstance(Entity entity) {
	auto &modelinstance = ecs_manager.get_component<ModelInstance>(entity);
	auto models = render_manager.get_models();
	if (ImGui::TreeNode("ModelInstance")) {
		std::string name = render_manager.get_model(modelinstance.model_handle).name;
		std::size_t last_slash_pos = name.find_last_of("/\\");

		if (last_slash_pos != std::string::npos) {
			name = name.substr(last_slash_pos + 1);
			std::size_t dot_pos = name.find_last_of(".");
			if (dot_pos != std::string::npos) {
				name = name.substr(0, dot_pos);
			}
		}

		if (ImGui::BeginCombo("Model", name.c_str())) {
			for (const auto &model : models) {
				bool is_selected = (modelinstance.model_handle.id == render_manager.get_model_handle(model.name).id);
				std::string model_name = model.name;
				std::size_t model_slash_pos = model_name.find_last_of("/\\");

				if (model_slash_pos != std::string::npos) {
					model_name = model_name.substr(model_slash_pos + 1);
					std::size_t model_dot_pos = model_name.find_last_of('.');
					if (model_dot_pos != std::string::npos) {
						model_name = model_name.substr(0, model_dot_pos);
					}
				}
				if (ImGui::Selectable(model_name.c_str(), is_selected)) {
					Handle<Model> new_handle = render_manager.get_model_handle(model.name);
					modelinstance.model_handle = new_handle;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("Material", magic_enum::enum_name(modelinstance.material_type).data())) {
			for (auto material : magic_enum::enum_values<MaterialType>()) {
				bool is_selected = (modelinstance.material_type == material);
				if (ImGui::Selectable(magic_enum::enum_name(material).data(), is_selected)) {
					modelinstance.material_type = material;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::TreePop();
	}
}
void Inspector::show_fmodlistener(Entity entity) {
	auto &fmodlistener = ecs_manager.get_component<FmodListener>(entity);
	if (ImGui::TreeNode("FmodListener")) {
		ImGui::Text("Listener: %d", fmodlistener.listener_id);
		ImGui::TreePop();
	}
}
void Inspector::show_collidertag(Entity entity) {
}
void Inspector::show_collidersphere(Entity entity) {
	auto &collidersphere = ecs_manager.get_component<ColliderSphere>(entity);
	if (ImGui::TreeNode("ColliderSphere")) {
		ImGui::DragFloat3("Center", &collidersphere.center.x, 0.1f);
		ImGui::DragFloat("Radius", &collidersphere.radius, 0.1f);
		ImGui::Checkbox("Is movable", &collidersphere.is_movable);
		ImGui::TreePop();
	}
}
void Inspector::show_collideraabb(Entity entity) {
	auto &collideraabb = ecs_manager.get_component<ColliderAABB>(entity);
	if (ImGui::TreeNode("ColliderAABB")) {
		ImGui::DragFloat3("Center", &collideraabb.center.x, 0.1f);
		ImGui::DragFloat3("Range", &collideraabb.range.x, 0.1f);
		ImGui::Checkbox("Is movable", &collideraabb.is_movable);
		ImGui::TreePop();
	}
}
void Inspector::show_colliderobb(Entity entity) {
	auto &colliderobb = ecs_manager.get_component<ColliderOBB>(entity);
	if (ImGui::TreeNode("ColliderOBB")) {
		ImGui::DragFloat3("Center", &colliderobb.center.x, 0.1f);
		ImGui::DragFloat3("Orientation 1", &colliderobb.orientation[0].x, 0.1f);
		ImGui::DragFloat3("Orientation 2", &colliderobb.orientation[1].x, 0.1f);
		ImGui::DragFloat3("Range", &colliderobb.range.x, 0.1f);
		ImGui::Checkbox("Is movable", &colliderobb.is_movable);
		ImGui::TreePop();
	}
}
