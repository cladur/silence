#include "inspector_gui.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

Inspector::Inspector() {
	type_to_show_functions_map[typeid(Name)] = [this]() { show_name(); };
	type_to_show_functions_map[typeid(Transform)] = [this]() { show_transform(); };
	type_to_show_functions_map[typeid(RigidBody)] = [this]() { show_rigidbody(); };
	type_to_show_functions_map[typeid(Gravity)] = [this]() { show_gravity(); };
	type_to_show_functions_map[typeid(Parent)] = [this]() { show_parent(); };
	type_to_show_functions_map[typeid(Children)] = [this]() { show_children(); };
	type_to_show_functions_map[typeid(ModelInstance)] = [this]() { show_modelinstance(); };
	type_to_show_functions_map[typeid(FmodListener)] = [this]() { show_fmodlistener(); };
	type_to_show_functions_map[typeid(ColliderTag)] = [this]() { show_collidertag(); };
	type_to_show_functions_map[typeid(ColliderSphere)] = [this]() { show_collidersphere(); };
	type_to_show_functions_map[typeid(ColliderAABB)] = [this]() { show_collideraabb(); };
	type_to_show_functions_map[typeid(ColliderOBB)] = [this]() { show_colliderobb(); };
}

Inspector &Inspector::get() {
	static Inspector instance;
	return instance;
}

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
	Inspector &inspector = Inspector::get();
	inspector.show_component_map[signature_index]();
}
void Inspector::show_name() {
}

void Inspector::show_transform() {
	auto &transform = ecs_manager.get_component<Transform>(selected_entity);
	bool changed = false;
	if (ImGui::CollapsingHeader("Transform", tree_flags)) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup("TransformContextMenu");
		}
		if (ImGui::BeginPopup("TransformContextMenu")) {
			if (ImGui::MenuItem("Reset Transform")) {
				transform.position = glm::vec3(0.0f);
				transform.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
				transform.scale = glm::vec3(1.0f);
				transform.set_changed(true);
			}
			ImGui::EndPopup();
		}
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		glm::vec3 euler_rot = glm::degrees(transform.get_euler_rot());
		glm::vec3 prev_euler_rot = euler_rot;

		changed |= show_vec3("Position", transform.position);
		changed |= show_vec3("Rotation", euler_rot, 1.0f);
		changed |= show_vec3("Scale", transform.scale, 0.1f, 1.0f);

		if (changed) {
			glm::vec3 change = glm::radians(euler_rot - prev_euler_rot);
			transform.add_euler_rot(glm::vec3(change.x, 0.0f, 0.0f));
			transform.add_euler_rot(glm::vec3(0.0f, change.y, 0.0f));
			transform.add_euler_rot(glm::vec3(0.0f, 0.0f, change.z));
			transform.set_changed(true);
		}
		ImGui::EndTable();
	}
}
void Inspector::show_rigidbody() {
	auto &rigidbody = ecs_manager.get_component<RigidBody>(selected_entity);
	if (ImGui::CollapsingHeader("RigidBody", tree_flags)) {
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_vec3("Velocity", rigidbody.velocity);
		show_vec3("Acceleration", rigidbody.acceleration);

		ImGui::EndTable();
	}
}
void Inspector::show_gravity() {
	auto &gravity = ecs_manager.get_component<Gravity>(selected_entity);
	if (ImGui::CollapsingHeader("Gravity", tree_flags)) {
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_vec3("Gravity", gravity.force);

		ImGui::EndTable();
	}
}
void Inspector::show_parent() {
	auto &parent = ecs_manager.get_component<Parent>(selected_entity);
	if (ImGui::CollapsingHeader("Parent", tree_flags)) {
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Model Instance", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		std::string text_value = parent.parent != 0 ? std::to_string(parent.parent) : "None";
		show_text("Parent:", text_value.c_str());

		ImGui::EndTable();
	}
}
void Inspector::show_children() {
	auto &children = ecs_manager.get_component<Children>(selected_entity);
	if (ImGui::CollapsingHeader("Children", tree_flags)) {
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Model Instance", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);
		bool has_children = children.children_count != 0;

		std::string text_value = has_children ? std::to_string(children.children_count) : "None";
		show_text("Children:", text_value.c_str());
		if (has_children && ImGui::CollapsingHeader("Children list", tree_flags)) {
			for (int i = 0; i < children.children_count; i++) {
				show_text("Child: ", children.children[i]);
			}
		}

		ImGui::EndTable();
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
	if (ImGui::CollapsingHeader("FmodListener", tree_flags)) {
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_text("Listener: ", fmodlistener.listener_id);

		ImGui::EndTable();
	}
}
void Inspector::show_collidertag() {
	if (ImGui::CollapsingHeader("ColliderTag", tree_flags)) {
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_text("Collider Tag", "Yes");

		ImGui::EndTable();
	}
}
void Inspector::show_collidersphere() {
	auto &collidersphere = ecs_manager.get_component<ColliderSphere>(selected_entity);
	if (ImGui::CollapsingHeader("ColliderSphere", tree_flags)) {
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_vec3("Center", collidersphere.center);
		show_float("Radius", collidersphere.radius);
		show_checkbox("Is movable", collidersphere.is_movable);

		ImGui::EndTable();
	}
}
void Inspector::show_collideraabb() {
	auto &collideraabb = ecs_manager.get_component<ColliderAABB>(selected_entity);
	if (ImGui::CollapsingHeader("ColliderAABB", tree_flags)) {
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_vec3("Center", collideraabb.center);
		show_vec3("Range", collideraabb.range);
		show_checkbox("Is movable", collideraabb.is_movable);

		ImGui::EndTable();
	}
}
void Inspector::show_colliderobb() {
	auto &colliderobb = ecs_manager.get_component<ColliderOBB>(selected_entity);
	if (ImGui::CollapsingHeader("ColliderOBB", tree_flags)) {
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_vec3("Center", colliderobb.center);
		show_vec3("Orientation 1", colliderobb.orientation[0]);
		show_vec3("Orientation 2", colliderobb.orientation[1]);
		show_vec3("Range", colliderobb.range);
		show_checkbox("Is movable", colliderobb.is_movable);

		ImGui::EndTable();
	}
}
bool Inspector::show_vec3(const char *label, glm::vec3 &vec3, float speed, float reset_value) {
	bool changed = false;

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::Text("%s", label);
	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
		vec3 = glm::vec3(reset_value);
		changed = true;
	}
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	changed |= ImGui::DragFloat3(fmt::format("##{}", label).c_str(), &vec3.x, speed);
	return changed;
}

