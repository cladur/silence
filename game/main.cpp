#include "audio/audio_manager.h"
#include "display/display_manager.h"
#include "font/font_manager.h"
#include "input/input_manager.h"

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
// #include "render/debug/debug_draw.h"
// #include "render/text/text_draw.h"

ECSManager ecs_manager;
AudioManager audio_manager;
InputManager input_manager;

Camera camera(glm::vec3(0.0f, 0.0f, -25.0f));

extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

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

	input_manager.add_action("collider_forward");
	input_manager.add_key_to_action("collider_forward", InputKey::I);
	input_manager.add_action("collider_backward");
	input_manager.add_key_to_action("collider_backward", InputKey::K);
	input_manager.add_action("collider_left");
	input_manager.add_key_to_action("collider_left", InputKey::J);
	input_manager.add_action("collider_right");
	input_manager.add_key_to_action("collider_right", InputKey::L);
	input_manager.add_action("collider_up");
	input_manager.add_key_to_action("collider_up", InputKey::O);
	input_manager.add_action("collider_down");
	input_manager.add_key_to_action("collider_down", InputKey::U);
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
	std::uniform_real_distribution<float> rand_color(0.0f, 1.0f);
	std::uniform_real_distribution<float> rand_gravity(-40.0f, -20.0f);
	std::uniform_real_distribution<float> rand_material(0.0f, 1.0f);

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

		Handle<ModelInstance> hndl = OpenglManager::get()->add_instance("woodenBox/woodenBox.pfb");
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
	auto display_manager_result = DisplayManager::get()->startup();
	if (display_manager_result == DisplayManager::Status::Ok) {
		SPDLOG_INFO("Initialized display manager");
	} else {
		SPDLOG_ERROR("Failed to initialize the display manager. Status: ({}) {}",
				magic_enum::enum_integer(display_manager_result), magic_enum::enum_name(display_manager_result));
		return false;
	}

	return true;
}

void setup_imgui_style() {
	// zajebisty styl wulkanowy czerwony 😎
	float hue_shift = 0.398f;
	float saturation_shift = 0.05f;
	static ImGuiStyle base_style = ImGui::GetStyle();
	ImGuiStyle &style = ImGui::GetStyle();
	for (int i = 0; i < ImGuiCol_COUNT; i++) {
		ImVec4 &base_col = base_style.Colors[i];
		float hue, saturation, value;
		ImGui::ColorConvertRGBtoHSV(base_col.x, base_col.y, base_col.z, hue, saturation, value);
		hue += hue_shift;
		if (hue > 1.0f) {
			hue -= 1.0f;
		}
		saturation += saturation_shift;
		if (saturation > 1.0f) {
			saturation = 1.0f;
		}
		ImVec4 &target_col = style.Colors[i];
		ImGui::ColorConvertHSVtoRGB(hue, saturation, value, target_col.x, target_col.y, target_col.z);
	}
}

