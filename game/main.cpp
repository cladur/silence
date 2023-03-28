#include "display_manager.h"
#include "render_manager.h"

#include "components/gravity_component.h"
#include "components/rigidbody_component.h"
#include "components/transform_component.h"
#include "ecs/ecs_manager.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "systems/physics_system.h"

#include <random>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

RenderManager render_manager;
DisplayManager display_manager;
ECSManager ecs_manager;

int main() {
	SPDLOG_INFO("Starting up engine systems...");

	auto dm_ret = display_manager.startup();
	if (dm_ret == DisplayManager::Status::Ok) {
		SPDLOG_INFO("Initialized display manager");
	} else {
		SPDLOG_ERROR("Failed to initialize the display manager. Status: ({}) {}", magic_enum::enum_integer(dm_ret),
				magic_enum::enum_name(dm_ret));
		return -1;
	}

	auto rm_ret = render_manager.startup(display_manager);
	if (rm_ret == RenderManager::Status::Ok) {
		SPDLOG_INFO("Initialized render manager");
	} else {
		SPDLOG_ERROR("Failed to initialize the render manager. Status: ({}) {}", magic_enum::enum_integer(rm_ret),
				magic_enum::enum_name(rm_ret));
		return -1;
	}

	// ECS DEMO ----------------------------------------
	ecs_manager.startup();

	ecs_manager.register_component<Transform>();
	ecs_manager.register_component<RigidBody>();
	ecs_manager.register_component<Gravity>();

	auto physics_system = ecs_manager.register_system<PhysicsSystem>();
	{
		Signature signature;
		signature.set(ecs_manager.get_component_type<Gravity>());
		signature.set(ecs_manager.get_component_type<RigidBody>());
		signature.set(ecs_manager.get_component_type<Transform>());
		ecs_manager.set_system_component_whitelist<PhysicsSystem>(signature);
		signature.reset();
		signature.set(ecs_manager.get_component_type<Gravity>());
		ecs_manager.set_system_component_blacklist<PhysicsSystem>(signature);
	}

	physics_system->startup();

	std::vector<Entity> entities(MAX_ENTITIES - 1);

	std::default_random_engine random_generator;
	std::uniform_real_distribution<float> rand_position(-100.0f, 100.0f);
	std::uniform_real_distribution<float> rand_rotation(0.0f, 3.0f);
	std::uniform_real_distribution<float> rand_scale(3.0f, 5.0f);
	std::uniform_real_distribution<float> rand_color(0.0f, 1.0f);
	std::uniform_real_distribution<float> rand_gravity(-10.0f, -1.0f);

	float scale = rand_scale(random_generator);

	for (auto &entity : entities) {
		entity = ecs_manager.create_entity();

		ecs_manager.add_component<Gravity>(entity, { glm::vec3(0.0f, rand_gravity(random_generator), 0.0f) });

		ecs_manager.add_component(entity,
				RigidBody{ .velocity = glm::vec3(0.0f, 0.0f, 0.0f), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f) });

		ecs_manager.add_component(entity,
				Transform{ .position = glm::vec3(rand_position(random_generator), rand_position(random_generator),
								   rand_position(random_generator)),
						.rotation = glm::vec3(rand_rotation(random_generator), rand_rotation(random_generator),
								rand_rotation(random_generator)),
						.scale = glm::vec3(scale, scale, scale) });
	}

	float dt{};

	// ECS DEMO -----------------------------------------

	// Run the game.
	bool should_run = true;
	while (should_run) {
		// GAME LOGIC

		auto start_time = std::chrono::high_resolution_clock::now();

		display_manager.poll_events();

		//imgui new frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		//imgui commands
		ImGui::ShowDemoWindow();

		if (display_manager.window_should_close()) {
			should_run = false;
		}

		physics_system->update(dt);

		SPDLOG_INFO("y position: {}", ecs_manager.get_component<Transform>(7).position.y);

		auto stop_time = std::chrono::high_resolution_clock::now();

		dt = std::chrono::duration<float, std::chrono::seconds::period>(stop_time - start_time).count();

		render_manager.draw();
	}

	// Shut everything down, in reverse order.
	SPDLOG_INFO("Shutting down engine subsystems...");
	render_manager.shutdown();
	display_manager.shutdown();

	return 0;
}
