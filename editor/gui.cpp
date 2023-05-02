#include "ImGuizmo.h"
#include "ecs/ecs_manager.h"
#include "editor.h"
#include "inspector_gui.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <glm/gtx/matrix_decompose.hpp>

#include "IconsMaterialDesign.h"

void Editor::imgui_menu_bar() {
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem(ICON_MD_SAVE " Save")) {
			SPDLOG_INFO("Saving scene...");
		}
		if (ImGui::MenuItem(ICON_MD_SAVE " Save as...")) {
			SPDLOG_INFO("Saving scene as...");
		}
		if (ImGui::MenuItem(ICON_MD_FOLDER_OPEN " Load")) {
			SPDLOG_INFO(ICON_MD_CLOSE "Loading scene...");
			nfdchar_t *out_path;
			nfdfilteritem_t filter_item[2] = { { "Source code", "c,cpp,cc" }, { "Headers", "h,hpp" } };
			nfdresult_t result = NFD_OpenDialog(&out_path, filter_item, 2, nullptr);
			if (result == NFD_OKAY) {
				puts("Success!");
				puts(out_path);
				NFD_FreePath(out_path);
			} else if (result == NFD_CANCEL) {
				puts("User pressed cancel.");
			} else {
				printf("Error: %s\n", NFD_GetError());
			}
		}
		if (ImGui::MenuItem(ICON_MD_CLOSE " Exit")) {
			should_run = false;
		}
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

void Editor::imgui_inspector(Scene &scene) {
	ECSManager &ecs_manager = ECSManager::get();
	RenderManager &render_manager = RenderManager::get();
	Inspector &inspector = Inspector::get();

	ImGui::Begin("Inspector");

	if (scene.last_entity_selected > 0) {
		Entity active_entity = scene.last_entity_selected;
		if (ecs_manager.has_component<Name>(active_entity)) {
			auto &name = ecs_manager.get_component<Name>(active_entity);
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputText("##Name", &name.name);
		} else {
			ImGui::Text("<Unnamed Entity %d>", active_entity);
		}
		ImGui::Spacing();
		ImGui::Spacing();
		inspector.set_active_entity(active_entity);
		inspector.show_components();
		inspector.show_add_component();
	} else {
		ImGui::Text("No entity selected");
	}

	ImGui::End();
}

