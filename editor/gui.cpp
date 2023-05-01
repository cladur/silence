#include "ImGuizmo.h"
#include "ecs/ecs_manager.h"
#include "editor.h"
#include "inspector_gui.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

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
	Inspector inspector = Inspector();

	ImGui::Begin("Inspector");

	if (scene.last_entity_selected > 0) {
		Entity active_entity = scene.last_entity_selected;
		if (ecs_manager.has_component<Name>(active_entity)) {
			auto &name = ecs_manager.get_component<Name>(active_entity);
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputText("##", &name.name);
		} else {
			ImGui::Text("<Unnamed Entity %d>", active_entity);
		}
		ImGui::Spacing();
		ImGui::Spacing();
		inspector.show_components(active_entity);
	} else {
		ImGui::Text("No entity selected");
	}

	ImGui::End();
}

void Editor::imgui_scene(Scene &scene) {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	// ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

	ImGui::Begin("Scene");
	ECSManager &ecs_manager = ECSManager::get();
	InputManager &input_manager = InputManager::get();

	static ImGuiTableFlags flags = ImGuiTableFlags_NoBordersInBody;

	if (ImGui::BeginTable("Scene", 2, flags)) {
		ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_None);
		ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, 15.0f);
		// ImGui::TableHeadersRow();

		for (auto &entity : scene.entities) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			std::string name = std::to_string(entity);
			if (ecs_manager.has_component<Name>(entity)) {
				name = ecs_manager.get_component<Name>(entity).name;
			}
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
						scene.entities_selected.erase(
								std::remove(scene.entities_selected.begin(), scene.entities_selected.end(), entity),
								scene.entities_selected.end());
					} else {
						scene.entities_selected.push_back(entity);
					}
				} else if (input_manager.is_action_pressed("select_rows") && scene.last_entity_selected != 0) {
					// Select all entities between last_entity_selected and entity
					scene.entities_selected.clear();
					uint32_t min = std::min(scene.last_entity_selected, entity);
					uint32_t max = std::max(scene.last_entity_selected, entity);
					for (uint32_t i = min; i <= max; i++) {
						scene.entities_selected.push_back(i);
					}
				} else {
					scene.entities_selected.clear();
					scene.entities_selected.push_back(entity);
				}
				scene.last_entity_selected = entity;
			}

			ImGui::TableNextColumn();
			static bool visible = true;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0, 0, 0, 0));

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

			ImGui::PopStyleColor(4);
		}

		ImGui::EndTable();
	}

	ImGui::End();

	ImGui::PopStyleVar(2);
}
void Editor::imgui_viewport(Scene &scene) {
	RenderManager &render_manager = RenderManager::get();
	ECSManager &ecs_manager = ECSManager::get();

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

	ImGui::Begin(scene.name.c_str());

	scene.viewport_hovered = ImGui::IsWindowHovered();
	scene.is_visible = !ImGui::IsWindowCollapsed();

	// If ImGui window is focused
	if (ImGui::IsWindowFocused()) {
		uint32_t our_scene_idx = get_scene_index(scene.name);
		active_scene = our_scene_idx;
	}

	ImVec2 cursor = ImGui::GetCursorPos();
	cursor.x += 4;
	cursor.y += 2;
	ImGui::SetCursorPos(cursor);
	// ImGui::SetWindowFontScale(1.4f);

	ImVec4 active_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

	ImRect clip_rect;
	clip_rect.Min.x = ImGui::GetWindowContentRegionMin().x - 50;
	clip_rect.Min.y = ImGui::GetWindowContentRegionMin().y - 50;
	clip_rect.Max.x = ImGui::GetWindowContentRegionMax().x + 50;
	clip_rect.Max.y = ImGui::GetWindowContentRegionMax().y + 50;
	ImGui::PushClipRect(clip_rect.Min, clip_rect.Max, false);

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

	ImGui::PopClipRect();

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
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, viewport_size.x, viewport_size.y);
	ImGuizmo::SetDrawlist();

	glm::mat4 *view = &scene.get_render_scene().view;
	glm::mat4 *projection = &scene.get_render_scene().projection;

	if (!scene.entities_selected.empty()) {
		// If we have entities selected, we want to take their average position and use that as the pivot point
		// for the gizmo

		glm::mat4 temp_matrix = glm::mat4(1.0f);

		if (scene.entities_selected.size() > 1) {
			auto average_position = glm::vec3(0.0f);
			for (auto &entity : scene.entities_selected) {
				if (ecs_manager.has_component<Transform>(entity)) {
					auto &transform = ecs_manager.get_component<Transform>(entity);
					average_position += transform.get_position();
				}
			}

			temp_matrix = glm::translate(glm::mat4(1.0f), average_position / (float)scene.entities_selected.size());
		} else {
			auto &transform = ecs_manager.get_component<Transform>(scene.entities_selected[0]);
			temp_matrix = transform.get_global_model_matrix();
		}

		if (ImGuizmo::Manipulate(glm::value_ptr(*view), glm::value_ptr(*projection), current_gizmo_operation,
					current_gizmo_mode, glm::value_ptr(temp_matrix), nullptr, nullptr)) {
			// Gizmo handles our final world transform, but we need to update our local transform (pos, orient,
			// scale) In order to do that, we extract the local transform from the world transform, by
			// multiplying by the inverse of the parent's world transform From there, we can decompose the local
			// transform into its components (pos, orient, scale)
			glm::vec3 skew;
			glm::vec4 perspective;
			// if (last_selected->parent) {
			// 	glm::decompose(glm::inverse(last_selected->parent->transform.modelMatrix) * tempMatrix,
			// 			last_selected->transform.scale, last_selected->transform.orient,
			// 			last_selected->transform.pos, skew, perspective);
			// } else {
			// 	glm::decompose(temp_matrix, last_selected->transform.scale, last_selected->transform.orient,
			// 			last_selected->transform.pos, skew, perspective);
			// }

			// last_selected->transform.isDirty = true;
		}
	}

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