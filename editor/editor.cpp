#include "editor.h"

#include "display/display_manager.h"
#include "editor/editor_scene.h"
#include "input/input_manager.h"
#include "physics/physics_manager.h"
#include "render/render_manager.h"

#include "ecs/world.h"

#include <imgui.h>
#include <imgui_internal.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "IconsMaterialDesign.h"
#include "inspector_gui.h"

void default_mappings() {
	InputManager &input_manager = InputManager::get();
	input_manager.add_action("move_forward");
	input_manager.add_key_to_action("move_forward", InputKey::W);
	input_manager.add_key_to_action("move_forward", InputKey::GAMEPAD_LEFT_STICK_Y_POSITIVE);
	input_manager.add_action("move_backward");
	input_manager.add_key_to_action("move_backward", InputKey::S);
	input_manager.add_key_to_action("move_backward", InputKey::GAMEPAD_LEFT_STICK_Y_NEGATIVE);
	input_manager.add_action("move_left");
	input_manager.add_key_to_action("move_left", InputKey::A);
	input_manager.add_key_to_action("move_left", InputKey::GAMEPAD_LEFT_STICK_X_NEGATIVE);
	input_manager.add_action("move_right");
	input_manager.add_key_to_action("move_right", InputKey::D);
	input_manager.add_key_to_action("move_right", InputKey::GAMEPAD_LEFT_STICK_X_POSITIVE);
	input_manager.add_action("move_up");
	input_manager.add_key_to_action("move_up", InputKey::E);
	input_manager.add_key_to_action("move_up", InputKey::GAMEPAD_BUTTON_A);
	input_manager.add_action("move_down");
	input_manager.add_key_to_action("move_down", InputKey::Q);
	input_manager.add_key_to_action("move_down", InputKey::GAMEPAD_BUTTON_B);
	input_manager.add_action("move_faster");
	input_manager.add_key_to_action("move_faster", InputKey::LEFT_SHIFT);

	input_manager.add_action("control_camera");
	input_manager.add_key_to_action("control_camera", InputKey::MOUSE_RIGHT);

	input_manager.add_action("select_multiple");
	input_manager.add_key_to_action("select_multiple", InputKey::LEFT_CONTROL);
	input_manager.add_action("select_rows");
	input_manager.add_key_to_action("select_rows", InputKey::LEFT_SHIFT);
	input_manager.add_action("select_mode");
	input_manager.add_key_to_action("select_mode", InputKey::Q);

	input_manager.add_action("translate_mode");
	input_manager.add_key_to_action("translate_mode", InputKey::W);
	input_manager.add_action("rotate_mode");
	input_manager.add_key_to_action("rotate_mode", InputKey::E);
	input_manager.add_action("scale_mode");
	input_manager.add_key_to_action("scale_mode", InputKey::R);
	input_manager.add_action("toggle_gizmo_mode");
	input_manager.add_key_to_action("toggle_gizmo_mode", InputKey::T);
	input_manager.add_action("toggle_snapping");
	input_manager.add_key_to_action("toggle_snapping", InputKey::Y);

	input_manager.add_action("clear_selection");
	input_manager.add_key_to_action("clear_selection", InputKey::ESCAPE);

	input_manager.add_action("control_modifier");
	input_manager.add_key_to_action("control_modifier", InputKey::LEFT_CONTROL);

	input_manager.add_action("duplicate");
	input_manager.add_key_to_action("duplicate", InputKey::D);

	input_manager.add_action("duplicate_wall_cube");
	input_manager.add_key_to_action("duplicate_wall_cube", InputKey::F);

	input_manager.add_action("delete");
	input_manager.add_key_to_action("delete", InputKey::DELETE);

	input_manager.add_action("hacker_exit_camera");
	input_manager.add_key_to_action("hacker_exit_camera", InputKey::TAB);
}

