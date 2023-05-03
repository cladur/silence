#include "inspector_gui.h"
#include "components/collider_aabb.h"
#include "components/collider_sphere.h"
#include "components/collider_tag_component.h"
#include "components/fmod_listener_component.h"
#include "components/rigidbody_component.h"
#include "render/ecs/model_instance.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#define SHOW_COMPONENT(type, func)                                                                                     \
	if (world->has_component<type>(selected_entity)) {                                                                 \
		func();                                                                                                        \
	}

#define SHOW_ADD_COMPONENT(type)                                                                                       \
	if (!world->has_component<type>(selected_entity)) {                                                                \
		if (ImGui::Selectable(#type)) {                                                                                \
			world->add_component<type>(selected_entity, type{});                                                       \
		}                                                                                                              \
	}

void Inspector::show_components() {
	if (selected_entity <= 0) {
		SPDLOG_WARN("No entity selected");
		return;
	}

	SHOW_COMPONENT(Name, show_name);
	SHOW_COMPONENT(Transform, show_transform);
	SHOW_COMPONENT(RigidBody, show_rigidbody);
	SHOW_COMPONENT(Gravity, show_gravity);
	SHOW_COMPONENT(Parent, show_parent);
	SHOW_COMPONENT(Children, show_children);
	SHOW_COMPONENT(ModelInstance, show_modelinstance);
	SHOW_COMPONENT(FmodListener, show_fmodlistener);
	SHOW_COMPONENT(ColliderTag, show_collidertag);
	SHOW_COMPONENT(ColliderSphere, show_collidersphere);
	SHOW_COMPONENT(ColliderAABB, show_collideraabb);
	SHOW_COMPONENT(ColliderOBB, show_colliderobb);

	for (int i = 0; i < remove_component_queue.size(); i++) {
		auto [entity, component_to_remove] = remove_component_queue.front();
		world->remove_component(entity, component_to_remove);
		remove_component_queue.pop();
	}
}

void Inspector::show_name() {
}

void Inspector::show_transform() {
	auto &transform = world->get_component<Transform>(selected_entity);
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
			remove_component_menu_item<Transform>();

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
	auto &rigidbody = world->get_component<RigidBody>(selected_entity);
	if (ImGui::CollapsingHeader("RigidBody", tree_flags)) {
		remove_component_popup<RigidBody>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_vec3("Velocity", rigidbody.velocity);
		show_vec3("Acceleration", rigidbody.acceleration);

		ImGui::EndTable();
	}
}
void Inspector::show_gravity() {
	auto &gravity = world->get_component<Gravity>(selected_entity);
	if (ImGui::CollapsingHeader("Gravity", tree_flags)) {
		remove_component_popup<Gravity>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_vec3("Gravity", gravity.force);

		ImGui::EndTable();
	}
}
void Inspector::show_parent() {
	auto &parent = world->get_component<Parent>(selected_entity);
	if (ImGui::CollapsingHeader("Parent", tree_flags)) {
		remove_component_popup<Parent>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Model Instance", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		std::string text_value = parent.parent != 0 ? std::to_string(parent.parent) : "None";
		show_text("Parent:", text_value.c_str());

		ImGui::EndTable();
	}
}
void Inspector::show_children() {
	auto &children = world->get_component<Children>(selected_entity);
	if (ImGui::CollapsingHeader("Children", tree_flags)) {
		remove_component_popup<Children>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Model Instance", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);
		bool has_children = children.children_count != 0;

		std::string text_value = has_children ? std::to_string(children.children_count) : "None";
		show_text("Children:", text_value.c_str());
		ImGui::EndTable();
		ImGui::BeginTable("HeaderTable", 3);
		ImGui::TableSetupColumn("##Col11", ImGuiTableColumnFlags_WidthFixed, available_width * 0.1f);
		ImGui::TableSetupColumn("##Col12", ImGuiTableColumnFlags_WidthFixed, available_width * 0.8f);
		ImGui::TableSetupColumn("##Col13", ImGuiTableColumnFlags_WidthFixed, available_width * 0.1f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(1);
		if (has_children && ImGui::CollapsingHeader("Children list", tree_flags)) {
			ImGui::EndTable();
			ImGui::BeginTable("Model Instance2", 2);
			ImGui::TableSetupColumn("##Col12", ImGuiTableColumnFlags_WidthFixed, available_width * 0.2f);
			for (int i = 0; i < children.children_count; i++) {
				show_text("Child: ", children.children[i]);
			}
		}
		ImGui::EndTable();
	}
}
void Inspector::show_modelinstance() {
	auto &modelinstance = world->get_component<ModelInstance>(selected_entity);
	auto models = render_manager.get_models();
	if (ImGui::CollapsingHeader("Model Instance", tree_flags)) {
		remove_component_popup<ModelInstance>();
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
	auto &fmodlistener = world->get_component<FmodListener>(selected_entity);
	if (ImGui::CollapsingHeader("FmodListener", tree_flags)) {
		remove_component_popup<FmodListener>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_text("Listener: ", fmodlistener.listener_id);

		ImGui::EndTable();
	}
}
void Inspector::show_collidertag() {
	if (ImGui::CollapsingHeader("ColliderTag", tree_flags)) {
		remove_component_popup<ColliderTag>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_text("Collider Tag", "Yes");

		ImGui::EndTable();
	}
}
void Inspector::show_collidersphere() {
	auto &collidersphere = world->get_component<ColliderSphere>(selected_entity);
	if (ImGui::CollapsingHeader("ColliderSphere", tree_flags)) {
		remove_component_popup<ColliderSphere>();
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
	auto &collideraabb = world->get_component<ColliderAABB>(selected_entity);
	if (ImGui::CollapsingHeader("ColliderAABB", tree_flags)) {
		remove_component_popup<ColliderAABB>();
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
	auto &colliderobb = world->get_component<ColliderOBB>(selected_entity);
	if (ImGui::CollapsingHeader("ColliderOBB", tree_flags)) {
		remove_component_popup<ColliderOBB>();
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
	std::vector<std::string> component_names = world->get_component_names();
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

			SHOW_ADD_COMPONENT(Name);
			SHOW_ADD_COMPONENT(Transform);
			SHOW_ADD_COMPONENT(RigidBody);
			SHOW_ADD_COMPONENT(Gravity);
			SHOW_ADD_COMPONENT(Parent);
			SHOW_ADD_COMPONENT(Children);
			SHOW_ADD_COMPONENT(ModelInstance);
			SHOW_ADD_COMPONENT(FmodListener);
			SHOW_ADD_COMPONENT(ColliderTag);
			SHOW_ADD_COMPONENT(ColliderSphere);
			SHOW_ADD_COMPONENT(ColliderAABB);
			SHOW_ADD_COMPONENT(ColliderOBB);

			ImGui::EndPopup();
		}
	}
}

void Inspector::set_active_entity(Entity entity) {
	selected_entity = entity;
}