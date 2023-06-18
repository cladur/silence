#include "inspector_gui.h"
#include "components/agent_data_component.h"
#include "components/collider_aabb.h"
#include "components/collider_sphere.h"
#include "components/collider_tag_component.h"
#include "components/detection_camera_component.h"
#include "components/enemy_data_component.h"
#include "components/exploding_box_component.h"
#include "components/fmod_listener_component.h"
#include "components/highlight_component.h"
#include "components/interactable_component.h"
#include "components/light_component.h"
#include "components/path_node_component.h"
#include "components/path_parent_component.h"
#include "components/platform_component.h"
#include "components/rigidbody_component.h"
#include "physics/physics_manager.h"
#include "render/ecs/model_instance.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include "audio/audio_manager.h"
#include "components/particle_emitter_component.h"
#include "editor.h"
#include "render/ecs/billboard_component.h"

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

glm::vec3 Inspector::copied_vector3;

void Inspector::show_components() {
	if (selected_entity <= 0) {
		SPDLOG_WARN("No entity selected");
		return;
	}

	ImGui::Text("ID: %d", selected_entity);
	ImGui::Spacing();

	SHOW_COMPONENT(Name, show_name);
	SHOW_COMPONENT(Transform, show_transform);
	SHOW_COMPONENT(RigidBody, show_rigidbody);
	SHOW_COMPONENT(Parent, show_parent);
	SHOW_COMPONENT(Children, show_children);
	SHOW_COMPONENT(ModelInstance, show_modelinstance);
	SHOW_COMPONENT(SkinnedModelInstance, show_skinnedmodelinstance);
	SHOW_COMPONENT(AnimationInstance, show_animationinstance);
	SHOW_COMPONENT(Attachment, show_attachment);
	SHOW_COMPONENT(FmodListener, show_fmodlistener);
	SHOW_COMPONENT(Camera, show_camera);
	SHOW_COMPONENT(ColliderTag, show_collidertag);
	SHOW_COMPONENT(StaticTag, show_statictag);
	SHOW_COMPONENT(ColliderSphere, show_collidersphere);
	SHOW_COMPONENT(ColliderAABB, show_collideraabb);
	SHOW_COMPONENT(ColliderOBB, show_colliderobb);
	SHOW_COMPONENT(ColliderCapsule, show_collidercapsule);
	SHOW_COMPONENT(Light, show_light);
	SHOW_COMPONENT(AgentData, show_agent_data);
	SHOW_COMPONENT(HackerData, show_hacker_data);
	SHOW_COMPONENT(EnemyPath, show_enemy_path);
	SHOW_COMPONENT(Interactable, show_interactable);
	SHOW_COMPONENT(Platform, show_platform);
	SHOW_COMPONENT(EnemyData, show_enemy_data);
	SHOW_COMPONENT(ExplodingBox, show_exploding_box);
	SHOW_COMPONENT(Billboard, show_billboard);
	SHOW_COMPONENT(PathNode, show_path_node);
	SHOW_COMPONENT(PathParent, show_path_parent);
	SHOW_COMPONENT(Taggable, show_taggable);
	SHOW_COMPONENT(FMODEmitter, show_fmod_emitter);
	SHOW_COMPONENT(Highlight, show_highlight);
	SHOW_COMPONENT(ParticleEmitter, show_particle_emitter);
	SHOW_COMPONENT(DetectionCamera, show_detection_camera);
	SHOW_COMPONENT(Decal, show_decal);

	for (int i = 0; i < remove_component_queue.size(); i++) {
		auto [entity, component_to_remove] = remove_component_queue.front();
		world->remove_component(entity, component_to_remove);
		remove_component_queue.pop();
	}
}

void Inspector::show_name() {
	ImGui::CollapsingHeader("Name");

	remove_component_popup<Name>();
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
		changed |= show_vec3("Scale", transform.scale, 0.02f, 1.0f, 0.001f, 100.0f);

		for (int i = 0; i < 3; i++) {
			if (transform.scale[i] < 0.001f) {
				transform.scale[i] = 0.001f;
			}
		}

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
		show_float("mass", rigidbody.mass);

		ImGui::EndTable();
	}
}

void Inspector::show_parent() {
	Entity parent = world->get_component<Parent>(selected_entity).parent;
	if (ImGui::CollapsingHeader("Parent", tree_flags)) {
		remove_component_popup<Parent>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Parent", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		EditorScene &scene = Editor::get()->get_active_scene();

		std::string text_value = "None";
		if (parent != 0) {
			if (world->has_component<Name>(parent)) {
				text_value = world->get_component<Name>(parent).name;
			} else {
				text_value = std::to_string(parent);
			}
		}
		show_text("Parent:", text_value.c_str());

		ImGui::EndTable();
	}
}

