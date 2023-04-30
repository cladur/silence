#include "ecs/ecs_manager.h"
#include "editor.h"
#include "inspector_gui.h"

void Editor::imgui_menu_bar() {
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("Save")) {
			SPDLOG_INFO("Saving scene...");
		}
		if (ImGui::MenuItem("Save as...")) {
			SPDLOG_INFO("Saving scene as...");
		}
		if (ImGui::MenuItem("Load")) {
			SPDLOG_INFO("Loading scene...");
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
		if (ImGui::MenuItem("Exit")) {
			should_run = false;
		}
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

void Editor::imgui_inspector() {
	ECSManager &ecs_manager = ECSManager::get();
	RenderManager &render_manager = RenderManager::get();
	Inspector inspector = Inspector();

	ImGui::Begin("Inspector");

	for (auto &entity : scenes[0].entities) {
		if (ImGui::Selectable(fmt::format("Entity {}", entity).c_str())) {
			entities_selected.push_back(entity);
		}
	}

	if (!entities_selected.empty()) {
		Entity active_entity = entities_selected.back();
		auto &transform = ecs_manager.get_component<Transform>(active_entity);
		inspector.show_components(active_entity);
	} else {
		ImGui::Text("No entity selected");
	}

	ImGui::End();
}

void Editor::imgui_scene(Scene &scene) {
	ImGui::Begin("Scene");
	ECSManager &ecs_manager = ECSManager::get();
	InputManager &input_manager = InputManager::get();

	for (auto &entity : scene.entities) {
		std::string entity_name = std::to_string(entity);
		if (ecs_manager.has_component<Name>(entity)) {
			entity_name = ecs_manager.get_component<Name>(entity).name;
		}

		static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
				ImGuiTreeNodeFlags_SpanAvailWidth;

		ImGuiTreeNodeFlags node_flags = base_flags;

		const bool is_selected =
				(std::find(entities_selected.begin(), entities_selected.end(), entity) != entities_selected.end());

		if (is_selected) {
			node_flags |= ImGuiTreeNodeFlags_Selected;
		}

		bool node_open = ImGui::TreeNodeEx(entity_name.c_str(), node_flags);

		if (ImGui::IsItemClicked()) {
			if (input_manager.is_action_pressed("select_multiple")) {
				if (is_selected) {
					entities_selected.erase(std::remove(entities_selected.begin(), entities_selected.end(), entity),
							entities_selected.end());
				} else {
					entities_selected.push_back(entity);
				}
			} else if (input_manager.is_action_pressed("select_rows") && last_entity_selected != 0) {
				// Select all entities between last_entity_selected and entity
				entities_selected.clear();
				uint32_t min = std::min(last_entity_selected, entity);
				uint32_t max = std::max(last_entity_selected, entity);
				for (uint32_t i = min; i <= max; i++) {
					entities_selected.push_back(i);
				}
			} else {
				entities_selected.clear();
				entities_selected.push_back(entity);
			}
			last_entity_selected = entity;
		}

		if (node_open) {
			ImGui::Text("TODO: Show entity children?");

			ImGui::TreePop();
		}
	}

	ImGui::End();
}
void Editor::imgui_viewport(Scene &scene) {
	RenderManager &render_manager = RenderManager::get();
	ECSManager &ecs_manager = ECSManager::get();

	ImGui::Begin("Viewport");

	viewport_hovered = ImGui::IsWindowHovered();

	if (ImGui::Button("Select")) {
	}
	ImGui::SameLine();
	if (ImGui::Button("Move")) {
		current_gizmo_operation = ImGuizmo::TRANSLATE;
	}
	ImGui::SameLine();
	if (ImGui::Button("Rotate")) {
		current_gizmo_operation = ImGuizmo::ROTATE;
	}
	ImGui::SameLine();
	if (ImGui::Button("Scale")) {
		current_gizmo_operation = ImGuizmo::SCALE;
	}
	ImGui::SameLine();
	if (current_gizmo_mode == ImGuizmo::WORLD) {
		if (ImGui::Button("World")) {
			current_gizmo_mode = ImGuizmo::LOCAL;
		}
	} else {
		if (ImGui::Button("Local")) {
			current_gizmo_mode = ImGuizmo::WORLD;
		}
	}

	// Get viewport size
	static ImVec2 last_viewport_size = ImVec2(0, 0);
	ImVec2 viewport_size = ImGui::GetContentRegionAvail();
	if (viewport_size.x != last_viewport_size.x || viewport_size.y != last_viewport_size.y) {
		// Resize the framebuffer
		scene.render_scene->resize_framebuffer(viewport_size.x, viewport_size.y);
		last_viewport_size = viewport_size;
	}

	uint32_t render_image = scene.render_scene->render_framebuffer.get_texture_id();
	ImGui::Image((void *)(intptr_t)render_image, viewport_size, ImVec2(0, 1), ImVec2(1, 0));

	// Draw gizmo
	ImGuiIO &io = ImGui::GetIO();
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, viewport_size.x, viewport_size.y);
	ImGuizmo::SetDrawlist();

	glm::mat4 *view = &scene.render_scene->view;
	glm::mat4 *projection = &scene.render_scene->projection;

	if (!entities_selected.empty()) {
		// If we have entities selected, we want to take their average position and use that as the pivot point
		// for the gizmo

		glm::mat4 temp_matrix = glm::mat4(1.0f);

		if (entities_selected.size() > 1) {
			auto average_position = glm::vec3(0.0f);
			for (auto &entity : entities_selected) {
				if (ecs_manager.has_component<Transform>(entity)) {
					auto &transform = ecs_manager.get_component<Transform>(entity);
					average_position += transform.get_position();
				}
			}

			temp_matrix = glm::translate(glm::mat4(1.0f), average_position / (float)entities_selected.size());
		} else {
			auto &transform = ecs_manager.get_component<Transform>(entities_selected[0]);
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