void Editor::imgui_scene(Scene &scene) {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

	ImGui::Begin("Scene");
	ECSManager &ecs_manager = ECSManager::get();
	InputManager &input_manager = InputManager::get();

	ImVec2 cursor_pos = ImGui::GetCursorPos();
	cursor_pos.x += 4;
	cursor_pos.y += 4;
	ImGui::SetCursorPos(cursor_pos);

	// ADD ENTITY BUTTON
	if (ImGui::Button(ICON_MD_ADD, ImVec2(20, 20))) {
		ImGui::OpenPopup("Add Entity");
	}

	if (ImGui::BeginPopup("Add Entity")) {
		ImGui::SeparatorText("Basic");
		if (ImGui::MenuItem("Empty")) {
			Entity new_entity = ecs_manager.create_entity();
			ecs_manager.add_component(new_entity, Transform{});
			scene.entities.push_back(new_entity);
			scene.last_entity_selected = new_entity;
		}
		ImGui::SeparatorText("Prefabs");

		ImGui::EndPopup();
	}

	// --------------------

	ImGui::SameLine();
	float avail_x = ImGui::GetContentRegionAvail().x;
	static ImGuiTextFilter filter;
	filter.Draw("##", avail_x - 5);

	ImGui::Separator();

	static ImGuiTableFlags flags = ImGuiTableFlags_NoBordersInBody;

	if (ImGui::BeginTable("Scene", 2, flags)) {
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
		ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, 15.0f);
		// ImGui::TableHeadersRow();

		for (auto &entity : scene.entities) {
			std::string name = std::to_string(entity);
			if (ecs_manager.has_component<Name>(entity)) {
				name = ecs_manager.get_component<Name>(entity).name;
			}

			if (!filter.PassFilter(name.c_str())) {
				continue;
			}

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
					ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth;

			ImGuiTreeNodeFlags node_flags = base_flags;

			bool is_selected = (std::find(scene.entities_selected.begin(), scene.entities_selected.end(), entity) !=
					scene.entities_selected.end());

			if (is_selected) {
				node_flags |= ImGuiTreeNodeFlags_Selected;
			}

			bool is_leaf = true;
			if (is_leaf) {
				node_flags |= ImGuiTreeNodeFlags_Leaf;
			}

			bool window_focused = ImGui::IsWindowFocused();

			if (window_focused) {
				ImVec4 blue = ImVec4(43.0f / 255.0f, 93.0f / 255.0f, 134.0f / 255.0f, 1.0f);
				if (is_selected) {
					ImGui::PushStyleColor(ImGuiCol_HeaderHovered, blue);
				}
				ImGui::PushStyleColor(ImGuiCol_Header, blue);
			} else {
				if (is_selected) {
					ImGui::PushStyleColor(
							ImGuiCol_Header, ImVec4(77.0f / 255.0f, 77.0f / 255.0f, 77.0f / 255.0f, 1.0f));
				}
			}

			// TODO: Add icons for different types of entities
			// Or, a seperate column that displays all components of the entity (sort of like the visibility icon)
			bool node_open = ImGui::TreeNodeEx(name.c_str(), node_flags);

			if (window_focused) {
				if (is_selected) {
					ImGui::PopStyleColor();
				}
				ImGui::PopStyleColor();
			} else {
				if (is_selected) {
					ImGui::PopStyleColor();
				}
			}

			if (ImGui::IsItemClicked()) {
				if (input_manager.is_action_pressed("select_multiple")) {
					if (is_selected) {
						scene.clear_selection();
					} else {
						scene.add_to_selection(entity);
					}
				} else if (input_manager.is_action_pressed("select_rows") && scene.last_entity_selected != 0) {
					// Select all entities between last_entity_selected and entity
					scene.clear_selection();
					uint32_t min = std::min(scene.last_entity_selected, entity);
					uint32_t max = std::max(scene.last_entity_selected, entity);
					for (uint32_t i = min; i <= max; i++) {
						scene.add_to_selection(i);
					}
				} else {
					scene.clear_selection();
					scene.add_to_selection(entity);
				}
				scene.calculate_multi_select_parent();
				scene.execute_reparent_queue();
			}

			ImGui::TableNextColumn();
			static bool visible = true;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

			if (visible) {
				if (ImGui::Button(ICON_MD_VISIBILITY, ImVec2(15, 15))) {
					visible = false;
				}
			} else {
				if (ImGui::Button(ICON_MD_VISIBILITY_OFF, ImVec2(15, 15))) {
					visible = true;
				}
			}

			if (node_open) {
				ImGui::TreePop();
			}

			ImGui::PopStyleVar();
			ImGui::PopStyleColor(4);
		}

		ImGui::EndTable();
	}

	ImGui::End();

	ImGui::PopStyleVar(2);
}
void Editor::imgui_viewport(Scene &scene, uint32_t scene_index) {
	RenderManager &render_manager = RenderManager::get();
	ECSManager &ecs_manager = ECSManager::get();

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

	ImVec2 viewport_start = ImGui::GetCursorPos();

	ImGui::Begin(scene.name.c_str());
	ImGuizmo::SetDrawlist();
	// We need to set unique ID to each viewport, otherwise modifying the transform of one viewport will affect all
	ImGuizmo::SetID((int)scene_index);
	auto viewport_min_region = ImGui::GetWindowContentRegionMin();
	auto viewport_max_region = ImGui::GetWindowContentRegionMax();
	auto viewport_offset = ImGui::GetWindowPos();
	glm::vec2 viewport_bounds[2];
	viewport_bounds[0] = { viewport_min_region.x + viewport_offset.x, viewport_min_region.y + viewport_offset.y };
	viewport_bounds[1] = { viewport_max_region.x + viewport_offset.x, viewport_max_region.y + viewport_offset.y };
	ImGuizmo::SetRect(viewport_bounds[0].x, viewport_bounds[0].y, viewport_bounds[1].x - viewport_bounds[0].x,
			viewport_bounds[1].y - viewport_bounds[0].y);

	scene.viewport_hovered = ImGui::IsWindowHovered();
	scene.is_visible = !ImGui::IsWindowCollapsed();

	// If ImGui window is focused
	if (ImGui::IsWindowFocused()) {
		uint32_t our_scene_idx = get_scene_index(scene.name);
		active_scene = our_scene_idx;
	}

	// Get viewport size
	ImVec2 viewport_size = ImGui::GetContentRegionAvail();
	if (viewport_size.x != scene.last_viewport_size.x || viewport_size.y != scene.last_viewport_size.y) {
		// Resize the framebuffer
		scene.get_render_scene().resize_framebuffer(viewport_size.x, viewport_size.y);
		scene.last_viewport_size = viewport_size;
	}

	uint32_t render_image = scene.get_render_scene().render_framebuffer.get_texture_id();
	ImGui::Image((void *)(intptr_t)render_image, viewport_size, ImVec2(0, 1), ImVec2(1, 0));

	// Draw gizmo
	ImGuiIO &io = ImGui::GetIO();

	glm::mat4 *view = &scene.get_render_scene().view;
	glm::mat4 *projection = &scene.get_render_scene().projection;

	if (!scene.entities_selected.empty()) {
		float *snap = nullptr;
		static float snap_values[3] = { 0.0f, 0.0f, 0.0f };

		if (io.KeyCtrl || use_snapping) {
			if (current_gizmo_operation == ImGuizmo::ROTATE) {
				snap_values[0] = snap_values[1] = snap_values[2] = rotation_snap;
			} else if (current_gizmo_operation == ImGuizmo::TRANSLATE) {
				snap_values[0] = snap_values[1] = snap_values[2] = translation_snap;
			} else if (current_gizmo_operation == ImGuizmo::SCALE) {
				snap_values[0] = snap_values[1] = snap_values[2] = scale_snap * 0.01f;
			}
			snap = snap_values;
		}

		auto &multi_transform = ecs_manager.get_component<Transform>(scene.multi_select_parent);
		auto &transform = ecs_manager.get_component<Transform>(scene.entities_selected[0]);
		glm::mat4 temp_matrix = glm::mat4(1.0f);

		if (scene.entities_selected.size() == 1) {
			temp_matrix = transform.get_global_model_matrix();
		} else {
			temp_matrix = multi_transform.get_global_model_matrix();
		}

		if (ImGuizmo::Manipulate(glm::value_ptr(*view), glm::value_ptr(*projection), current_gizmo_operation,
					current_gizmo_mode, glm::value_ptr(temp_matrix), nullptr, snap)) {
			glm::vec3 skew;
			glm::vec4 perspective;

			if (scene.entities_selected.size() == 1) {
				glm::mat4 parent_matrix = glm::mat4(1.0f);
				if (ecs_manager.has_component<Parent>(scene.entities_selected[0])) {
					auto &parent = ecs_manager.get_component<Parent>(scene.entities_selected[0]);
					auto &parent_transform = ecs_manager.get_component<Transform>(parent.parent);
					parent_matrix = glm::inverse(parent_transform.get_global_model_matrix());
				}
				glm::decompose(parent_matrix * temp_matrix, transform.scale, transform.orientation, transform.position,
						skew, perspective);
				transform.set_changed(true);
			} else {
				glm::decompose(temp_matrix, multi_transform.scale, multi_transform.orientation,
						multi_transform.position, skew, perspective);
				multi_transform.set_changed(true);
			}
			// glm::decompose(temp_matrix, multi_transform.scale, multi_transform.orientation, multi_transform.position,
			// 		skew, perspective);
			// multi_transform.set_changed(true);
		}
	}

	viewport_start.x += 4;
	viewport_start.y += 2;
	ImGui::SetCursorPos(viewport_start);

	ImVec4 active_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

	if (ImGui::Button(ICON_MD_ADS_CLICK)) {
	}
	ImGui::SameLine();

	int push_count = 0;
	if (current_gizmo_operation == ImGuizmo::TRANSLATE) {
		ImGui::PushStyleColor(ImGuiCol_Button, active_color);
		push_count++;
	}
	if (ImGui::Button(ICON_MD_OPEN_WITH)) {
		current_gizmo_operation = ImGuizmo::TRANSLATE;
	}
	ImGui::PopStyleColor(push_count);
	push_count = 0;
	ImGui::SameLine();

	if (current_gizmo_operation == ImGuizmo::ROTATE) {
		ImGui::PushStyleColor(ImGuiCol_Button, active_color);
		push_count++;
	}
	if (ImGui::Button(ICON_MD_ROTATE_LEFT)) {
		current_gizmo_operation = ImGuizmo::ROTATE;
	}
	ImGui::PopStyleColor(push_count);
	push_count = 0;
	ImGui::SameLine();

	if (current_gizmo_operation == ImGuizmo::SCALE) {
		ImGui::PushStyleColor(ImGuiCol_Button, active_color);
		push_count++;
	}
	if (ImGui::Button(ICON_MD_OPEN_IN_FULL)) {
		current_gizmo_operation = ImGuizmo::SCALE;
	}
	ImGui::PopStyleColor(push_count);
	push_count = 0;
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.345f, 0.345f, 0.345f, 1.0f));
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	ImGui::PopStyleColor();
	ImGui::SameLine();

	if (current_gizmo_mode == ImGuizmo::WORLD) {
		if (ImGui::Button(ICON_MD_PUBLIC " World")) {
			current_gizmo_mode = ImGuizmo::LOCAL;
		}
	} else {
		if (ImGui::Button(ICON_MD_HOME " Local")) {
			current_gizmo_mode = ImGuizmo::WORLD;
		}
	}
	ImGui::SameLine();

	// SNAPPING
	if (use_snapping) {
		ImGui::PushStyleColor(ImGuiCol_Button, active_color);
		push_count++;
	}
	if (ImGui::Button(ICON_MD_STRAIGHTEN)) {
		use_snapping = !use_snapping;
	}
	ImGui::PopStyleColor(push_count);
	push_count = 0;
	ImGui::SameLine();

	if (ImGui::Button(ICON_MD_TUNE)) {
		ImGui::OpenPopup("Gizmo Settings");
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("Gizmo Settings")) {
		ImGui::InputFloat("Translation", &translation_snap, 0.1f);
		ImGui::InputFloat("Rotation", &rotation_snap, 0.1f);
		ImGui::InputFloat("Scale (%)", &scale_snap, 0.1f);
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	ImGui::End();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void Editor::imgui_resources() {
	ImGui::Begin("Resources");

	ImGui::Text("TODO: List resources");

	ImGui::End();
}

void Editor::imgui_settings() {
	ImGui::Begin("Settings");

	ImGui::Checkbox("Show demo window", &show_demo_window);
	ImGui::Checkbox("Show CVAR editor", &show_cvar_editor);

	if (show_demo_window) {
		ImGui::ShowDemoWindow();
	}

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
			ImGui::GetIO().Framerate);

	ImGui::End();
}