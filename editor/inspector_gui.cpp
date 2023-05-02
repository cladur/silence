#include "inspector_gui.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <glm/gtc/quaternion.hpp>

void Inspector::show_components() {
	if (selected_entity <= 0) {
		SPDLOG_WARN("No entity selected");
		return;
	}

	for (auto component : selected_entity_components) {
		show_component(component);
	}
}
void Inspector::show_component(int signature_index) {
	switch (signature_index) {
		case 0:
			show_name();
			break;
		case 1:
			show_transform();
			break;
		case 2:
			show_rigidbody();
			break;
		case 3:
			show_gravity();
			break;
		case 4:
			show_parent();
			break;
		case 5:
			show_children();
			break;
		case 6:
			show_modelinstance();
			break;
		case 7:
			show_fmodlistener();
			break;
		case 8:
			show_collidertag();
			break;
		case 9:
			show_collidersphere();
			break;
		case 10:
			show_collideraabb();
			break;
		case 11:
			show_colliderobb();
			break;
		default:
			SPDLOG_WARN("Invalid signature index {}", signature_index);
			break;
	}
}
void Inspector::show_name() {
}

void Inspector::show_transform() {
	auto &transform = ecs_manager.get_component<Transform>(selected_entity);
	bool changed = false;
	if (ImGui::CollapsingHeader("Transform", tree_flags)) {
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		glm::vec3 euler_rot = glm::degrees(transform.get_euler_rot());
		glm::vec3 prev_euler_rot = euler_rot;

		changed |= show_vec3("Position", transform.position);
		changed |= show_vec3("Rotation", euler_rot, 1.0f);
		changed |= show_vec3("Scale", transform.scale);

		if (changed) {
			glm::vec3 change = euler_rot - prev_euler_rot;
			transform.add_euler_rot(glm::radians(change));
			transform.set_changed(true);
		}
		ImGui::EndTable();
	}
}
void Inspector::show_rigidbody() {
	auto &rigidbody = ecs_manager.get_component<RigidBody>(selected_entity);
	if (ImGui::TreeNodeEx("RigidBody", tree_flags)) {
		ImGui::DragFloat3("Velocity", &rigidbody.velocity.x, 0.1f);
		ImGui::DragFloat3("Acceleration", &rigidbody.acceleration.x, 0.1f);
		ImGui::TreePop();
	}
}
void Inspector::show_gravity() {
	auto &gravity = ecs_manager.get_component<Gravity>(selected_entity);
	if (ImGui::TreeNodeEx("Gravity", tree_flags)) {
		ImGui::DragFloat3("Gravity", &gravity.force.x, 0.1f);
		ImGui::TreePop();
	}
}
void Inspector::show_parent() {
	auto &parent = ecs_manager.get_component<Parent>(selected_entity);
	if (ImGui::TreeNodeEx("Parent", tree_flags)) {
		ImGui::Text("Parent: %d", parent.parent);
		ImGui::TreePop();
	}
}
void Inspector::show_children() {
	auto &children = ecs_manager.get_component<Children>(selected_entity);
	if (ImGui::TreeNodeEx("Children", tree_flags)) {
		ImGui::Text("Children: %d", children.children_count);
		if (ImGui::TreeNodeEx("Children list", tree_flags)) {
			for (int i = 0; i < children.children_count; i++) {
				ImGui::Text("%d", children.children[i]);
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}
void Inspector::show_modelinstance() {
	auto &modelinstance = ecs_manager.get_component<ModelInstance>(selected_entity);
	auto models = render_manager.get_models();
	if (ImGui::CollapsingHeader("Model Instance", tree_flags)) {
		std::string name = render_manager.get_model(modelinstance.model_handle).name;
		std::size_t last_slash_pos = name.find_last_of("/\\");

		if (last_slash_pos != std::string::npos) {
			name = name.substr(last_slash_pos + 1);
			std::size_t dot_pos = name.find_last_of('.');
			if (dot_pos != std::string::npos) {
				name = name.substr(0, dot_pos);
			}
		}

		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Model Instance", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Model");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Model", name.c_str())) {
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
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Material");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Material", magic_enum::enum_name(modelinstance.material_type).data())) {
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
		ImGui::EndTable();
	}
}
void Inspector::show_fmodlistener() {
	auto &fmodlistener = ecs_manager.get_component<FmodListener>(selected_entity);
	if (ImGui::TreeNodeEx("FmodListener", tree_flags)) {
		ImGui::Text("Listener: %d", fmodlistener.listener_id);
		ImGui::TreePop();
	}
}
void Inspector::show_collidertag() {
}
void Inspector::show_collidersphere() {
	auto &collidersphere = ecs_manager.get_component<ColliderSphere>(selected_entity);
	if (ImGui::TreeNodeEx("ColliderSphere", tree_flags)) {
		ImGui::DragFloat3("Center", &collidersphere.center.x, 0.1f);
		ImGui::DragFloat("Radius", &collidersphere.radius, 0.1f);
		ImGui::Checkbox("Is movable", &collidersphere.is_movable);
		ImGui::TreePop();
	}
}
void Inspector::show_collideraabb() {
	auto &collideraabb = ecs_manager.get_component<ColliderAABB>(selected_entity);
	if (ImGui::TreeNodeEx("ColliderAABB", tree_flags)) {
		ImGui::DragFloat3("Center", &collideraabb.center.x, 0.1f);
		ImGui::DragFloat3("Range", &collideraabb.range.x, 0.1f);
		ImGui::Checkbox("Is movable", &collideraabb.is_movable);
		ImGui::TreePop();
	}
}
void Inspector::show_colliderobb() {
	auto &colliderobb = ecs_manager.get_component<ColliderOBB>(selected_entity);
	if (ImGui::TreeNodeEx("ColliderOBB", tree_flags)) {
		ImGui::DragFloat3("Center", &colliderobb.center.x, 0.1f);
		ImGui::DragFloat3("Orientation 1", &colliderobb.orientation[0].x, 0.1f);
		ImGui::DragFloat3("Orientation 2", &colliderobb.orientation[1].x, 0.1f);
		ImGui::DragFloat3("Range", &colliderobb.range.x, 0.1f);
		ImGui::Checkbox("Is movable", &colliderobb.is_movable);
		ImGui::TreePop();
	}
}
bool Inspector::show_vec3(const char *label, glm::vec3 &vec3, float speed) {
	bool changed = false;
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::Text("%s", label);
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	changed |= ImGui::DragFloat3(fmt::format("##{}", label).c_str(), &vec3.x, speed);
	return changed;
}
void Inspector::show_add_component() {
	std::vector<std::string> component_names = ecs_manager.get_component_names();
	float available_width = ImGui::GetContentRegionAvail().x;
	bool open_popup = false;

	ImGui::BeginTable("table1", 3, ImGuiTableFlags_SizingFixedFit);
	ImGui::TableSetupColumn("Column 1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.15f);
	ImGui::TableSetupColumn("Column 2", ImGuiTableColumnFlags_WidthFixed, available_width * 0.7f);
	ImGui::TableSetupColumn("Column 3", ImGuiTableColumnFlags_WidthFixed, available_width * 0.15f);

	// Add table contents here
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::TableSetColumnIndex(1);
	if (ImGui::Button("Add Component", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
		open_popup = true;
	}
	ImGui::TableSetColumnIndex(2);

	ImGui::EndTable();

	if (open_popup) {
		ImGui::OpenPopup("add_component_popup");
	}

	if (ImGui::BeginPopup("add_component_popup")) {
		ImGui::SeparatorText("Components");
		for (auto component_id : not_selected_entity_components) {
			if (ImGui::Selectable(component_names[component_id].c_str())) {
				ecs_manager.add_component(selected_entity, component_id);
				set_active_entity(selected_entity);
			}
		}
		ImGui::EndPopup();
	}
}

void Inspector::set_active_entity(Entity entity) {
	if (entity == selected_entity) {
		return;
	}

	selected_entity = entity;
	selected_entity_signature = ecs_manager.get_entity_signature(entity);
	int number_of_components = ecs_manager.get_registered_components();

	selected_entity_components.clear();
	for (int i = 0; i < number_of_components; i++) {
		if (selected_entity_signature[i] == true) {
			selected_entity_components.push_back(i);
		} else {
			not_selected_entity_components.push_back(i);
		}
	}
}