void Inspector::show_children() {
	auto &children = world->get_component<Children>(selected_entity);
	if (ImGui::CollapsingHeader("Children", tree_flags)) {
		remove_component_popup<Children>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Children", 2);
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
			ImGui::BeginTable("Children2", 2);
			ImGui::TableSetupColumn("##Col12", ImGuiTableColumnFlags_WidthFixed, available_width * 0.2f);
			for (int i = 0; i < children.children_count; i++) {
				show_text("Child: ", children.children[i]);
			}
		}
		ImGui::EndTable();
	}
}

void Inspector::show_skinnedmodelinstance() {
	auto &modelinstance = world->get_component<SkinnedModelInstance>(selected_entity);
	auto &models = resource_manager.get_skinned_models();
	if (ImGui::CollapsingHeader("Skinned Model Instance", tree_flags)) {
		remove_component_popup<SkinnedModelInstance>();
		std::string name = resource_manager.get_skinned_model(modelinstance.model_handle).name;
		std::size_t last_slash_pos = name.find_last_of("/\\");

		if (last_slash_pos != std::string::npos) {
			name = name.substr(last_slash_pos + 1);
			std::size_t dot_pos = name.find_last_of('.');
			if (dot_pos != std::string::npos) {
				name = name.substr(0, dot_pos);
			}
		}

		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Skinned Model Instance", 2);
		ImGui::TableSetupColumn("##Skinned Model Instance", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Model");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Skinned Model", name.c_str())) {
			for (const auto &model : models) {
				bool is_selected =
						(modelinstance.model_handle.id == resource_manager.get_skinned_model_handle(model.name).id);
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
					Handle<SkinnedModel> new_handle = resource_manager.get_skinned_model_handle(model.name);
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
		if (ImGui::BeginCombo("##Skinned Material", magic_enum::enum_name(modelinstance.material_type).data())) {
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

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Cast shadow");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::Checkbox("##Cast shadow", &modelinstance.in_shadow_pass);
		ImGui::EndTable();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_SKINMODEL_PATH")) {
				std::string payload_n = *(const std::string *)payload->Data;
				std::string search_string = "\\";
				std::string replace_string = "/";

				size_t pos = payload_n.find(search_string);
				while (pos != std::string::npos) {
					payload_n.replace(pos, search_string.length(), replace_string);
					pos = payload_n.find(search_string, pos + replace_string.length());
				}
				resource_manager.load_skinned_model(payload_n.c_str());
				modelinstance.model_handle = resource_manager.get_skinned_model_handle(payload_n);
			}

			ImGui::EndDragDropTarget();
		}
	}
}

void Inspector::show_modelinstance() {
	auto &modelinstance = world->get_component<ModelInstance>(selected_entity);
	auto &models = resource_manager.get_models();
	if (ImGui::CollapsingHeader("Model Instance", tree_flags)) {
		remove_component_popup<ModelInstance>();
		std::string name = resource_manager.get_model(modelinstance.model_handle).name;
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
				bool is_selected = (modelinstance.model_handle.id == resource_manager.get_model_handle(model.name).id);
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
					Handle<Model> new_handle = resource_manager.get_model_handle(model.name);
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
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("UV Scale");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::Checkbox("##UV Scale", &modelinstance.scale_uv_with_transform);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Cast shadow");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::Checkbox("##Cast shadow", &modelinstance.in_shadow_pass);
		ImGui::EndTable();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_MODEL_PATH")) {
				std::string payload_n = *(const std::string *)payload->Data;
				std::string search_string = "\\";
				std::string replace_string = "/";

				size_t pos = payload_n.find(search_string);
				while (pos != std::string::npos) {
					payload_n.replace(pos, search_string.length(), replace_string);
					pos = payload_n.find(search_string, pos + replace_string.length());
				}
				resource_manager.load_model(payload_n.c_str());
				modelinstance.model_handle = resource_manager.get_model_handle(payload_n);
			}

			ImGui::EndDragDropTarget();
		}
	}
}

