#include "audio/audio_manager.h"
#include "managers/display/display_manager.h"
#include "managers/render/render_manager.h"

#include "components/children_component.h"
#include "components/gravity_component.h"
#include "components/parent_component.h"
#include "components/rigidbody_component.h"
#include "components/transform_component.h"

#include "ecs/systems/parent_system.h"
#include "ecs/systems/physics_system.h"
#include "render/render_system.h"

#include "ecs/ecs_manager.h"

#include "audio/fmod_listener.h"
#include "components/fmod_listener_component.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

RenderManager render_manager;
DisplayManager display_manager;
ECSManager ecs_manager;
AudioManager audio_manager;

void default_ecs_manager_init() {
	ecs_manager.startup();

	ecs_manager.register_component<Transform>();
	ecs_manager.register_component<RigidBody>();
	ecs_manager.register_component<Gravity>();
	ecs_manager.register_component<Parent>();
	ecs_manager.register_component<Children>();
	ecs_manager.register_component<MeshInstance>();
	ecs_manager.register_component<FmodListener>();
}

void demo_entities_init(std::vector<Entity> &entities) {
	std::default_random_engine random_generator; // NOLINT(cert-msc51-cpp)
	std::uniform_real_distribution<float> rand_position(-40.0f, 40.0f);
	std::uniform_real_distribution<float> rand_rotation(0.0f, 3.0f);
	std::uniform_real_distribution<float> rand_scale(3.0f, 5.0f);
	std::uniform_real_distribution<float> rand_color(0.0f, 1.0f);
	std::uniform_real_distribution<float> rand_gravity(-1000.0f, -100.0f);

	float scale = rand_scale(random_generator);

	for (unsigned int &entity : entities) {
		entity = ecs_manager.create_entity();

		ecs_manager.add_component<Gravity>(entity, { glm::vec3(0.0f, rand_gravity(random_generator), 0.0f) });

		ecs_manager.add_component(entity,
				RigidBody{ .velocity = glm::vec3(0.0f, 0.0f, 0.0f), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f) });

		ecs_manager.add_component(entity,
				Transform{ glm::vec3(rand_position(random_generator), rand_position(random_generator) + 40.0f,
								   rand_position(random_generator)),
						glm::vec3(rand_rotation(random_generator), rand_rotation(random_generator),
								rand_rotation(random_generator)),
						glm::vec3(scale, scale, scale) });

		ecs_manager.add_component<MeshInstance>(
				entity, { render_manager.get_mesh("box"), render_manager.get_material("default_mesh") });
	}

	auto listener = ecs_manager.create_entity();
	ecs_manager.add_component(listener, Transform{ glm::vec3(0.0f, 0.0f, -25.0f), glm::vec3(0.0f), glm::vec3(1.0f) });
	// Later on attach FmodListener component to camera
	ecs_manager.add_component<FmodListener>(listener,
			FmodListener{ .listener_id = SILENCE_FMOD_LISTENER_DEBUG_CAMERA,
					.prev_frame_position = glm::vec3(0.0f, 0.0f, -25.0f) });
	entities.push_back(listener);
}

bool display_manager_init() {
	auto display_manager_result = display_manager.startup();
	if (display_manager_result == DisplayManager::Status::Ok) {
		SPDLOG_INFO("Initialized display manager");
	} else {
		SPDLOG_ERROR("Failed to initialize the display manager. Status: ({}) {}",
				magic_enum::enum_integer(display_manager_result), magic_enum::enum_name(display_manager_result));
		return false;
	}

	return true;
}

bool render_manager_init() {
	auto render_manager_result = render_manager.startup(display_manager);
	if (render_manager_result == RenderManager::Status::Ok) {
		SPDLOG_INFO("Initialized render manager");
	} else {
		SPDLOG_ERROR("Failed to initialize the render manager. Status: ({}) {}",
				magic_enum::enum_integer(render_manager_result), magic_enum::enum_name(render_manager_result));
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

int main() {
	SPDLOG_INFO("Starting up engine systems...");

	if (!display_manager_init() || !render_manager_init()) {
		return -1;
	}

	setup_imgui_style();

	// ECS ----------------------------------------

	default_ecs_manager_init();
	auto physics_system = ecs_manager.register_system<PhysicsSystem>();
	auto parent_system = ecs_manager.register_system<ParentSystem>();
	auto render_system = ecs_manager.register_system<RenderSystem>();
	auto fmod_listener_system = ecs_manager.register_system<FmodListenerSystem>();

	physics_system->startup();
	parent_system->startup();
	render_system->startup();
	fmod_listener_system->startup();

	std::vector<Entity> entities(50);
	demo_entities_init(entities);

	audio_manager.startup();
	audio_manager.load_bank("Music");
	audio_manager.load_bank("SFX");
	audio_manager.load_bank("Ambience");
	audio_manager.load_sample_data();

	// Run the game.
	float dt{};
	bool show_ecs_logs = false;
	bool show_demo_window = false;
	bool physics_system_enabled = false;
	bool entities_destroyed = false;
	int imgui_children_id = 1;
	int imgui_entity_id = 1;
	int max_imgui_entities = 50;
	int max_entities = 100;
	int imgui_entities_count = 50;

	// TEST FOR 3D AUDIO
	glm::vec3 sound_position = glm::vec3(0.0f, 0.0f, 0.0f);
	EventReference test_pluck = EventReference("test_pluck");
	// #################

	bool should_run = true;
	while (should_run) {
		// GAME LOGIC

		auto start_time = std::chrono::high_resolution_clock::now();

		display_manager.poll_events();

		//imgui new frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings");

		ImGui::Checkbox("Show console ecs logs", &show_ecs_logs);

		ImGui::Checkbox("Show demo window", &show_demo_window);

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
			if (!entities_destroyed) {
				destroy_all_entities(entities);
			}
			entities_destroyed = true;
		}

		ImGui::DragInt("Entities count", &imgui_entities_count, 1, 1, max_entities);

		if (ImGui::Button("Create entities")) {
			if (!entities_destroyed) {
				destroy_all_entities(entities);
			}
			entities.resize(imgui_entities_count);
			demo_entities_init(entities);
			entities_destroyed = false;
		}

		// 3D SOUND DEMO
		ImGui::SliderFloat3("Sound position", &sound_position[0], -100.0f, 100.0f);

		if (ImGui::Button("Play pluck")) {
			//audio_manager.test_play_sound();
			audio_manager.play_one_shot_3d(test_pluck, sound_position);
		}
		// 3D SOUND DEMO

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
				ImGui::GetIO().Framerate);

		ImGui::End();

		if (display_manager.window_should_close()) {
			should_run = false;
		}

		if (physics_system_enabled) {
			physics_system->update(dt);
		}

		parent_system->update();
		render_system->update(render_manager);

		auto stop_time = std::chrono::high_resolution_clock::now();

		dt = std::chrono::duration<float, std::chrono::seconds::period>(stop_time - start_time).count();

		render_manager.draw();

		fmod_listener_system->update(dt);
		audio_manager.update();
	}

	// Shut everything down, in reverse order.
	SPDLOG_INFO("Shutting down engine subsystems...");
	audio_manager.shutdown();
	render_manager.shutdown();
	display_manager.shutdown();

	return 0;
}
