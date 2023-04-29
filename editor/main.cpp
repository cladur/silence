#include "audio/audio_manager.h"
#include "display/display_manager.h"
#include "font/font_manager.h"
#include "input/input_manager.h"
#include "opengl/material.h"

#ifdef USE_OPENGL
#include "imgui_impl_opengl3.h"
#include "opengl/opengl_manager.h"
#include "opengl/opengl_system.h"
#else
#include "imgui_impl_vulkan.h"
#include "render/render_manager.h"
#include "render/render_system.h"
#endif

#include "components/children_component.h"
#include "components/collider_aabb.h"
#include "components/collider_obb.h"
#include "components/collider_sphere.h"
#include "components/gravity_component.h"
#include "components/parent_component.h"
#include "components/rigidbody_component.h"
#include "components/transform_component.h"
#include "opengl/render_handle.h"

#include "ecs/ecs_manager.h"
#include "ecs/systems/collider_components_factory.h"
#include "ecs/systems/collision_system.h"
#include "ecs/systems/parent_system.h"
#include "ecs/systems/physics_system.h"

#include "audio/fmod_listener_system.h"
#include "components/fmod_listener_component.h"
#include "imgui_impl_glfw.h"

#include "scene/scene_manager.h"
#include "serialization.h"
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <fstream>

#include "core/camera/camera.h"

#include <nfd.h>

ECSManager ecs_manager;
AudioManager audio_manager;
InputManager input_manager;

Camera camera(glm::vec3(0.0f, 0.0f, -25.0f));

bool in_debug_menu = true;

void default_mappings() {
	input_manager.add_action("debug_menu");
	input_manager.add_key_to_action("debug_menu", InputKey::ESCAPE);
	input_manager.add_key_to_action("debug_menu", InputKey::GAMEPAD_START);
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
	input_manager.add_key_to_action("move_up", InputKey::SPACE);
	input_manager.add_key_to_action("move_up", InputKey::GAMEPAD_BUTTON_A);
	input_manager.add_action("move_down");
	input_manager.add_key_to_action("move_down", InputKey::LEFT_SHIFT);
	input_manager.add_key_to_action("move_down", InputKey::GAMEPAD_BUTTON_B);
}

void default_ecs_manager_init() {
	ecs_manager.startup();

	ecs_manager.register_component<Transform>();
	ecs_manager.register_component<RigidBody>();
	ecs_manager.register_component<Gravity>();
	ecs_manager.register_component<Parent>();
	ecs_manager.register_component<Children>();
	ecs_manager.register_component<RenderHandle>();
	ecs_manager.register_component<FmodListener>();
	ecs_manager.register_component<ColliderTag>();
	ecs_manager.register_component<ColliderSphere>();
	ecs_manager.register_component<ColliderAABB>();
	ecs_manager.register_component<ColliderOBB>();
}

