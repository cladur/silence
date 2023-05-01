#ifndef SILENCE_EDITOR_H
#define SILENCE_EDITOR_H

#include "audio/audio_manager.h"
#include "display/display_manager.h"
#include "font/font_manager.h"
#include "input/input_manager.h"
#include "managers/render/common/material.h"
#include <string>

#include "imgui_impl_opengl3.h"
#include "managers/render/ecs/render_system.h"
#include "render/render_manager.h"

#include "components/children_component.h"
#include "components/collider_aabb.h"
#include "components/collider_obb.h"
#include "components/collider_sphere.h"
#include "components/gravity_component.h"
#include "components/name_component.h"
#include "components/parent_component.h"
#include "components/rigidbody_component.h"
#include "components/transform_component.h"

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

#include <ImGuizmo.h>
#include <nfd.h>
#include <glm/gtc/type_ptr.hpp>

#include "scene.h"

class Editor {
public:
	// Systems
	std::shared_ptr<PhysicsSystem> physics_system;
	std::shared_ptr<CollisionSystem> collision_system;
	std::shared_ptr<ParentSystem> parent_system;
	std::shared_ptr<RenderSystem> render_system;

	// Viewport
	bool controlling_camera = false;
	bool viewport_hovered = false;

	// Gizmos
	ImGuizmo::OPERATION current_gizmo_operation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE current_gizmo_mode = ImGuizmo::WORLD;

	// Misc
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
	bool show_cvar_editor = false;
	bool show_demo_window = false;
	bool should_run = true;

	// Scenes
	std::vector<Scene> scenes;
	uint32_t active_scene = 0;

	static Editor *get();
	void startup();
	void shutdown();
	void run();
	void update(float dt);

	// GUI
	void imgui_menu_bar();
	void imgui_inspector(Scene &scene);
	void imgui_scene(Scene &scene);
	void imgui_viewport(Scene &scene);
	void imgui_resources();
	void imgui_settings();

	void create_scene(const std::string &name);
	uint32_t get_scene_index(const std::string &name);
};

#endif //SILENCE_EDITOR_H