void Inspector::show_animationinstance() {
	auto &animation_instance = world->get_component<AnimationInstance>(selected_entity);
	auto &animations = resource_manager.get_animations();
	if (ImGui::CollapsingHeader("Animation Instance", tree_flags)) {
		remove_component_popup<AnimationInstance>();
		auto &selected_animation = resource_manager.get_animation(animation_instance.animation_handle);
		std::string name = selected_animation.name;
		std::size_t last_slash_pos = name.find_last_of("/\\");

		if (last_slash_pos != std::string::npos) {
			name = name.substr(last_slash_pos + 1);
			std::size_t dot_pos = name.find_last_of('.');
			if (dot_pos != std::string::npos) {
				name = name.substr(0, dot_pos);
			}
		}

		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Animation Instance", 2);
		ImGui::TableSetupColumn("##Animation Instance", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Animation");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Animation", name.c_str())) {
			for (const auto &animation : animations) {
				bool is_selected = (animation_instance.animation_handle.id ==
						resource_manager.get_animation_handle(animation.name).id);

				auto animation_handle = resource_manager.get_animation_handle(animation.name);
				std::string animation_name = animation.name;
				std::size_t animation_slash_pos = animation_name.find_last_of("/\\");
				if (animation_slash_pos != std::string::npos) {
					animation_name = animation_name.substr(animation_slash_pos + 1);
					std::size_t animation_dot_pos = animation_name.find_last_of('.');
					if (animation_dot_pos != std::string::npos) {
						animation_name = animation_name.substr(0, animation_dot_pos);
					}
				}

				if (ImGui::Selectable(animation_name.c_str(), is_selected)) {
					animation_instance.animation_handle = animation_handle;
					animation_instance.current_time = 0.0f;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Loop");
		ImGui::TableSetColumnIndex(1);
		ImGui::Checkbox("##Is looping", &animation_instance.is_looping);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Freeze");
		ImGui::TableSetColumnIndex(1);
		ImGui::Checkbox("##Is freeze", &animation_instance.is_freeze);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Ticks per second");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::DragFloat("##Ticks per second", &animation_instance.ticks_per_second, 20.0f, 0, 50000);

		ImGui::EndTable();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ANIMATION_PATH")) {
				std::string payload_n = *(const std::string *)payload->Data;
				std::string search_string = "\\";
				std::string replace_string = "/";

				size_t pos = payload_n.find(search_string);
				while (pos != std::string::npos) {
					payload_n.replace(pos, search_string.length(), replace_string);
					pos = payload_n.find(search_string, pos + replace_string.length());
				}
				resource_manager.load_animation(payload_n.c_str());
				animation_instance.animation_handle = resource_manager.get_animation_handle(payload_n);
			}

			ImGui::EndDragDropTarget();
		}
	}
}

