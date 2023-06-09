#include "engine.h"

#include "assets/material_asset.h"
#include "assets/model_asset.h"
#include "audio/audio_manager.h"
#include "display/display_manager.h"
#include "font/font_manager.h"
#include "input/input_manager.h"
#include "render/render_manager.h"

#include "audio/adaptive_music_manager.h"
#include "gameplay/gameplay_manager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "physics/physics_manager.h"
#include "render/transparent_elements/particle_manager.h"
#include "render/transparent_elements/ui_manager.h"
#include <string>

void Engine::startup() {
	// Managers
	SPDLOG_INFO("Starting up engine systems...");
	DisplayManager::get().startup("Silence Engine", true);
	InputManager::get().startup();
	RenderManager::get().startup();
	FontManager::get().startup();
	AudioManager::get().startup();
	AdaptiveMusicManager::get().startup("AdaptiveMusic/Music_1");
	UIManager::get();
	ParticleManager::get().startup();

	FontManager::get().load_font("resources/fonts/PoltawskiNowy.ttf", 48, "PoltawskiNowy");
}

void Engine::shutdown() {
	// Managers
	SPDLOG_INFO("Shutting down engine systems...");
	AdaptiveMusicManager::get().shutdown();
	AudioManager::get().shutdown();
	FontManager::get().shutdown();
	RenderManager::get().shutdown();
	InputManager::get().shutdown();
	DisplayManager::get().shutdown();
	ParticleManager::get().shutdown();
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

		{
			ZoneScopedNC("Sleep", tracy::Color::Blue);
			while (std::chrono::duration<float, std::chrono::seconds::period>(stop_time - start_time).count() <
					target_frame_time) {
				stop_time = std::chrono::high_resolution_clock::now();
			}
		}

		dt = std::chrono::duration<float, std::chrono::seconds::period>(stop_time - start_time).count();

		FrameMark;
	}
}

void Engine::update(float dt) {
	InputManager &input_manager = InputManager::get();
	DisplayManager &display_manager = DisplayManager::get();
	RenderManager &render_manager = RenderManager::get();

	AdaptiveMusicManager::get().update(dt);

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
		scene->world.update(dt);
		scene->update(dt);
	}

	ParticleManager::get().update(dt);

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
	GameplayManager::get().startup(&get_active_scene()); // dunno where to put this honestly.
}

Scene &Engine::get_active_scene() {
	return *scenes[active_scene];
}