void bootleg_unity_theme() {
	ImVec4 *colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
	colors[ImGuiCol_Border] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.06f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.21f, 0.21f, 0.21f, 0.54f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.37f, 0.37f, 0.37f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.37f, 0.37f, 0.37f, 0.54f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_Button] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.28f, 0.38f, 0.49f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.24f, 0.24f, 0.33f);
	colors[ImGuiCol_Separator] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 0.52f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.16f, 0.16f, 0.16f, 0.52f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.71f, 0.71f, 0.71f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

	ImGuiStyle &style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(8.00f, 8.00f);
	style.FramePadding = ImVec2(5.00f, 2.00f);
	style.CellPadding = ImVec2(6.00f, 1.00f);
	style.ItemSpacing = ImVec2(6.00f, 3.00f);
	style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
	style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	style.IndentSpacing = 25;
	style.ScrollbarSize = 15;
	style.GrabMinSize = 10;
	style.WindowBorderSize = 1;
	style.ChildBorderSize = 0;
	style.PopupBorderSize = 1;
	style.FrameBorderSize = 1;
	style.TabBorderSize = 0;
	style.WindowRounding = 7;
	style.ChildRounding = 4;
	style.FrameRounding = 2;
	style.PopupRounding = 4;
	style.ScrollbarRounding = 9;
	style.GrabRounding = 2;
	style.LogSliderDeadzone = 4;
	style.TabRounding = 2;

	ImGuiIO &io = ImGui::GetIO();
	ImFontConfig config;
	config.OversampleH = 4;
	config.OversampleV = 4;
	float base_font_size = 15.0f;
	ImFont *font = io.Fonts->AddFontFromFileTTF("resources/fonts/Lato-Regular.ttf", 15, &config);

	// Setup ImGui icons
	static const ImWchar icons_ranges[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
	float icon_font_size = base_font_size * 2.0f / 3.0f;
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.OversampleH = 4;
	icons_config.OversampleV = 4;
	icons_config.GlyphMinAdvanceX = icon_font_size;

	io.Fonts->AddFontFromFileTTF(
			"resources/fonts/MaterialIcons-Regular.ttf", icon_font_size, &icons_config, icons_ranges);
}

Editor *Editor::get() {
	static Editor editor;
	return &editor;
}

