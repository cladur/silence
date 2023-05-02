#include "editor.h"
#include "ecs/ecs_manager.h"
#include "input/input_manager.h"
#include "render/render_manager.h"
#include <imgui.h>
#include <imgui_internal.h>

#include "IconsMaterialDesign.h"
#include "inspector_gui.h"
// #include "IconsFontAwesome5.h"
template <typename T> void register_component() {
	ECSManager &ecs_manager = ECSManager::get();
	int type_id = ecs_manager.get_registered_components();
	ecs_manager.register_component<T>();
	Inspector::add_mapping(type_id, []() { Inspector::show_component<T>(); });
}

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

	input_manager.add_action("delete");
	input_manager.add_key_to_action("delete", InputKey::BACKSPACE);
}

void demo_entities_init(std::vector<Entity> &entities) {
	ECSManager &ecs_manager = ECSManager::get();

	std::default_random_engine random_generator; // NOLINT(cert-msc51-cpp)
	std::uniform_real_distribution<float> rand_position(-20.0f, 20.0f);
	std::uniform_real_distribution<float> rand_rotation(0.0f, 0.0f);
	std::uniform_real_distribution<float> rand_scale(3.0f, 5.0f);
	std::uniform_real_distribution<float> rand_gravity(-9.8f, -9.8f);

	float scale = rand_scale(random_generator);

	for (unsigned int &entity : entities) {
		entity = ecs_manager.create_entity();

		ecs_manager.add_component<Name>(entity, Name("Entity " + std::to_string(entity)));

		ecs_manager.add_component<Gravity>(entity, { glm::vec3(0.0f, rand_gravity(random_generator), 0.0f) });

		ecs_manager.add_component(entity,
				RigidBody{ .velocity = glm::vec3(0.0f, 0.0f, 0.0f), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f) });

		Transform transform = Transform{ glm::vec3(rand_position(random_generator),
												 rand_position(random_generator) + 20.0f,
												 rand_position(random_generator)),
			glm::vec3(rand_rotation(random_generator), -50.0f, rand_rotation(random_generator)), glm::vec3(1.0f) };
		ecs_manager.add_component(entity, transform);

		ColliderComponentsFactory::add_collider_component(
				entity, ColliderAABB{ transform.get_position(), transform.get_scale(), true });

		//		Handle<ModelInstance> hndl = RenderManager::get()->add_instance("electricBox/electricBox.mdl");
		//		Handle<ModelInstance> hndl = RenderManager::get()->add_instance("Agent/agent_idle.mdl");
		ecs_manager.add_component<ModelInstance>(entity, ModelInstance("woodenBox/woodenBox.mdl"));
	}

	auto listener = ecs_manager.create_entity();
	ecs_manager.add_component(listener, Transform{ glm::vec3(0.0f, 0.0f, -25.0f), glm::vec3(0.0f), glm::vec3(1.0f) });
	// Later on attach FmodListener component to camera
	ecs_manager.add_component<FmodListener>(listener,
			FmodListener{ .listener_id = SILENCE_FMOD_LISTENER_DEBUG_CAMERA,
					.prev_frame_position = glm::vec3(0.0f, 0.0f, -25.0f) });
	entities.push_back(listener);

	Entity floor = ecs_manager.create_entity();

	Transform transform = Transform{ glm::vec3(0.0f, -20.0f, 0.0f), glm::vec3(0.0f), glm::vec3(20.0f, 1.0f, 20.0f) };
	ecs_manager.add_component(floor, transform);

	ColliderComponentsFactory::add_collider_component(
			floor, ColliderAABB{ transform.get_position(), transform.get_scale(), false });

	entities.push_back(floor);
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
	// Managers
	SPDLOG_INFO("Starting up engine systems...");
	DisplayManager::get().startup("Silence Engine", true);
	InputManager::get().startup();
	ECSManager::get().startup();
	RenderManager::get().startup();

	ECSManager &ecs_manager = ECSManager::get();
	RenderManager &render_manager = RenderManager::get();

	// Components
	register_component<Name>();
	register_component<Transform>();
	register_component<RigidBody>();
	register_component<Gravity>();
	register_component<Parent>();
	register_component<Children>();
	register_component<ModelInstance>();
	register_component<FmodListener>();
	register_component<ColliderTag>();
	register_component<ColliderSphere>();
	register_component<ColliderAABB>();
	register_component<ColliderOBB>();

	// Systems
	physics_system = ecs_manager.register_system<PhysicsSystem>();
	collision_system = ecs_manager.register_system<CollisionSystem>();
	parent_system = ecs_manager.register_system<ParentSystem>();
	render_system = ecs_manager.register_system<RenderSystem>();

	render_system->startup();
	physics_system->startup();
	collision_system->startup();
	parent_system->startup();
	FontManager::get().startup();
	FontManager::get().load_font("resources/fonts/PoltawskiNowy.ttf", 48);

	// Additional setup
	default_mappings();

	// Native file dialog
	NFD_Init();

	// Default scene
	create_scene("Default");

	Entity entity = ecs_manager.create_entity();

	ecs_manager.add_component<Transform>(
			entity, Transform{ glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f) });

	ecs_manager.add_component<ModelInstance>(entity, ModelInstance("woodenBox/woodenBox.mdl", MaterialType::PBR));

	ecs_manager.add_component<Name>(entity, Name("Wooden Box"));

	render_manager.load_model("cardboardBox/console.mdl");
	render_manager.load_model("electricBox2/electricBox2.mdl");

	scenes[0].entities.push_back(entity);

	create_scene("Another Scene");

	entity = ecs_manager.create_entity();

	ecs_manager.add_component<Transform>(
			entity, Transform{ glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f) });

	ecs_manager.add_component<ModelInstance>(entity, ModelInstance("electricBox/electricBox.mdl", MaterialType::PBR));

	ecs_manager.add_component<Name>(entity, Name("Electric Box"));

	scenes[1].entities.push_back(entity);

	Entity parent = entity;

	entity = ecs_manager.create_entity();

	ecs_manager.add_component<Transform>(
			entity, Transform{ glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f) });

	ecs_manager.add_component<ModelInstance>(entity, ModelInstance("woodenBox/woodenBox.mdl", MaterialType::PBR));

	ecs_manager.add_component<Name>(entity, Name("Wooden Box"));

	ecs_manager.add_child(parent, entity);

	scenes[1].entities.push_back(entity);

	for (int i = 0; i < 10; i++) {
		entity = ecs_manager.create_entity();
		ecs_manager.add_component<Transform>(
				entity, Transform{ glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f) });
		scenes[1].entities.push_back(entity);
	}

	bootleg_unity_theme();
}

