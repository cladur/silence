#include "ImGuizmo.h"
#include "ecs/world.h"
#include "editor.h"
#include "input/input_manager.h"
#include "inspector_gui.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <filesystem>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "IconsMaterialDesign.h"
#include "nfd.h"

void Editor::imgui_menu_bar() {
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem(ICON_MD_NEW_LABEL " New")) {
			SPDLOG_INFO("Create scene...");
			std::string name = fmt::format("New Scene {}", scenes.size());
			create_scene(name);
		}
		if (ImGui::MenuItem(ICON_MD_SAVE " Save")) {
			SPDLOG_INFO("Saving scene...");
			if (get_active_scene().path.empty()) {
				nfdchar_t *out_path;
				nfdfilteritem_t filter_item[1] = { { "Scene", "scn" } };
				nfdresult_t result = NFD_SaveDialog(&out_path, filter_item, 1, nullptr, nullptr);
				if (result == NFD_OKAY) {
					auto filename = std::filesystem::path(out_path).filename().string();
					get_active_scene().name = filename;
					get_active_scene().save_to_file(out_path);
					NFD_FreePath(out_path);
				} else if (result == NFD_CANCEL) {
					puts("User pressed cancel.");
				} else {
					printf("Error: %s\n", NFD_GetError());
				}
			} else {
				get_active_scene().save_to_file("");
			}
		}
		if (ImGui::MenuItem(ICON_MD_SAVE " Save as...")) {
			SPDLOG_INFO("Saving scene as...");
			nfdchar_t *out_path;
			nfdfilteritem_t filter_item[1] = { { "Scene", "scn" } };
			nfdresult_t result = NFD_SaveDialog(&out_path, filter_item, 1, nullptr, nullptr);
			if (result == NFD_OKAY) {
				auto filename = std::filesystem::path(out_path).filename().string();
				get_active_scene().name = filename;
				get_active_scene().save_to_file(out_path);
				NFD_FreePath(out_path);
			} else if (result == NFD_CANCEL) {
				puts("User pressed cancel.");
			} else {
				printf("Error: %s\n", NFD_GetError());
			}
		}
		if (ImGui::MenuItem(ICON_MD_FOLDER_OPEN " Load")) {
			SPDLOG_INFO(ICON_MD_CLOSE "Loading scene...");
			nfdchar_t *out_path;
			nfdfilteritem_t filter_item[1] = { { "Scene", "scn" } };
			nfdresult_t result = NFD_OpenDialog(&out_path, filter_item, 1, nullptr);
			if (result == NFD_OKAY) {
				auto filename = std::filesystem::path(out_path).filename().string();
				create_scene(filename);
				get_editor_scene(scenes.size() - 1).load_from_file(out_path);
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

void Editor::imgui_inspector(EditorScene &scene) {
	World &world = scene.world;
	RenderManager &render_manager = RenderManager::get();
	ImGui::Begin("Inspector");

	if (scene.last_entity_selected > 0) {
		Entity active_entity = scene.last_entity_selected;
		if (world.has_component<Name>(active_entity)) {
			auto &name = world.get_component<Name>(active_entity);
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

void Editor::imgui_scene(EditorScene &scene) {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

	ImGui::Begin("Scene");
	World &world = scene.world;
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
			Entity new_entity = world.create_entity();
			world.add_component(new_entity, Transform{});
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
			if (world.has_component<Name>(entity)) {
				name = world.get_component<Name>(entity).name;
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
void Editor::imgui_viewport(EditorScene &scene, uint32_t scene_index) {
	RenderManager &render_manager = RenderManager::get();

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

	ImVec2 viewport_start = ImGui::GetCursorPos();

	bool window_opened = true;
	ImGui::Begin(scene.name.c_str(), &window_opened);
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

		auto &multi_transform = scene.world.get_component<Transform>(scene.multi_select_parent);
		auto &transform = scene.world.get_component<Transform>(scene.entities_selected[0]);
		glm::mat4 temp_matrix = glm::mat4(1.0f);

		if (scene.entities_selected.size() == 1) {
			temp_matrix = transform.get_global_model_matrix();
		} else if (current_gizmo_operation == ImGuizmo::ROTATE && use_individual_origins) {
			temp_matrix = scene.dummy_transform.get_global_model_matrix();
		} else {
			temp_matrix = multi_transform.get_global_model_matrix();
		}

		static auto prev_scale = glm::vec3(1.0f);
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			prev_scale = glm::vec3(1.0f);
		}

		glm::mat4 delta_matrix = glm::mat4(1.0f);
		if (ImGuizmo::Manipulate(glm::value_ptr(*view), glm::value_ptr(*projection), current_gizmo_operation,
					current_gizmo_mode, glm::value_ptr(temp_matrix), glm::value_ptr(delta_matrix), snap)) {
			glm::vec3 skew;
			glm::vec4 perspective;

			if (scene.entities_selected.size() == 1) {
				glm::mat4 parent_matrix = glm::mat4(1.0f);
				if (scene.world.has_component<Parent>(scene.entities_selected[0])) {
					auto &parent = scene.world.get_component<Parent>(scene.entities_selected[0]);
					auto &parent_transform = scene.world.get_component<Transform>(parent.parent);
					parent_matrix = glm::inverse(parent_transform.get_global_model_matrix());
				}
				glm::decompose(parent_matrix * temp_matrix, transform.scale, transform.orientation, transform.position,
						skew, perspective);
				transform.set_changed(true);
			} else if (use_individual_origins) {
				glm::vec3 scale;
				glm::quat orientation;
				glm::vec3 position;

				glm::decompose(delta_matrix, scale, orientation, position, skew, perspective);
				glm::vec3 delta_scale = scale / prev_scale;
				prev_scale = scale;

				for (auto &entity : scene.entities_selected) {
					auto &t = scene.world.get_component<Transform>(entity);

					t.add_orientation(orientation);

					if (current_gizmo_operation == ImGuizmo::SCALE) {
						t.scale *= delta_scale;
					}
					t.set_changed(true);
				}

				glm::decompose(temp_matrix, scale, orientation, multi_transform.position, skew, perspective);
				multi_transform.set_changed(true);

				glm::decompose(temp_matrix, scene.dummy_transform.scale, scene.dummy_transform.orientation,
						scene.dummy_transform.position, skew, perspective);
				scene.dummy_transform.set_changed(true);
				scene.dummy_transform.update_global_model_matrix();
			} else {
				glm::decompose(temp_matrix, multi_transform.scale, multi_transform.orientation,
						multi_transform.position, skew, perspective);
				multi_transform.set_changed(true);
			}
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
	ImGui::SameLine();
	// INDIVIDUAL ORIGINS
	if (use_individual_origins) {
		ImGui::PushStyleColor(ImGuiCol_Button, active_color);
		push_count++;
	}
	if (ImGui::Button(ICON_MD_TRIP_ORIGIN)) {
		use_individual_origins = !use_individual_origins;
	}
	ImGui::PopStyleColor(push_count);
	push_count = 0;

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

	if (!window_opened) {
		scene_to_delete = scene_index;
		scene_deletion_queued = true;
	}
}

void Editor::display_folder(const std::string &path) {
	for (auto &entry : std::filesystem::directory_iterator(path)) {
		if (!entry.is_directory()) {
			continue;
		}

		static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
				ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;

		ImGuiTreeNodeFlags node_flags = base_flags;

		// Check if entry has folders inside it
		bool is_empty = true;
		for (auto &child : std::filesystem::directory_iterator(entry.path())) {
			if (child.is_directory()) {
				is_empty = false;
				break;
			}
		}
		if (is_empty) {
			node_flags |= ImGuiTreeNodeFlags_Leaf;
		} else {
			node_flags |= ImGuiTreeNodeFlags_OpenOnArrow;
		}

		if (content_browser_current_path == entry.path().string()) {
			node_flags |= ImGuiTreeNodeFlags_Selected;
		}

		bool node_open = ImGui::TreeNodeEx(entry.path().filename().string().c_str(), node_flags);

		if (ImGui::IsItemClicked()) {
			content_browser_current_path = entry.path().string();
		}

		if (node_open) {
			display_folder(entry.path().string());

			ImGui::TreePop();
		}
	}
}

void Editor::imgui_content_browser() {
	ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImGui::BeginTable("Content Browser", 2, ImGuiTableFlags_Resizable);

	ImGui::TableSetupColumn("Folders", ImGuiTableColumnFlags_WidthFixed, 120.0f);

	ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetContentRegionAvail().y);
	ImGui::TableSetColumnIndex(0);

	display_folder("resources");

	ImGui::TableNextColumn();

	float padding = 16.0f;
	float thumbnail_size = 48.0f;
	float cell_size = thumbnail_size + padding;

	float panel_width = ImGui::GetContentRegionAvail().x;
	int column_count = (int)(panel_width / cell_size);
	if (column_count < 1) {
		column_count = 1;
	}

	ImGui::BeginChild("##ScrollingRegion", ImVec2(0, 0), false);

	if (ImGui::Button(ICON_MD_ARROW_UPWARD)) {
		if (content_browser_current_path != "resources") {
			content_browser_current_path = std::filesystem::path(content_browser_current_path).parent_path().string();
		}
	}
	ImGui::SameLine();
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	ImGui::SameLine();
	ImGui::Text("%s", content_browser_current_path.c_str());

	ImGui::Spacing();

	ImGui::BeginTable("##Content Browser", column_count, ImGuiTableFlags_NoBordersInBody);

	ImGui::TableNextColumn();

	for (auto &entry : std::filesystem::directory_iterator(content_browser_current_path)) {
		ImGui::BeginGroup();

		std::string label = entry.path().filename().string();
		std::string extension = entry.path().extension().string();

		ImVec2 label_size = ImGui::CalcTextSize(label.c_str());

		ImVec2 thumbnail_size_vec2 = ImVec2(thumbnail_size, thumbnail_size);
		ImVec2 thumbnail_uv0 = ImVec2(0.0f, 0.0f);
		ImVec2 thumbnail_uv1 = ImVec2(1.0f, 1.0f);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

		uint32_t entry_texture = file_texture;
		if (entry.is_directory()) {
			entry_texture = folder_texture;
		}
		ImGui::ImageButton((ImTextureID)(uint64_t)entry_texture, thumbnail_size_vec2, thumbnail_uv0, thumbnail_uv1);

		ImGui::PopStyleColor(3);

		float text_scale = 0.75f;
		ImGui::SetWindowFontScale(text_scale);
		ImGuiStyle &style = ImGui::GetStyle();

		float size = ImGui::CalcTextSize(label.c_str()).x * text_scale + style.FramePadding.x * 2.0f;
		float avail = thumbnail_size;

		float off = (avail - size) * 0.5f;
		if (off > 0.0f) {
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
		}

		ImGui::Text("%s", label.c_str());
		ImGui::SetWindowFontScale(1.0f);

		ImGui::Spacing();

		ImGui::EndGroup();

		if (ImGui::IsItemClicked()) {
			if (entry.is_directory()) {
				content_browser_current_path = entry.path().string();
			} else {
				SPDLOG_INFO("Clicked on file {}", entry.path().string());
			}
		}

		ImGui::TableNextColumn();
	}

	ImGui::EndTable();

	ImGui::EndChild();

	ImGui::EndTable();

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