void Editor::startup() {
	Engine::startup();
	RenderManager::get().editor_mode = true;

	// Additional setup
	default_mappings();

	// Native file dialog
	NFD_Init();

	bootleg_unity_theme();

	// load file image
	int tex_width, tex_height, tex_channels;
	stbi_uc *pixels = stbi_load("resources/icons/file.png", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

	glGenTextures(1, &file_texture);
	glBindTexture(GL_TEXTURE_2D, file_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(pixels);

	// load folder image
	pixels = stbi_load("resources/icons/folder.png", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

	glGenTextures(1, &folder_texture);
	glBindTexture(GL_TEXTURE_2D, folder_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(pixels);
	SPDLOG_INFO("Editor started up");
}

void Editor::shutdown() {
	Engine::shutdown();
}

void Editor::custom_update(float dt) {
	ZoneScopedN("Editor::custom_update");
	InputManager &input_manager = InputManager::get();
	DisplayManager &display_manager = DisplayManager::get();
	RenderManager &render_manager = RenderManager::get();

	// Handle current gizmo operation
	if (!controlling_camera && viewport_hovered) {
		if (input_manager.is_action_just_pressed("translate_mode")) {
			current_gizmo_operation = ImGuizmo::TRANSLATE;
		} else if (input_manager.is_action_just_pressed("rotate_mode")) {
			current_gizmo_operation = ImGuizmo::ROTATE;
		} else if (input_manager.is_action_just_pressed("scale_mode")) {
			current_gizmo_operation = ImGuizmo::SCALE;
		}

		if (input_manager.is_action_just_pressed("toggle_gizmo_mode")) {
			if (current_gizmo_mode == ImGuizmo::WORLD) {
				current_gizmo_mode = ImGuizmo::LOCAL;
			} else {
				current_gizmo_mode = ImGuizmo::WORLD;
			}
		}

		if (input_manager.is_action_just_pressed("toggle_snapping")) {
			use_snapping = !use_snapping;
		}

		if (input_manager.is_action_just_pressed("clear_selection")) {
			auto editor_scene = dynamic_cast<EditorScene *>(scenes[active_scene].get());
			get_editor_scene(active_scene).selected_entity = 0;
		}
	}

	if (!controlling_camera) {
		if (input_manager.is_action_pressed("control_modifier") && input_manager.is_action_just_pressed("duplicate")) {
			get_active_scene().duplicate_selected_entity();
		}

		if (input_manager.is_action_pressed("control_modifier") &&
				input_manager.is_action_just_pressed("duplicate_wall_cube")) {
			auto &scene = get_active_scene();
			auto &world = scene.world;
			auto &entities = scene.entities;
			auto selected_entity = scene.selected_entity;
			std::ifstream file("resources/prefabs/Walls/WallCube.pfb");
			nlohmann::json json;
			file >> json;
			SPDLOG_CRITICAL(json.dump(2));
			auto root_entity = world.deserialize_prefab(json, entities);

			if (world.has_component<Parent>(selected_entity)) {
				auto &selected_parent_component = world.get_component<Parent>(selected_entity);
				auto selected_parent = selected_parent_component.parent;

				if (world.has_component<Parent>(root_entity)) {
					auto &root_parent = world.get_component<Parent>(root_entity);
					root_parent.parent = selected_parent;
				} else {
					world.add_component(root_entity, Parent{ selected_parent });
				}

				auto &selected_parent_children = world.get_component<Children>(selected_parent);
				world.add_child(selected_parent, root_entity);
			}

			if (selected_entity != 0 && root_entity != 0) {
				auto transform = world.get_component<Transform>(selected_entity);
				auto &new_transform = world.get_component<Transform>(root_entity);
				new_transform = transform;
				file.close();
				scene.selected_entity = root_entity;
			}
		}

		if (input_manager.is_action_pressed("delete")) {
			get_active_scene().entity_deletion_queue.push(get_active_scene().selected_entity);
		}
	}

	if (show_cvar_editor) {
		CVarSystem::get()->draw_imgui_editor();
	}

	if (display_manager.window_should_close()) {
		should_run = false;
	}

	// GUI
	ImGuiIO &io = ImGui::GetIO();
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoWindowMenuButton);

	imgui_menu_bar();
	imgui_content_browser();
	imgui_settings();

	imgui_layers_settings();
	if (!scenes.empty()) {
		auto &scene = get_editor_scene(active_scene);
		inspector.world = &scene.world;
		imgui_inspector(scene);
		imgui_scene(scene);
	} else {
		ImGui::Begin("Scene");
		ImGui::End();
		ImGui::Begin("Inspector");
		ImGui::End();
	}

	viewport_hovered = false;
	for (int i = 0; i < scenes.size(); i++) {
		World &w = scenes[i]->world;
		auto &scene = dynamic_cast<EditorScene &>(*scenes[i]);
		if (scene.is_visible) {
			scene.update(dt);
		}
		imgui_viewport(scene, i);

		if (!scene.entity_deletion_queue.empty()) {
			Entity entity_to_remove = scene.entity_deletion_queue.front();
			scene.entity_deletion_queue.pop();
			// Deselect entity if it is selected
			if (entity_to_remove == scene.selected_entity) {
				scene.selected_entity = 0;
			}
			// Remove parent / child compontents if they're present
			if (w.has_component<Parent>(entity_to_remove)) {
				Entity parent = w.get_component<Parent>(entity_to_remove).parent;
				w.remove_child(parent, entity_to_remove);
			}
			if (w.has_component<Children>(entity_to_remove)) {
				auto children = w.get_component<Children>(entity_to_remove);
				for (int i = 0; i < children.children_count; i++) {
					w.remove_child(entity_to_remove, children.children[i]);
				}
			}
			scene.entities.erase(
					std::remove(scene.entities.begin(), scene.entities.end(), entity_to_remove), scene.entities.end());
			w.destroy_entity(entity_to_remove);
		}
	}

	if (scene_deletion_queued) {
		EditorScene &scene = get_editor_scene(scene_to_delete);
		render_manager.render_scenes.erase(render_manager.render_scenes.begin() + scene.render_scene_idx);
		scenes.erase(scenes.begin() + scene_to_delete);
		active_scene = 0;
		scene_deletion_queued = false;
	}
}
void Editor::create_scene(const std::string &name) {
	create_scene(name, SceneType::GameScene);
}

void Editor::create_scene(const std::string &name, SceneType type, const std::string &path) {
	auto scene = std::make_unique<EditorScene>(type);
	scene->name = name;

	// Create RenderScene for scene
	RenderManager &render_manager = RenderManager::get();
	scene->render_scene_idx = render_manager.create_render_scene();
	if (type == SceneType::Prefab) {
		render_manager.render_scenes[scene->render_scene_idx].skybox_pass.skybox.load_from_directory(
				asset_path("cubemaps/venice_sunset"));
	}

	Entity entity;

	if (!path.empty()) {
		std::ifstream file(path);
		nlohmann::json serialized_scene = nlohmann::json::parse(file);
		file.close();
		//		file >> serialized_scene;
		//		serialized_scene.back()["entity"] = 0;
		scene->world.deserialize_entities_json(serialized_scene, scene->entities);
		scenes.push_back(std::move(scene));
		return;
	}

	if (type == SceneType::Prefab) {
		entity = scene->world.create_entity();
		scene->entities.push_back(entity);
		scene->world.add_component<Name>(entity, Name{ "New prefab" });
		scene->world.add_component<Transform>(entity, Transform{});
	}

	scenes.push_back(std::move(scene));
}

EditorScene &Editor::get_editor_scene(uint32_t index) {
	return dynamic_cast<EditorScene &>(*scenes[index]);
}

EditorScene &Editor::get_active_scene() {
	return get_editor_scene(active_scene);
}