void demo_entities_init(std::vector<Entity> &entities) {
	std::default_random_engine random_generator; // NOLINT(cert-msc51-cpp)
	std::uniform_real_distribution<float> rand_position(-40.0f, 40.0f);
	std::uniform_real_distribution<float> rand_rotation(0.0f, 0.0f);
	std::uniform_real_distribution<float> rand_scale(3.0f, 5.0f);
	std::uniform_real_distribution<float> rand_gravity(-40.0f, -20.0f);

	float scale = rand_scale(random_generator);

	for (unsigned int &entity : entities) {
		entity = ecs_manager.create_entity();

		ecs_manager.add_component<Gravity>(entity, { glm::vec3(0.0f, rand_gravity(random_generator), 0.0f) });

		ecs_manager.add_component(entity,
				RigidBody{ .velocity = glm::vec3(0.0f, 0.0f, 0.0f), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f) });

		Transform transform = Transform{ glm::vec3(rand_position(random_generator),
												 rand_position(random_generator) + 40.0f,
												 rand_position(random_generator)),
			glm::vec3(rand_rotation(random_generator), -50.0f, rand_rotation(random_generator)), glm::vec3(4.0f) };
		ecs_manager.add_component(entity, transform);

		ColliderComponentsFactory::add_collider_component(
				entity, ColliderAABB{ transform.get_position(), transform.get_scale(), true });

		Handle<ModelInstance> hndl = OpenglManager::get()->add_instance("woodenBox/woodenBox.pfb", MATERIAL_TYPE_UNLIT);
		//		Handle<ModelInstance> hndl = OpenglManager::get()->add_instance("electricBox/electricBox.pfb");
		//		Handle<ModelInstance> hndl = OpenglManager::get()->add_instance("Agent/agent_idle.pfb");
		ecs_manager.add_component<RenderHandle>(entity, RenderHandle{ .handle = hndl });
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

void demo_collision_init(Entity &entity) {
	entity = ecs_manager.create_entity();

	Transform transform = Transform{ glm::vec3(0.0f), glm::vec3(45.0f), glm::vec3(1.0f) };
	ecs_manager.add_component(entity, transform);

	ColliderOBB c{};
	c.center = transform.get_position();
	c.range = transform.get_scale();
	c.set_orientation(transform.get_euler_rot());
	c.is_movable = true;
	ColliderComponentsFactory::add_collider_component(entity, c);

	// ecs_manager.add_component<MeshInstance>(
	// 		entity, { render_manager.get_mesh("box"), render_manager.get_material("default_mesh") });
}

void demo_collision_sphere(std::vector<Entity> &entities) {
	std::default_random_engine random_generator; // NOLINT(cert-msc51-cpp)
	std::uniform_real_distribution<float> rand_position(-20.0f, 20.0f);
	std::uniform_real_distribution<float> rand_gravity(-40.0f, -20.0f);

	for (Entity &entity : entities) {
		entity = ecs_manager.create_entity();

		Transform transform = Transform{ glm::vec3(rand_position(random_generator), rand_position(random_generator),
												 rand_position(random_generator)),
			glm::vec3(0.0f), glm::vec3(1.0f) };
		ecs_manager.add_component(entity, transform);

		ColliderComponentsFactory::add_collider_component(
				entity, ColliderSphere{ transform.get_position(), transform.get_scale().x, true });

		ecs_manager.add_component<Gravity>(entity, { glm::vec3(0.0f, rand_gravity(random_generator), 0.0f) });

		ecs_manager.add_component(entity,
				RigidBody{ .velocity = glm::vec3(0.0f, 0.0f, 0.0f), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f) });
	}
}

void demo_collision_obb(std::vector<Entity> &entities) {
	std::default_random_engine random_generator; // NOLINT(cert-msc51-cpp)
	std::uniform_real_distribution<float> rand_position(-10.0f, 10.0f);
	std::uniform_real_distribution<float> rand_rotation(-90.0f, 90.0f);
	std::uniform_real_distribution<float> rand_gravity(-40.0f, -20.0f);

	for (Entity &entity : entities) {
		entity = ecs_manager.create_entity();

		Transform transform = Transform{ glm::vec3(rand_position(random_generator), rand_position(random_generator),
												 rand_position(random_generator)),
			glm::vec3(rand_rotation(random_generator)), glm::vec3(1.0f) };
		ecs_manager.add_component(entity, transform);

		ColliderOBB c{};
		c.center = transform.get_position();
		c.range = transform.get_scale();
		c.set_orientation(transform.get_euler_rot());
		c.is_movable = true;

		ColliderComponentsFactory::add_collider_component(entity, c);

		ecs_manager.add_component<Gravity>(entity, { glm::vec3(0.0f, rand_gravity(random_generator), 0.0f) });

		ecs_manager.add_component(entity,
				RigidBody{ .velocity = glm::vec3(0.0f, 0.0f, 0.0f), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f) });
	}
}

bool display_manager_init() {
	auto display_manager_result = DisplayManager::get()->startup(true);
	if (display_manager_result == DisplayManager::Status::Ok) {
		SPDLOG_INFO("Initialized display manager");
	} else {
		SPDLOG_ERROR("Failed to initialize the display manager. Status: ({}) {}",
				magic_enum::enum_integer(display_manager_result), magic_enum::enum_name(display_manager_result));
		return false;
	}

	return true;
}