void Inspector::show_attachment() {
	auto &attachment = world->get_component<Attachment>(selected_entity);
	if (ImGui::CollapsingHeader("Attachment", tree_flags)) {
		remove_component_popup<Attachment>();

		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Bone name");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Bone name", attachment.bone_name.c_str())) {
			if (attachment.holder != -1) {
				SkinnedModelInstance &instance = world->get_component<SkinnedModelInstance>(attachment.holder);
				SkinnedModel &model = resource_manager.get_skinned_model(instance.model_handle);
				for (const auto &pair : model.joint_map) {
					bool is_selected = (attachment.bone_name == pair.first);

					if (ImGui::Selectable(pair.first.c_str(), is_selected)) {
						attachment.bone_name = pair.first;
					}
					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
			}
			ImGui::EndCombo();
		}
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Holder ");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		std::string name;
		if (attachment.holder == -1) {
			name = "None";
		} else if (world->has_component<Name>(attachment.holder)) {
			name = world->get_component<Name>(attachment.holder).name;
		} else {
			name = std::to_string(attachment.holder);
		}

		if (ImGui::BeginCombo("##Holder", name.c_str())) {
			for (auto entity : world->get_parent_scene()->entities) {
				if (!world->has_component<SkinnedModelInstance>(entity)) {
					continue;
				}
				bool is_selected = (attachment.holder == entity);
				std::string other_name;
				if (world->has_component<Name>(entity)) {
					other_name = world->get_component<Name>(entity).name;
				} else {
					other_name = std::to_string(entity);
				}
				if (ImGui::Selectable(other_name.c_str(), is_selected)) {
					attachment.holder = entity;
					attachment.bone_name = "";
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
		ImGui::BeginTable("FmodListener", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_text("Listener: ", fmodlistener.listener_id);

		ImGui::EndTable();
	}
}

void Inspector::show_camera() {
	auto &camera = world->get_component<Camera>(selected_entity);
	if (ImGui::CollapsingHeader("Camera", tree_flags)) {
		remove_component_popup<Camera>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_float("Fov", camera.fov);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Right Side");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::Checkbox("##Right Side", &camera.right_side);

		ImGui::EndTable();
	}
}

void Inspector::show_collidertag() {
	if (ImGui::CollapsingHeader("ColliderTag", tree_flags)) {
		remove_component_popup<ColliderTag>();
		PhysicsManager &physics_manager = PhysicsManager::get();
		auto &tag = world->get_component<ColliderTag>(selected_entity);
		const auto &map = physics_manager.get_layers_map();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Collider Tag", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Layer");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Layer", tag.layer_name.c_str())) {
			for (const auto &pair : map) {
				bool is_selected = tag.layer_name == pair.first;

				if (ImGui::Selectable(pair.first.c_str(), is_selected)) {
					tag.layer_name = pair.first;
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

void Inspector::show_statictag() {
	if (ImGui::CollapsingHeader("StaticTag", tree_flags)) {
		remove_component_popup<StaticTag>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_text("Static Tag", "Yes");

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

		ImGui::EndTable();
	}
}

void Inspector::show_collidercapsule() {
	auto &collider_capsule = world->get_component<ColliderCapsule>(selected_entity);
	if (ImGui::CollapsingHeader("ColliderCapsule", tree_flags)) {
		remove_component_popup<ColliderCapsule>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Transform", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_float("Radius", collider_capsule.radius);
		show_vec3("Start", collider_capsule.start);
		show_vec3("End", collider_capsule.end);

		ImGui::EndTable();
	}
}

void Inspector::show_light() {
	auto &light = world->get_component<Light>(selected_entity);
	if (ImGui::CollapsingHeader("Light", tree_flags)) {
		remove_component_popup<Light>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Light", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Color");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::ColorPicker3("##Color", (float *)&light.color);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Intensity");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::DragFloat("##Intensity", &light.intensity, 0.01f, 0.0f, 100.0f);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Cast shadow");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::Checkbox("##Cast shadow", &light.cast_shadow);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Is On");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::Checkbox("##Is On", &light.is_on);

		if (light.type == LightType::SPOT_LIGHT) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Cutoff");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::DragFloat("##Cutoff", &light.cutoff, 0.01f, 0.0f, 200.0f);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Outer cutoff");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::DragFloat("##Outer cutoff", &light.outer_cutoff, 0.01f, 0.0f, 200.0f);
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Light type");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Light type", magic_enum::enum_name(light.type).data())) {
			for (auto type : magic_enum::enum_values<LightType>()) {
				bool is_selected = (light.type == type);
				if (ImGui::Selectable(magic_enum::enum_name(type).data(), is_selected)) {
					light.type = type;
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

void Inspector::show_hacker_data() {
	auto &hacker_data = world->get_component<HackerData>(selected_entity);
	if (ImGui::CollapsingHeader("Hacker Data", tree_flags)) {
		remove_component_popup<HackerData>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Hacker Data", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Hacker Model");
		ImGui::TableSetColumnIndex(1);
		ImGui::InputInt("", (int *)&hacker_data.model, 0, 0);

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				hacker_data.model = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Camera Pivot");
		ImGui::TableSetColumnIndex(1);
		ImGui::InputInt("", (int *)&hacker_data.camera_pivot, 0, 0);

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				hacker_data.camera_pivot = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Scorpion camera transform");
		ImGui::TableSetColumnIndex(1);
		ImGui::InputInt("", (int *)&hacker_data.scorpion_camera_transform, 0, 0);

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				hacker_data.scorpion_camera_transform = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Camera");
		ImGui::TableSetColumnIndex(1);
		ImGui::InputInt("", (int *)&hacker_data.camera, 0, 0);

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				hacker_data.camera = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::EndTable();
	}
}

void Inspector::show_agent_data() {
	auto &agent_data = world->get_component<AgentData>(selected_entity);
	if (ImGui::CollapsingHeader("Agent Data", tree_flags)) {
		remove_component_popup<AgentData>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Agent Data", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Agent Model");
		ImGui::TableSetColumnIndex(1);
		ImGui::InputInt("", (int *)&agent_data.model, 0, 0);

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				agent_data.model = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Camera Pivot");
		ImGui::TableSetColumnIndex(1);
		ImGui::InputInt("", (int *)&agent_data.camera_pivot, 0, 0);

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				agent_data.camera_pivot = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Spring Arm");
		ImGui::TableSetColumnIndex(1);
		ImGui::InputInt("", (int *)&agent_data.spring_arm, 0, 0);

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				agent_data.spring_arm = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Camera");
		ImGui::TableSetColumnIndex(1);
		ImGui::InputInt("", (int *)&agent_data.camera, 0, 0);

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				agent_data.camera = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::EndTable();
	}
}

void Inspector::show_enemy_path() {
	auto &enemy_path = world->get_component<EnemyPath>(selected_entity);
	if (ImGui::CollapsingHeader("Enemy Path", tree_flags)) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup("EnemyPathContextMenu");
		}
		if (ImGui::BeginPopup("EnemyPathContextMenu")) {
			if (ImGui::MenuItem("Reset Path")) {
				enemy_path.path_parent = -1;
				enemy_path.speed = 1.0f;
				enemy_path.rotation_speed = 1.0f;
			}
			remove_component_menu_item<EnemyPath>();

			ImGui::EndPopup();
		}

		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Enemy Path", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		int i = 0;
		show_float("Speed", enemy_path.speed);
		show_float("Rot Speed", enemy_path.rotation_speed);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Path Parent: %d", enemy_path.path_parent);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				enemy_path.path_parent = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::EndTable();
	}
}

void Inspector::show_path_node() {
	auto &path_node = world->get_component<PathNode>(selected_entity);
	if (ImGui::CollapsingHeader("Path Node", tree_flags)) {
		remove_component_popup<PathNode>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Path Node", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_checkbox("Patrol Point", path_node.is_patrol_point);
		show_float("Patrol Time", path_node.patrol_time);

		ImGui::EndTable();
	}
}

void Inspector::show_path_parent() {
	auto &path_parent = world->get_component<PathParent>(selected_entity);
	if (ImGui::CollapsingHeader("Path Parent", tree_flags)) {
		remove_component_popup<PathParent>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Path Parent", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		ImGui::EndTable();
	}
}

void Inspector::show_interactable() {
	auto &interactable = world->get_component<Interactable>(selected_entity);
	if (ImGui::CollapsingHeader("Interactable", tree_flags)) {
		remove_component_popup<Interactable>();
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Interactable", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Interaction type");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Interaction type", magic_enum::enum_name(interactable.type).data())) {
			for (auto type : magic_enum::enum_values<InteractionType>()) {
				bool is_selected = (interactable.type == type);
				if (ImGui::Selectable(magic_enum::enum_name(type).data(), is_selected)) {
					interactable.type = type;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Interaction");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Interaction", magic_enum::enum_name(interactable.interaction).data())) {
			for (auto type : magic_enum::enum_values<Interaction>()) {
				bool is_selected = (interactable.interaction == type);
				if (ImGui::Selectable(magic_enum::enum_name(type).data(), is_selected)) {
					interactable.interaction = type;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		show_checkbox("Single use", interactable.single_use);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Interaction target 1");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%s", fmt::format("{}", interactable.interaction_targets[0]).c_str());

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				interactable.interaction_targets[0] = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Interaction target 2");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%s", fmt::format("{}", interactable.interaction_targets[1]).c_str());

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				interactable.interaction_targets[1] = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Interaction target 3");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%s", fmt::format("{}", interactable.interaction_targets[2]).c_str());

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				interactable.interaction_targets[2] = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Interaction target 4");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%s", fmt::format("{}", interactable.interaction_targets[3]).c_str());

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				interactable.interaction_targets[3] = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Interaction target 5");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%s", fmt::format("{}", interactable.interaction_targets[4]).c_str());

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				interactable.interaction_targets[4] = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::EndTable();
	}
}

void Inspector::show_platform() {
	auto &platform = world->get_component<Platform>(selected_entity);

	if (ImGui::CollapsingHeader("Platform", tree_flags)) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup("PlatformContextMenu");
		}
		if (ImGui::BeginPopup("PlatformContextMenu")) {
			if (ImGui::MenuItem("Reset Platform")) {
				platform.starting_position = glm::vec3(0.0f);
				platform.ending_position = glm::vec3(0.0f);
			}
			remove_component_menu_item<Platform>();

			ImGui::EndPopup();
		}
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Platform Component", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		bool changed = false;

		show_checkbox("Is door", platform.is_door);

		changed |= show_vec3("Starting position", platform.starting_position);
		changed |= show_vec3("Ending position", platform.ending_position);

		show_float("Platform speed", platform.speed);

		ImGui::EndTable();
	}
}

