#include "editor.h"
#include "ecs/ecs_manager.h"
#include "input/input_manager.h"
#include "render/render_manager.h"
#include <imgui.h>

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

Editor *Editor::get() {
	static Editor editor;
	return &editor;
}

void Editor::startup() {
	// Managers
	SPDLOG_INFO("Starting up engine systems...");
	DisplayManager::get().startup(true);
	InputManager::get().startup();
	ECSManager::get().startup();
	RenderManager::get().startup();

	ECSManager &ecs_manager = ECSManager::get();
	RenderManager &render_manager = RenderManager::get();

	// Components
	ecs_manager.register_component<Name>();
	ecs_manager.register_component<Transform>();
	ecs_manager.register_component<RigidBody>();
	ecs_manager.register_component<Gravity>();
	ecs_manager.register_component<Parent>();
	ecs_manager.register_component<Children>();
	ecs_manager.register_component<ModelInstance>();
	ecs_manager.register_component<FmodListener>();
	ecs_manager.register_component<ColliderTag>();
	ecs_manager.register_component<ColliderSphere>();
	ecs_manager.register_component<ColliderAABB>();
	ecs_manager.register_component<ColliderOBB>();

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

	render_manager.load_model("cardboardBox/console.mdl");
	render_manager.load_model("electricBox2/electricBox2.mdl");

	scenes[0].entities.push_back(entity);

	create_scene("Another Scene");

	entity = ecs_manager.create_entity();

	ecs_manager.add_component<Transform>(
			entity, Transform{ glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f) });

	ecs_manager.add_component<ModelInstance>(entity, ModelInstance("electricBox/electricBox.mdl", MaterialType::PBR));

	scenes[1].entities.push_back(entity);
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
	//	if (!controlling_camera) {
	//		if (input_manager.is_action_just_pressed("translate_mode")) {
	//			current_gizmo_operation = ImGuizmo::TRANSLATE;
	//		} else if (input_manager.is_action_just_pressed("rotate_mode")) {
	//			current_gizmo_operation = ImGuizmo::ROTATE;
	//		} else if (input_manager.is_action_just_pressed("scale_mode")) {
	//			current_gizmo_operation = ImGuizmo::SCALE;
	//		}
	//
	//		if (input_manager.is_action_just_pressed("toggle_gizmo_mode")) {
	//			if (current_gizmo_mode == ImGuizmo::WORLD) {
	//				current_gizmo_mode = ImGuizmo::LOCAL;
	//			} else {
	//				current_gizmo_mode = ImGuizmo::WORLD;
	//			}
	//		}
	//	}

	if (show_cvar_editor) {
		CVarSystem::get()->draw_imgui_editor();
	}

	if (display_manager.window_should_close()) {
		should_run = false;
	}

	// GUI
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiTreeNodeFlags_SpanFullWidth;
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	imgui_menu_bar();
	imgui_resources();
	imgui_inspector();
	imgui_settings();
	imgui_scene(scenes[current_scene]);

	for (auto &scene : scenes) {
		scene.update(dt);
		imgui_viewport(scene);
	}

	debug_draw::draw_line(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f, 0.0f, 0.0f));
	debug_draw::draw_line(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(10.0f, 0.0f, 0.0f));

	debug_draw::draw_line(glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(10.0f, 0.0f, 2.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	debug_draw::draw_box(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	debug_draw::draw_box(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(10.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

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
