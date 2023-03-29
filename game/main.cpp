#include "display_manager.h"
#include "ecs/parent_manager.h"
#include "render_manager.h"

#include "components/gravity_component.h"
#include "components/rigidbody_component.h"
#include "components/state_component.h"
#include "components/transform_component.h"
#include "ecs/ecs_manager.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "systems/physics_system.h"

#include "ai/state_machine/states/test_state.h"
#include "systems/state_machine_system.h"

#include <random>

#include "components/children_component.h"
#include "components/parent_component.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "systems/parent_system.h"
#include "types.h"

RenderManager render_manager;
DisplayManager display_manager;
ECSManager ecs_manager;

void default_ecs_manager_init() {
	ecs_manager.startup();

	ecs_manager.register_component<Transform>();
	ecs_manager.register_component<RigidBody>();
	ecs_manager.register_component<Gravity>();
	ecs_manager.register_component<Parent>();
	ecs_manager.register_component<Children>();
	ecs_manager.register_component<State>();
}

std::shared_ptr<PhysicsSystem> default_physics_system_init() {
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

std::shared_ptr<ParentSystem> default_parent_system_init() {
	auto parent_system = ecs_manager.register_system<ParentSystem>();

	parent_system->startup();

	return parent_system;
}

std::shared_ptr<StateMachineSystem> default_state_system_init() {
	auto state_machine_system = ecs_manager.register_system<StateMachineSystem>();

	Signature signature;
	signature.set(ecs_manager.get_component_type<State>());
	signature.set(ecs_manager.get_component_type<RigidBody>());
	ecs_manager.set_system_component_whitelist<StateMachineSystem>(signature);

	state_machine_system->startup();

	return state_machine_system;
}

void demo_entities_init(std::vector<Entity> entities) {
	std::default_random_engine random_generator; // NOLINT(cert-msc51-cpp)
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
						.euler_rot = glm::vec3(rand_rotation(random_generator), rand_rotation(random_generator),
								rand_rotation(random_generator)),
						.scale = glm::vec3(scale, scale, scale) });
		ecs_manager.add_component(entity, State{ .state = new TestState(std::string("idle")) });
	}
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

void setup_im_gui_style() {
	// Classic Steam style by metasprite from ImThemes
	ImGuiStyle &style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6000000238418579f;
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(4.0f, 3.0f);
	style.FrameRounding = 0.0f;
	style.FrameBorderSize = 1.0f;
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 14.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.TabBorderSize = 0.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2862745225429535f, 0.3372549116611481f, 0.2588235437870026f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.2862745225429535f, 0.3372549116611481f, 0.2588235437870026f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.239215686917305f, 0.2666666805744171f, 0.2000000029802322f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 0.5f);
	style.Colors[ImGuiCol_BorderShadow] =
			ImVec4(0.1372549086809158f, 0.1568627506494522f, 0.1098039224743843f, 0.5199999809265137f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.239215686917305f, 0.2666666805744171f, 0.2000000029802322f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2666666805744171f, 0.2980392277240753f, 0.2274509817361832f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2980392277240753f, 0.3372549116611481f, 0.2588235437870026f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.239215686917305f, 0.2666666805744171f, 0.2000000029802322f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2862745225429535f, 0.3372549116611481f, 0.2588235437870026f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.5099999904632568f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.239215686917305f, 0.2666666805744171f, 0.2000000029802322f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2784313857555389f, 0.3176470696926117f, 0.239215686917305f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] =
			ImVec4(0.2470588237047195f, 0.2980392277240753f, 0.2196078449487686f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] =
			ImVec4(0.2274509817361832f, 0.2666666805744171f, 0.2078431397676468f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] =
			ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 0.5f);
	style.Colors[ImGuiCol_Button] =
			ImVec4(0.2862745225429535f, 0.3372549116611481f, 0.2588235437870026f, 0.4000000059604645f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 0.5f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] =
			ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 0.6000000238418579f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 0.5f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.1372549086809158f, 0.1568627506494522f, 0.1098039224743843f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] =
			ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] =
			ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1882352977991104f, 0.2274509817361832f, 0.1764705926179886f, 0.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] =
			ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] =
			ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] =
			ImVec4(0.5372549295425415f, 0.5686274766921997f, 0.5098039507865906f, 0.7799999713897705f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.239215686917305f, 0.2666666805744171f, 0.2000000029802322f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] =
			ImVec4(0.3490196168422699f, 0.4196078479290009f, 0.3098039329051971f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] =
			ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.7764706015586853f, 0.2784313857555389f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6000000238418579f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] =
			ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] =
			ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.729411780834198f, 0.6666666865348816f, 0.239215686917305f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.5882353186607361f, 0.5372549295425415f, 0.1764705926179886f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
	style.Colors[ImGuiCol_NavWindowingDimBg] =
			ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
	style.Colors[ImGuiCol_ModalWindowDimBg] =
			ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
}

int main() {
	SPDLOG_INFO("Starting up engine systems...");

	if (!display_manager_init() || !render_manager_init()) {
		return -1;
	}

	setup_im_gui_style();

	// ECS ----------------------------------------

	default_ecs_manager_init();
	auto physics_system = default_physics_system_init();
	auto parent_system = default_parent_system_init();
	auto state_system = default_state_system_init();

	std::vector<Entity> entities(50);
	demo_entities_init(entities);

	// ECS -----------------------------------------

	// Run the game.
	float dt{};
	bool show_ecs_logs = false;
	bool show_demo_window = false;
	int imgui_children_id = 1;
	int imgui_entity_id = 1;

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

		if (show_demo_window) {
			ImGui::ShowDemoWindow();
		}

		ImGui::DragInt("Entity Id", &imgui_entity_id, 1, 1, MAX_IMGUI_ENTITIES);
		ImGui::DragInt("Children Id", &imgui_children_id, 1, 1, MAX_IMGUI_ENTITIES);

		if (ImGui::Button("Remove children")) {
			ParentManager::remove_children(imgui_entity_id, imgui_children_id);
		}

		if (ImGui::Button("Add children")) {
			ParentManager::add_children(imgui_entity_id, imgui_children_id);
		}

		if (ImGui::Button("List children")) {
			parent_system->update();
		}

		ImGui::End();

		if (display_manager.window_should_close()) {
			should_run = false;
		}

		physics_system->update(dt);
		state_system->update(dt);

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