void Inspector::show_exploding_box() {
	auto &exploding_box = world->get_component<ExplodingBox>(selected_entity);

	if (ImGui::CollapsingHeader("ExplodingBox", tree_flags)) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup("ExplodingBoxContextMenu");
		}
		if (ImGui::BeginPopup("ExplodingBoxContextMenu")) {
			if (ImGui::MenuItem("Reset ExplodingBox")) {
				exploding_box.explosion_radius = 0.0f;
				exploding_box.distraction_radius = 0.0f;
				exploding_box.distraction_time = 1.0f;
			}
			remove_component_menu_item<ExplodingBox>();

			ImGui::EndPopup();
		}
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("ExplodingBox Component", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_float("Explosion radius", exploding_box.explosion_radius);
		show_float("Distraction radius", exploding_box.distraction_radius);
		show_float("Distraction time", exploding_box.distraction_time);

		ImGui::EndTable();
	}
}

void Inspector::show_enemy_data() {
	auto &data = world->get_component<EnemyData>(selected_entity);
	if (ImGui::CollapsingHeader("Enemy Data", tree_flags)) {
		remove_component_popup<EnemyData>();

		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Enemy Path", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		ImGui::EndTable();
	}
}

void Inspector::show_billboard() {
	auto &billboard = world->get_component<Billboard>(selected_entity);

	if (ImGui::CollapsingHeader("Billboard", tree_flags)) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup("BillboardContextMenu");
		}
		if (ImGui::BeginPopup("BillboardContextMenu")) {
			if (ImGui::MenuItem("Reset Billboard")) {
				billboard.texture = Handle<Texture>(0);
				billboard.scale = glm::vec2(1.0f);
				billboard.color = glm::vec4(1.0f);
			}
			remove_component_menu_item<Billboard>();

			ImGui::EndPopup();
		}
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Billboard Component", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_vec3("Position", billboard.position_offset);
		show_float("Z Offset", billboard.billboard_z_offset);
		show_vec2("Size", billboard.scale);
		show_checkbox("Use Camera Right", billboard.use_camera_right);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%s", "Color");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::ColorPicker4("##Color", &billboard.color[0]);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Texture");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);

		std::string texture_name = resource_manager.get_texture_name(billboard.texture);
		if (texture_name.empty()) {
			ImGui::Text("Texture: None");
		} else {
			ImGui::Text("%s", texture_name.c_str());
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_TEXTURE_PATH")) {
				std::string payload_n = *(std::string *)payload->Data;
				resource_manager.load_texture(payload_n.c_str());
				billboard.texture = resource_manager.get_texture_handle(payload_n);
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::EndTable();
	}
}

