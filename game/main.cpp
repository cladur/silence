#include "display_manager.h"
#include "ecs/parent_manager.h"
#include "render_manager.h"

#include "components/gravity_component.h"
#include "components/rigidbody_component.h"
#include "components/transform_component.h"
#include "ecs/ecs_manager.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "systems/physics_system.h"

#include <random>

#include "components/children_component.h"
#include "components/parent_component.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "state_machine/state_machine.h"
#include "state_machine/states/test_state.h"
#include "systems/parent_system.h"
#include "types.h"

RenderManager render_manager;
DisplayManager display_manager;
ECSManager ecs_manager;

void default_ecs_manager_init()
{
	ecs_manager.startup();

	ecs_manager.register_component<Transform>();
	ecs_manager.register_component<RigidBody>();
	ecs_manager.register_component<Gravity>();
	ecs_manager.register_component<Parent>();
	ecs_manager.register_component<Children>();
}

std::shared_ptr<PhysicsSystem> default_physics_system_init()
{
	auto physics_system = ecs_manager.register_system<PhysicsSystem>();

	Signature signature;
	signature.set(ecs_manager.get_component_type<Gravity>());
	signature.set(ecs_manager.get_component_type<RigidBody>());
	signature.set(ecs_manager.get_component_type<Transform>());
	ecs_manager.set_system_component_whitelist<PhysicsSystem>(signature);
	signature.reset();
	signature.set(ecs_manager.get_component_type<Gravity>());
	ecs_manager.set_system_component_blacklist<PhysicsSystem>(signature);

	physics_system->startup();

	return physics_system;
}

std::shared_ptr<ParentSystem> default_parent_system_init()
{
	auto parent_system = ecs_manager.register_system<ParentSystem>();

	parent_system->startup();

	return parent_system;
}

std::shared_ptr<StateMachineSystem> default_state_system_init()
{
	auto state_machine_system = ecs_manager.register_system<StateMachineSystem>();

	Signature signature;
	signature.set(ecs_manager.get_component_type<State>());
	signature.set(ecs_manager.get_component_type<RigidBody>());
	ecs_manager.set_system_component_whitelist<StateMachineSystem>(signature);

	state_machine_system->startup();

	return state_machine_system;
}

void demo_entities_init(std::vector<Entity> entities)
{
	std::default_random_engine random_generator; // NOLINT(cert-msc51-cpp)
	std::uniform_real_distribution<float> rand_position(-100.0f, 100.0f);
	std::uniform_real_distribution<float> rand_rotation(0.0f, 3.0f);
	std::uniform_real_distribution<float> rand_scale(3.0f, 5.0f);
	std::uniform_real_distribution<float> rand_color(0.0f, 1.0f);
	std::uniform_real_distribution<float> rand_gravity(-10.0f, -1.0f);

	float scale = rand_scale(random_generator);

	for (auto& entity : entities)
	{
		entity = ecs_manager.create_entity();

		ecs_manager.add_component<Gravity>(entity, { glm::vec3(0.0f, rand_gravity(random_generator), 0.0f) });

		ecs_manager.add_component(entity,
			RigidBody { .velocity = glm::vec3(0.0f, 0.0f, 0.0f), .acceleration = glm::vec3(0.0f, 0.0f, 0.0f) });

		ecs_manager.add_component(entity,
			Transform { glm::vec3(rand_position(random_generator), rand_position(random_generator),
							   rand_position(random_generator)),
					glm::vec3(rand_rotation(random_generator), rand_rotation(random_generator),
							rand_rotation(random_generator)),
					glm::vec3(scale, scale, scale) });
		ecs_manager.add_component(entity, State { .state = new TestState(std::string("idle")) });
	}
}

bool display_manager_init()
{
	auto display_manager_result = display_manager.startup();
	if (display_manager_result == DisplayManager::Status::Ok)
	{
		SPDLOG_INFO("Initialized display manager");
	}
	else
	{
		SPDLOG_ERROR("Failed to initialize the display manager. Status: ({}) {}",
			magic_enum::enum_integer(display_manager_result), magic_enum::enum_name(display_manager_result));
		return false;
	}

	return true;
}

bool render_manager_init()
{
	auto render_manager_result = render_manager.startup(display_manager);
	if (render_manager_result == RenderManager::Status::Ok)
	{
		SPDLOG_INFO("Initialized render manager");
	}
	else
	{
		SPDLOG_ERROR("Failed to initialize the render manager. Status: ({}) {}",
			magic_enum::enum_integer(render_manager_result), magic_enum::enum_name(render_manager_result));
		return false;
	}

	return true;
}

void setup_imgui_style()
{
	// zajebisty styl wulkanowy czerwony 😎
	float hue_shift = 0.398f;
	float saturation_shift = 0.05f;
	static ImGuiStyle base_style = ImGui::GetStyle();
	ImGuiStyle& style = ImGui::GetStyle();
	for (int i = 0; i < ImGuiCol_COUNT; i++)
	{
		ImVec4& base_col = base_style.Colors[ i ];
		float hue, saturation, value;
		ImGui::ColorConvertRGBtoHSV(base_col.x, base_col.y, base_col.z, hue, saturation, value);
		hue += hue_shift;
		if (hue > 1.0f)
		{
			hue -= 1.0f;
		}
		saturation += saturation_shift;
		if (saturation > 1.0f)
		{
			saturation = 1.0f;
		}
		ImVec4& target_col = style.Colors[ i ];
		ImGui::ColorConvertHSVtoRGB(hue, saturation, value, target_col.x, target_col.y, target_col.z);
	}
}

int main()
{
	SPDLOG_INFO("Starting up engine systems...");

	if (!display_manager_init() || !render_manager_init())
	{
		return -1;
	}

	setup_imgui_style();

	// ECS ----------------------------------------

	default_ecs_manager_init();
	auto physics_system = default_physics_system_init();
	auto parent_system = default_parent_system_init();
	auto state_system = default_state_system_init();

	std::vector<Entity> entities(50);
	demo_entities_init(entities);

	// ECS -----------------------------------------

	StateMachine machine = StateMachine();
	TestState state_1 = TestState("one");
	TestState state_2 = TestState("two");
	TestState state_3 = TestState("three");

	machine.add_state(&state_1);
	machine.add_state(&state_2);
	machine.add_state(&state_3);

	machine.set_state("two");
	machine.set_state("three");

	// Run the game.
	float dt {};
	bool show_ecs_logs = false;
	bool show_demo_window = false;
	int imgui_children_id = 1;
	int imgui_entity_id = 1;

	bool should_run = true;
	while (should_run)
	{
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

		if (show_demo_window)
		{
			ImGui::ShowDemoWindow();
		}

		ImGui::DragInt("Entity Id", &imgui_entity_id, 1, 1, MAX_IMGUI_ENTITIES);
		ImGui::DragInt("Children Id", &imgui_children_id, 1, 1, MAX_IMGUI_ENTITIES);

		if (ImGui::Button("Add child"))
		{
			ParentManager::add_children(imgui_entity_id, imgui_children_id);
		}

		if (ImGui::Button("Remove child"))
		{
			ParentManager::remove_children(imgui_entity_id, imgui_children_id);
		}

		//		if (ImGui::Button("Parent system update")) {
		//
		//		}

		ImGui::End();

		if (display_manager.window_should_close())
		{
			should_run = false;
		}

		physics_system->update(dt);
		state_system->update(dt);
		parent_system->update();

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