void destroy_all_entities(const std::vector<Entity> &entities) {
	for (unsigned int entity : entities) {
		ecs_manager.destroy_entity(entity);
	}
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

void handle_collider_movement(Entity entity, float dt) {
	float forward = -input_manager.get_axis("collider_backward", "collider_forward");
	float right = -input_manager.get_axis("collider_left", "collider_right");
	float up = -input_manager.get_axis("collider_down", "collider_up");

	auto &t = ecs_manager.get_component<Transform>(entity);

	const float speed = 5.0f;
	t.add_position(glm::vec3(forward, up, right) * speed * dt);
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

	// nie dla psa 😔
	// setup_imgui_style();

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
	int imgui_children_id = 1;
	int imgui_entity_id = 1;
	int max_imgui_entities = 50;
	int max_entities = 100;
	int imgui_entities_count = 50;
	int frames_count = 0;

	char load_file_name[128] = "scene.json";
	char save_file_name[128] = "scene.json";

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

		DisplayManager::get()->poll_events();

		if (input_manager.is_action_just_pressed("debug_menu")) {
			in_debug_menu = !in_debug_menu;
			DisplayManager::get()->capture_mouse(!in_debug_menu);
		}

		if (!in_debug_menu) {
			handle_camera(camera, dt);
		}
		handle_collider_movement(collision_tester, dt);

		//imgui new frame
#ifdef USE_OPENGL
		ImGui_ImplOpenGL3_NewFrame();
#else
		ImGui_ImplVulkan_NewFrame();
#endif
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings");

		ImGui::Text("%s", fmt::format("In debug menu: {}", in_debug_menu).c_str());

		ImGui::Checkbox("Show console ecs logs", &show_ecs_logs);

		ImGui::Checkbox("Show demo window", &show_demo_window);

		ImGui::Checkbox("Show CVAR editor", &show_cvar_editor);

		ImGui::Checkbox("Physics system", &physics_system_enabled);

		if (show_demo_window) {
			ImGui::ShowDemoWindow();
		}

		ImGui::DragInt("Entity Id", &imgui_entity_id, 1, 1, max_imgui_entities);
		ImGui::DragInt("Children Id", &imgui_children_id, 1, 1, max_imgui_entities);

		if (ImGui::Button("Add child")) {
			ecs_manager.add_child(imgui_entity_id, imgui_children_id);
		}

		if (ImGui::Button("Remove child")) {
			ecs_manager.remove_child(imgui_entity_id, imgui_children_id);
		}

		if (ImGui::Button("Destroy all entities")) {
			destroy_all_entities(entities);
			entities.clear();
		}

		ImGui::Text("Save file");
		ImGui::InputText("#Save file name", save_file_name, IM_ARRAYSIZE(save_file_name));

		if (ImGui::Button("Save scene")) {
			scene = SceneManager::save_scene(entities);
			SceneManager::save_json_to_file(save_file_name, scene);
		}

		ImGui::Text("Load file");
		ImGui::InputText("#Load file name", load_file_name, IM_ARRAYSIZE(load_file_name));

		if (ImGui::Button("Load scene")) {
			std::ifstream file(load_file_name);
			if (file.is_open()) {
				SPDLOG_INFO("Loaded scene from file {}", load_file_name);
				nlohmann::json scene_json = nlohmann::json::parse(file);
				file.close();
				destroy_all_entities(entities);
				SceneManager::load_scene_from_json_file(scene_json, load_file_name, entities);
			} else {
				SPDLOG_ERROR("File {} not found", load_file_name);
			}
		}

		ImGui::DragInt("Entities count", &imgui_entities_count, 1, 1, max_entities);

		if (ImGui::Button("Create entities")) {
			destroy_all_entities(entities);
			entities.resize(imgui_entities_count - 1);
			demo_entities_init(entities);
		}

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

		ImGui::Begin("Text Demo");

		static char buffer[128] = { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!' };
		static float scale = 1.0f;
		static float position[3] = { 0.0f, 0.0f, 0.0f };
		static glm::vec3 color = { 1.0f, 1.0f, 1.0f };
		static bool screenspace = false;
		static bool centered_x = false;
		static bool centered_y = false;
		ImGui::InputText("Text", buffer, 128);
		ImGui::InputFloat("Scale", &scale);
		ImGui::InputFloat3("Position", position);
		ImGui::InputFloat3("Color", &color.x);
		ImGui::Checkbox("Screenspace", &screenspace);
		ImGui::Checkbox("Center X", &centered_x);
		ImGui::Checkbox("Center Y", &centered_y);

		ImGui::End();

		text_draw::draw_text(std::string(buffer), screenspace, glm::vec3(position[0], position[1], position[2]), color,
				scale, nullptr, centered_x, centered_y);

		debug_draw::draw_line(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f, 0.0f, 0.0f));
		debug_draw::draw_line(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(10.0f, 0.0f, 0.0f));

		debug_draw::draw_line(glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(10.0f, 0.0f, 2.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		debug_draw::draw_box(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		debug_draw::draw_box(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(10.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		// TODO: remove this when collision demo will be removed
		for (auto sphere : spheres) {
			auto &c = ecs_manager.get_component<ColliderSphere>(sphere);
			debug_draw::draw_sphere(c.center, c.radius);
		}

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