void handle_camera(Camera &cam, float dt) {
	float forward = input_manager.get_axis("move_backward", "move_forward");
	float right = input_manager.get_axis("move_left", "move_right");
	float up = input_manager.get_axis("move_down", "move_up");
	cam.move_forward(forward * dt);
	cam.move_right(right * dt);
	cam.move_up(up * dt);

	glm::vec2 mouse_delta = input_manager.get_mouse_delta();
	cam.rotate(mouse_delta.x * dt, mouse_delta.y * dt);
}

int main() {
	SPDLOG_INFO("Starting up engine systems...");

	display_manager_init();
	input_manager.startup();
#ifdef USE_OPENGL
	OpenglManager::get()->startup();
#else
	RenderManager::get()->startup();
#endif

	// ECS ----------------------------------------

	default_ecs_manager_init();
	auto physics_system = ecs_manager.register_system<PhysicsSystem>();
	auto collision_system = ecs_manager.register_system<CollisionSystem>();
	auto parent_system = ecs_manager.register_system<ParentSystem>();
	auto fmod_listener_system = ecs_manager.register_system<FmodListenerSystem>();

#ifdef USE_OPENGL
	auto opengl_system = ecs_manager.register_system<OpenglSystem>();
	opengl_system->startup();
#else
	auto render_system = ecs_manager.register_system<RenderSystem>();
	render_system->startup();
#endif

	physics_system->startup();
	collision_system->startup();
	parent_system->startup();
	fmod_listener_system->startup();
	FontManager::get()->startup();

	NFD_Init();

	std::vector<Entity> entities(50);
	demo_entities_init(entities);

	std::vector<Entity> spheres(10);
	demo_collision_sphere(spheres);

	std::vector<Entity> obbs(10);
	demo_collision_obb(obbs);

	Entity collision_tester;
	demo_collision_init(collision_tester);

	audio_manager.startup();
	audio_manager.load_bank("Music");
	audio_manager.load_bank("SFX");
	audio_manager.load_bank("Ambience");
	audio_manager.load_sample_data();

	FontManager::get()->load_font("resources/fonts/PoltawskiNowy.ttf", 48);

	//Map inputs
	default_mappings();

	// Run the game.
	bool show_cvar_editor = false;
	bool show_ecs_logs = false;
	bool show_demo_window = false;
	bool physics_system_enabled = false;
	int max_entities = 100;
	int imgui_entities_count = 50;
	int frames_count = 0;

	float target_frame_time = 1.0f / (float)DisplayManager::get()->get_refresh_rate();
	float dt = target_frame_time;

	// TEST FOR 3D AUDIO
	glm::vec3 sound_position = glm::vec3(0.0f, 0.0f, 0.0f);
	EventReference test_pluck = EventReference("test_pluck");
	// #################

	bool should_run = true;
	nlohmann::json scene;
	while (should_run) {
		// GAME LOGIC
		auto start_time = std::chrono::high_resolution_clock::now();

		//imgui new frame
#ifdef USE_OPENGL
		ImGui_ImplOpenGL3_NewFrame();
#else
		ImGui_ImplVulkan_NewFrame();
#endif
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		DisplayManager::get()->poll_events();

		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		// Add menu bar flag and disable everything else
		ImGuiWindowFlags flags2 = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse |
				ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar;

		//		if (ImGui::Begin("StatusBar", nullptr, flags)) {
		//			if (ImGui::BeginMenuBar()) {
		//				if (ImGui::BeginMenu("Rage")) {
		//					ImGui::MenuItem("Exit", NULL, &should_run);
		//					ImGui::EndMenu();
		//				}
		//
		//				ImGui::Button("Test button");
		//				ImGui::EndMenuBar();
		//			}
		//			ImGui::End();
		//		}

		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save")) {
				SPDLOG_INFO("Saving scene...");
			}
			if (ImGui::MenuItem("Save as...")) {
				SPDLOG_INFO("Saving scene...");
				nfdchar_t *outPath;
				nfdfilteritem_t filterItem[2] = { { "Source code", "c,cpp,cc" }, { "Headers", "h,hpp" } };
				nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 2, NULL);
				if (result == NFD_OKAY) {
					puts("Success!");
					puts(outPath);
					NFD_FreePath(outPath);
				} else if (result == NFD_CANCEL) {
					puts("User pressed cancel.");
				} else {
					printf("Error: %s\n", NFD_GetError());
				}
			}
			if (ImGui::MenuItem("Load")) {
				SPDLOG_INFO("Loading scene...");
			}
			if (ImGui::MenuItem("Exit")) {
				should_run = false;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();

		ImGui::Begin("Viewport");

		// Get viewport size
		static ImVec2 last_viewport_size = ImVec2(0, 0);
		ImVec2 viewport_size = ImGui::GetContentRegionAvail();
		if (viewport_size.x != last_viewport_size.x || viewport_size.y != last_viewport_size.y) {
			// Resize the framebuffer
			OpenglManager::get()->resize_framebuffer(viewport_size.x, viewport_size.y);
			last_viewport_size = viewport_size;
		}

		uint32_t render_image = OpenglManager::get()->render_framebuffer.get_texture_id();
		ImGui::Image((void *)(intptr_t)render_image, viewport_size, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();

		ImGui::Begin("Scene");

		ImGui::Text("TODO: List entities");

		ImGui::End();

		ImGui::Begin("Inspector");

		ImGui::Text("TODO: Show entity info");

		ImGui::End();

		ImGui::Begin("Resources");

		ImGui::Text("TODO: List resources");

		ImGui::End();

		if (input_manager.is_action_just_pressed("debug_menu")) {
			in_debug_menu = !in_debug_menu;
			DisplayManager::get()->capture_mouse(!in_debug_menu);
		}

		if (!in_debug_menu) {
			handle_camera(camera, dt);
		}

		ImGui::Begin("Settings");

		ImGui::Text("%s", fmt::format("In debug menu: {}", in_debug_menu).c_str());

		ImGui::Checkbox("Show console ecs logs", &show_ecs_logs);

		ImGui::Checkbox("Show demo window", &show_demo_window);

		ImGui::Checkbox("Show CVAR editor", &show_cvar_editor);

		ImGui::Checkbox("Physics system", &physics_system_enabled);

		if (show_demo_window) {
			ImGui::ShowDemoWindow();
		}

		ImGui::DragInt("Entities count", &imgui_entities_count, 1, 1, max_entities);

		// 3D SOUND DEMO
		ImGui::SliderFloat3("Sound position", &sound_position[0], -100.0f, 100.0f);

		if (ImGui::Button("Play pluck")) {
			audio_manager.play_one_shot_3d(test_pluck, sound_position);
		}
		// 3D SOUND DEMO

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
				ImGui::GetIO().Framerate);

		ImGui::End();

		if (show_cvar_editor) {
			CVarSystem::get()->draw_imgui_editor();
		}

		if (DisplayManager::get()->window_should_close()) {
			should_run = false;
		}

		if (physics_system_enabled) {
			physics_system->update(dt);
		}

		collision_system->update();

		parent_system->update();
#ifdef USE_OPENGL
		opengl_system->update();
#else
		render_system->update(*RenderManager::get());
#endif

		debug_draw::draw_line(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f, 0.0f, 0.0f));
		debug_draw::draw_line(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(10.0f, 0.0f, 0.0f));

		debug_draw::draw_line(glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(10.0f, 0.0f, 2.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		debug_draw::draw_box(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		debug_draw::draw_box(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(10.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		input_manager.process_input();

#ifdef USE_OPENGL
		OpenglManager::get()->draw();
#else
		RenderManager::get()->draw(camera);
#endif

		fmod_listener_system->update(dt);
		audio_manager.update();

		frames_count++;
		auto stop_time = std::chrono::high_resolution_clock::now();

		// TODO: This is a hack to make sure we don't go over the target frame time.
		// We need to make calculate dt properly and make target frame time changeable.
		while (std::chrono::duration<float, std::chrono::seconds::period>(stop_time - start_time).count() <
				target_frame_time) {
			stop_time = std::chrono::high_resolution_clock::now();
		}

		FrameMark;
	}

	// Shut everything down, in reverse order.
	SPDLOG_INFO("Shutting down engine subsystems...");
	input_manager.shutdown();
	audio_manager.shutdown();
#ifdef USE_OPENGL
	OpenglManager::get()->shutdown();
#else
	RenderManager::get()->shutdown();
#endif
	DisplayManager::get()->shutdown();

	return 0;
}