bool Inspector::show_float(const char *label, float &value, float speed) {
	bool changed = false;

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::Text("%s", label);
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	changed |= ImGui::DragFloat(fmt::format("##{}", label).c_str(), &value, speed);
	return changed;
}

void Inspector::show_checkbox(const char *label, bool &value) {
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::Text("%s", label);
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::Checkbox(fmt::format("##{}", label).c_str(), &value);
}

void Inspector::show_text(const char *label, const char *value) {
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::Text("%s", label);
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::Text("%s", value);
}

void Inspector::show_text(const char *label, int value) {
	std::string value_str = std::to_string(value);

	Inspector::show_text(label, value_str.c_str());
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
		ImGui::SetNextWindowFocus();
	}

	ImGui::SetNextWindowSize(ImVec2(200, 0));
	if (ImGui::IsPopupOpen("add_component_popup")) {
		ImGui::GetIO().WantTextInput = true;
		if (ImGui::BeginPopup("add_component_popup")) {
			// We only want to set the focus once, at the beginning
			if (open_popup) {
				ImGui::SetKeyboardFocusHere();
			}
			float components_filter_available_width = ImGui::GetContentRegionAvail().x;
			static ImGuiTextFilter components_filter;
			components_filter.Draw("##component_filter", components_filter_available_width - 5);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			for (auto component_id : not_selected_entity_components) {
				const char *component_name = component_names[component_id].c_str();

				if (!components_filter.PassFilter(component_name)) {
					continue;
				}

				if (ImGui::Selectable(component_name)) {
					ecs_manager.add_component(selected_entity, component_id);
					set_active_entity(selected_entity);
					break;
				}
			}
			ImGui::EndPopup();
		}
	}
}

void Inspector::set_active_entity(Entity entity) {
	if (entity == selected_entity) {
		refresh_entity();
		return;
	}

	selected_entity = entity;
	refresh_entity();
}

void Inspector::add_mapping(int signature_index, std::function<void()> func) {
	Inspector &inspector = Inspector::get();
	inspector.show_component_map[signature_index] = std::move(func);
}
void Inspector::refresh_entity() {
	selected_entity_signature = ecs_manager.get_entity_signature(selected_entity);
	int number_of_components = ecs_manager.get_registered_components();

	selected_entity_components.clear();
	not_selected_entity_components.clear();
	for (int i = 0; i < number_of_components; i++) {
		if (selected_entity_signature[i] == true) {
			selected_entity_components.push_back(i);
		} else {
			not_selected_entity_components.push_back(i);
		}
	}
}