void Inspector::show_fmod_emitter() {
	auto &emitter = world->get_component<FMODEmitter>(selected_entity);

	if (ImGui::CollapsingHeader("Fmod Emitter", tree_flags)) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup("FmodEmitterContextMenu");
		}
		if (ImGui::BeginPopup("FmodEmitterContextMenu")) {
			if (ImGui::MenuItem("Reset Fmod Emitter")) {
				emitter.event_path = "";
			}
			remove_component_menu_item<FMODEmitter>();

			ImGui::EndPopup();
		}
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Fmod Emitter Component", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Event Name");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		static char event_name[256];
		ImGui::InputText("##EventName", event_name, 256);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Event Status: ");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (AudioManager::get().is_valid_event_path(event_name)) {
			emitter.event_path = event_name;
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
			ImGui::Text("Ok");
		} else {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
			ImGui::Text("Event Not Found in Banks");
		}
		ImGui::PopStyleColor();

		show_checkbox("Is 3D", emitter.is_3d);

		ImGui::EndTable();
	}
}

void Inspector::show_taggable() {
	auto &taggable = world->get_component<Taggable>(selected_entity);

	if (ImGui::CollapsingHeader("Taggable", tree_flags)) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup("TaggableContextMenu");
		}
		if (ImGui::BeginPopup("TaggableContextMenu")) {
			remove_component_menu_item<Taggable>();
			ImGui::EndPopup();
		}
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Position", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_vec3("Position", taggable.tag_position);

		ImGui::EndTable();
	}
}