void Editor::shutdown() {
	// Shut everything down, in reverse order.
	SPDLOG_INFO("Shutting down engine subsystems...");
	InputManager::get().shutdown();
	RenderManager::get().shutdown();
	DisplayManager::get().shutdown();
}

void Editor::run() {
	float target_frame_time = 1.0f / (float)DisplayManager::get().get_refresh_rate();
	float dt = target_frame_time;

	nlohmann::json scene;
	while (should_run) {
		auto start_time = std::chrono::high_resolution_clock::now();

		// GAME LOGIC
		update(dt);

		auto stop_time = std::chrono::high_resolution_clock::now();

		// TODO: This is a hack to make sure we don't go over the target frame time.
		// We need to make calculate dt properly and make target frame time changeable.
		while (std::chrono::duration<float, std::chrono::seconds::period>(stop_time - start_time).count() <
				target_frame_time) {
			stop_time = std::chrono::high_resolution_clock::now();
		}

		FrameMark;
	}
}

void Editor::update(float dt) {
	InputManager &input_manager = InputManager::get();
	DisplayManager &display_manager = DisplayManager::get();
	RenderManager &render_manager = RenderManager::get();
	ECSManager &ecs_manager = ECSManager::get();

	//imgui new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	DisplayManager::get().poll_events();

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
	imgui_resources();
	imgui_settings();
	imgui_inspector(scenes[active_scene]);
	imgui_scene(scenes[active_scene]);

	viewport_hovered = false;
	for (int i = 0; i < scenes.size(); i++) {
		Scene &scene = scenes[i];
		if (scene.is_visible) {
			scene.update(dt);
		}
		imgui_viewport(scene, i);
	}

	parent_system->update();
	render_system->update();
	input_manager.process_input();

	render_manager.draw();
}

void Editor::create_scene(const std::string &name) {
	Scene scene = {};
	scene.name = name;

	// Create RenderScene for scene
	RenderManager &render_manager = RenderManager::get();
	scene.render_scene_idx = render_manager.create_render_scene();

	scenes.push_back(scene);
}

uint32_t Editor::get_scene_index(const std::string &name) {
	for (int i = 0; i < scenes.size(); i++) {
		if (scenes[i].name == name) {
			return i;
		}
	}
	return 0;
}
