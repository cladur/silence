#include "engine.h"

#include "audio/audio_manager.h"
#include "display/display_manager.h"
#include "font/font_manager.h"
#include "input/input_manager.h"
#include "render/render_manager.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void Engine::startup() {
	// Managers
	SPDLOG_INFO("Starting up engine systems...");
	DisplayManager::get().startup("Silence Engine", true);
	InputManager::get().startup();
	RenderManager::get().startup();
	FontManager::get().startup();
	AudioManager::get().startup();

	FontManager::get().load_font("resources/fonts/PoltawskiNowy.ttf", 48, "PoltawskiNowy");
}

void Engine::shutdown() {
	// Managers
	SPDLOG_INFO("Shutting down engine systems...");
	AudioManager::get().shutdown();
	FontManager::get().shutdown();
	RenderManager::get().shutdown();
	InputManager::get().shutdown();
	DisplayManager::get().shutdown();
}

void Engine::run() {
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

void Engine::update(float dt) {
	InputManager &input_manager = InputManager::get();
	DisplayManager &display_manager = DisplayManager::get();
	RenderManager &render_manager = RenderManager::get();

	if (display_manager.window_should_close()) {
		should_run = false;
	}

	// Process input
	DisplayManager::get().poll_events();

	// ImGui new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (show_cvar_editor) {
		CVarSystem::get()->draw_imgui_editor();
	}

	// Update

	custom_update(dt);

	for (auto &scene : scenes) {
		scene->update(dt);

		scene->world.update(dt);
	}

	// scene.world.pre_animation_update(dt);
	// AnimationManager::get().update_animations(dt);
	// scene.world.post_animation_update(dt);
	// PhysicsManager::get()->step(dt);
	// AnimationManager::get().update_ragdolls(dt);
	// scene.world.post_physics_update(dt);
	// AnimationManager::get().finalize();
	// scene.world.pre_render_update(dt);

	AudioManager::get().update();

	input_manager.process_input();

	render_manager.draw();
}

void Engine::create_scene(const std::string &name) {
	auto scene = std::make_unique<Scene>();
	scene->name = name;

	// Create RenderScene for scene
	RenderManager &render_manager = RenderManager::get();
	scene->render_scene_idx = render_manager.create_render_scene();

	scenes.push_back(std::move(scene));
}

uint32_t Engine::get_scene_index(const std::string &name) {
	for (int i = 0; i < scenes.size(); i++) {
		if (scenes[i]->name == name) {
			return i;
		}
	}
	return 0;
}

void Engine::set_active_scene(const std::string &name) {
	active_scene = get_scene_index(name);
	RenderManager::get().displayed_scene = scenes[active_scene]->render_scene_idx;
}

Scene &Engine::get_active_scene() {
	return *scenes[active_scene];
}