void Inspector::show_highlight() {
	auto &highlighted = world->get_component<Highlight>(selected_entity);

	if (ImGui::CollapsingHeader("Highlighted", tree_flags)) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup("HighlightedContextMenu");
		}
		if (ImGui::BeginPopup("HighlightedContextMenu")) {
			remove_component_menu_item<Highlight>();
			ImGui::EndPopup();
		}

		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Highlight", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Color");
		ImGui::TableSetColumnIndex(1);
		ImGui::ColorEdit3("##Color", &highlighted.highlight_color[0]);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Highlight Type");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Highlight Type", magic_enum::enum_name(highlighted.target).data())) {
			for (auto type : magic_enum::enum_values<HighlightTarget>()) {
				bool is_selected = (highlighted.target == type);
				if (ImGui::Selectable(magic_enum::enum_name(type).data(), is_selected)) {
					highlighted.target = type;
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

void Inspector::show_particle_emitter() {
	auto &ps = world->get_component<ParticleEmitter>(selected_entity);
	if (ImGui::CollapsingHeader("Particle Emitter", tree_flags)) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup("ParticleEmitterContextMenu");
		}
		if (ImGui::BeginPopup("ParticleEmitterContextMenu")) {
			remove_component_menu_item<ParticleEmitter>();
			ImGui::EndPopup();
		}
		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Particle System", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		show_vec3("Position", ps.position);
		show_vec3("Position Variation", ps.position_variance);
		show_vec3("Start Velocity", ps.velocity_begin);
		show_vec3("End Velocity", ps.velocity_end);
		show_vec3("Velocity Variance", ps.velocity_variance);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Velocity Transition");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Velocity Transition", magic_enum::enum_name(ps.velocity_transition).data())) {
			for (auto transition : magic_enum::enum_values<TransitionType>()) {
				bool is_selected = (ps.velocity_transition == transition);
				if (ImGui::Selectable(magic_enum::enum_name(transition).data(), is_selected)) {
					ps.velocity_transition = transition;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Start Color");
		ImGui::TableSetColumnIndex(1);
		ImGui::ColorEdit4("##Start Color", &ps.color_begin.x);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("End Color");
		ImGui::TableSetColumnIndex(1);
		ImGui::ColorEdit4("##End Color", &ps.color_end.x);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Color Transition");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Color Transition", magic_enum::enum_name(ps.color_transition).data())) {
			for (auto transition : magic_enum::enum_values<TransitionType>()) {
				bool is_selected = (ps.color_transition == transition);
				if (ImGui::Selectable(magic_enum::enum_name(transition).data(), is_selected)) {
					ps.color_transition = transition;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		show_float("Start Scale", ps.size_begin);
		show_float("End Scale", ps.size_end);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Scale Transition");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Scale Transition", magic_enum::enum_name(ps.size_transition).data())) {
			for (auto transition : magic_enum::enum_values<TransitionType>()) {
				bool is_selected = (ps.size_transition == transition);
				if (ImGui::Selectable(magic_enum::enum_name(transition).data(), is_selected)) {
					ps.size_transition = transition;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		show_float("Start Rotation", ps.rotation_begin);
		show_float("End Rotation", ps.rotation_end);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Rotation Transition");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Rotation Transition", magic_enum::enum_name(ps.rotation_transition).data())) {
			for (auto transition : magic_enum::enum_values<TransitionType>()) {
				bool is_selected = (ps.rotation_transition == transition);
				if (ImGui::Selectable(magic_enum::enum_name(transition).data(), is_selected)) {
					ps.rotation_transition = transition;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		show_float("Particles Spawn Rate", ps.rate);
		ps.rate = glm::max(0.0f, ps.rate);
		show_float("Particle Lifetime", ps.lifetime);
		ps.lifetime = glm::max(0.0f, ps.lifetime);
		show_checkbox("Is One Shot", ps.is_one_shot);
		if (ps.is_one_shot) {
			// display a button that triggers a oneshot
			if (ImGui::Button("Trigger One Shot")) {
				ps.trigger_oneshot();
			}
			show_float("One Shot Duration", ps.one_shot_duration);
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Texture");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ps.is_textured) {
			ImGui::Text("Texture: %s", resource_manager.get_texture_name(ps.texture).c_str());
		} else {
			ImGui::Text("Texture: None");
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_TEXTURE_PATH")) {
				const std::string payload_n = *(const std::string *)payload->Data;
				resource_manager.load_texture(payload_n.c_str());
				ps.texture = resource_manager.get_texture_handle(payload_n);
				ps.is_textured = true;
			}

			ImGui::EndDragDropTarget();
		}

		show_checkbox("Is Billboard", ps.is_billboard);

		ImGui::EndTable();
	}
}

void Inspector::show_detection_camera() {
	auto &data = world->get_component<DetectionCamera>(selected_entity);
	if (ImGui::CollapsingHeader("DetectionCamera", tree_flags)) {
		remove_component_popup<DetectionCamera>();

		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Particles Parent", 2);
		ImGui::TableSetupColumn("##Col1", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Particle Parent");
		ImGui::TableSetColumnIndex(1);
		ImGui::InputInt("", (int *)&data.particles_parent, 0, 0);

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				Entity payload_entity = *(Entity *)payload->Data;
				data.particles_parent = payload_entity;
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::EndTable();
	}
}

void Inspector::show_decal() {
	auto &decal = world->get_component<Decal>(selected_entity);
	auto &textures = resource_manager.get_textures();
	if (ImGui::CollapsingHeader("Decal", tree_flags)) {
		remove_component_popup<Decal>();
		auto &selected_albedo = resource_manager.get_texture(decal.albedo);
		std::string albedo_name = selected_albedo.name;
		auto &selected_normal = resource_manager.get_texture(decal.normal);
		std::string normal_name = selected_normal.name;
		std::size_t last_slash_pos = albedo_name.find_last_of("/\\");

		if (last_slash_pos != std::string::npos) {
			albedo_name = albedo_name.substr(last_slash_pos + 1);
			std::size_t dot_pos = albedo_name.find_last_of('.');
			if (dot_pos != std::string::npos) {
				albedo_name = albedo_name.substr(0, dot_pos);
			}
		}
		last_slash_pos = normal_name.find_last_of("/\\");
		if (last_slash_pos != std::string::npos) {
			normal_name = normal_name.substr(last_slash_pos + 1);
			std::size_t dot_pos = normal_name.find_last_of('.');
			if (dot_pos != std::string::npos) {
				normal_name = normal_name.substr(0, dot_pos);
			}
		}

		float available_width = ImGui::GetContentRegionAvail().x;
		ImGui::BeginTable("Decal", 2);
		ImGui::TableSetupColumn("##Decal", ImGuiTableColumnFlags_WidthFixed, available_width * 0.33f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Albedo");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Albedo", albedo_name.c_str())) {
			for (const auto &texture : textures) {
				bool is_selected = (decal.albedo.id == resource_manager.get_texture_handle(texture.name).id);

				auto texture_handle = resource_manager.get_texture_handle(texture.name);
				std::string texture_name = texture.name;
				std::size_t texture_slash_pos = texture_name.find_last_of("/\\");
				if (texture_slash_pos != std::string::npos) {
					texture_name = texture_name.substr(texture_slash_pos + 1);
					std::size_t texture_dot_pos = texture_name.find_last_of('.');
					if (texture_dot_pos != std::string::npos) {
						texture_name = texture_name.substr(0, texture_dot_pos);
					}
				}

				if (ImGui::Selectable(texture_name.c_str(), is_selected)) {
					decal.albedo = texture_handle;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_TEXTURE_PATH")) {
				std::string payload_n = *(const std::string *)payload->Data;
				std::string search_string = "\\";
				std::string replace_string = "/";

				size_t pos = payload_n.find(search_string);
				while (pos != std::string::npos) {
					payload_n.replace(pos, search_string.length(), replace_string);
					pos = payload_n.find(search_string, pos + replace_string.length());
				}
				resource_manager.load_texture(payload_n.c_str());
				auto texture_handle = resource_manager.get_texture_handle(payload_n);
				decal.albedo = texture_handle;
			}

			ImGui::EndDragDropTarget();
		}
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Normal");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo("##Normal", normal_name.c_str())) {
			for (const auto &texture : textures) {
				bool is_selected = (decal.normal.id == resource_manager.get_texture_handle(texture.name).id);

				auto texture_handle = resource_manager.get_texture_handle(texture.name);
				std::string texture_name = texture.name;
				std::size_t texture_slash_pos = texture_name.find_last_of("/\\");
				if (texture_slash_pos != std::string::npos) {
					texture_name = texture_name.substr(texture_slash_pos + 1);
					std::size_t texture_dot_pos = texture_name.find_last_of('.');
					if (texture_dot_pos != std::string::npos) {
						texture_name = texture_name.substr(0, texture_dot_pos);
					}
				}

				if (ImGui::Selectable(texture_name.c_str(), is_selected)) {
					decal.normal = texture_handle;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_TEXTURE_PATH")) {
				std::string payload_n = *(const std::string *)payload->Data;
				std::string search_string = "\\";
				std::string replace_string = "/";

				size_t pos = payload_n.find(search_string);
				while (pos != std::string::npos) {
					payload_n.replace(pos, search_string.length(), replace_string);
					pos = payload_n.find(search_string, pos + replace_string.length());
				}
				resource_manager.load_texture(payload_n.c_str());
				auto texture_handle = resource_manager.get_texture_handle(payload_n);
				decal.normal = texture_handle;
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Color");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::ColorPicker4("##Color", (float *)&decal.color);
		ImGui::EndTable();
	}
}

bool Inspector::show_vec2(
		const char *label, glm::vec2 &vec2, float speed, float reset_value, float min_value, float max_value) {
	bool changed = false;
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::Text("%s", label);
	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
		vec2 = glm::vec3(reset_value);
		changed = true;
	}
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	changed |= ImGui::DragFloat2(fmt::format("##{}", label).c_str(), &vec2.x, speed, min_value, max_value);
	return changed;
}

bool Inspector::show_vec3(
		const char *label, glm::vec3 &vec3, float speed, float reset_value, float min_value, float max_value) {
	bool changed = false;
	bool open_context_menu = false;

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::Text("%s", label);
	if (ImGui::IsItemHovered()) {
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			vec3 = glm::vec3(reset_value);
			changed = true;
		} else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			open_context_menu = true;
		}
	}
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	changed |= ImGui::DragFloat3(fmt::format("##{}", label).c_str(), &vec3.x, speed, min_value, max_value);

	if (open_context_menu) {
		ImGui::OpenPopup(label);
	}

	if (ImGui::IsPopupOpen(label)) {
		ImGui::SetNextWindowSize(ImVec2(200, 0));
		if (ImGui::BeginPopup(label)) {
			if (ImGui::MenuItem("Copy")) {
				Inspector::copied_vector3 = vec3;
			}
			if (ImGui::MenuItem("Paste")) {
				vec3 = Inspector::copied_vector3;
				changed = true;
			}

			ImGui::EndPopup();
		}
	}

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

void Inspector::show_vector_vec3(const char *label, std::vector<glm::vec3> &vec3) {
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::Text("%s", label);
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::Text("%zu", vec3.size());
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
			SHOW_ADD_COMPONENT(Parent);
			SHOW_ADD_COMPONENT(Children);
			SHOW_ADD_COMPONENT(ModelInstance);
			SHOW_ADD_COMPONENT(SkinnedModelInstance);
			SHOW_ADD_COMPONENT(AnimationInstance);
			SHOW_ADD_COMPONENT(Attachment);
			SHOW_ADD_COMPONENT(FmodListener);
			SHOW_ADD_COMPONENT(Camera);
			SHOW_ADD_COMPONENT(ColliderTag);
			SHOW_ADD_COMPONENT(StaticTag);
			SHOW_ADD_COMPONENT(ColliderSphere);
			SHOW_ADD_COMPONENT(ColliderAABB);
			SHOW_ADD_COMPONENT(ColliderOBB);
			SHOW_ADD_COMPONENT(ColliderCapsule);
			SHOW_ADD_COMPONENT(Light);
			SHOW_ADD_COMPONENT(AgentData);
			SHOW_ADD_COMPONENT(HackerData);
			SHOW_ADD_COMPONENT(EnemyPath);
			SHOW_ADD_COMPONENT(Interactable);
			SHOW_ADD_COMPONENT(Platform);
			SHOW_ADD_COMPONENT(ExplodingBox);
			SHOW_ADD_COMPONENT(EnemyData);
			SHOW_ADD_COMPONENT(Billboard);
			SHOW_ADD_COMPONENT(PathNode);
			SHOW_ADD_COMPONENT(PathParent);
			SHOW_ADD_COMPONENT(Taggable);
			SHOW_ADD_COMPONENT(FMODEmitter);
			SHOW_ADD_COMPONENT(Highlight);
			SHOW_ADD_COMPONENT(ParticleEmitter);
			SHOW_ADD_COMPONENT(DetectionCamera);
			SHOW_ADD_COMPONENT(Decal);

			ImGui::EndPopup();
		}
	}
}

void Inspector::set_active_entity(Entity entity) {
	selected_entity = entity;
}
