#ifndef SILENCE_EDITOR_H
#define SILENCE_EDITOR_H

#include "engine/engine.h"
#include "engine/scene.h"

#include "editor_scene.h"
#include "inspector_gui.h"

#include <ImGuizmo.h>
#include <nfd.h>

class Editor : public Engine {
public:
	Inspector inspector;

	// Viewport
	bool controlling_camera = false;
	bool viewport_hovered = false;

	// Collision Layers
	std::string selected_layer = "default";

	// Drag and drop
	std::string drag_and_drop_path;

	// Gizmos
	ImGuizmo::OPERATION current_gizmo_operation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE current_gizmo_mode = ImGuizmo::WORLD;
	bool use_snapping = false;
	float translation_snap = 1.0f;
	float rotation_snap = 15.0f;
	float scale_snap = 10.0f;

	// Misc
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
	bool show_cvar_editor = false;
	bool show_demo_window = false;
	bool should_run = true;

	// Scenes
	uint32_t scene_to_delete = 0;
	bool scene_deletion_queued = false;

	void create_scene(const std::string &name) override;
	void create_scene(const std::string &name, SceneType type, const std::string &path = "");
	EditorScene &get_editor_scene(uint32_t index);
	EditorScene &get_active_scene();

	// Content Browser
	std::string content_browser_current_path = "resources";
	uint32_t file_texture;
	uint32_t folder_texture;

	static Editor *get();
	void startup() override;
	void shutdown() override;
	void custom_update(float dt) override;

	// GUI
	void imgui_menu_bar();
	void imgui_inspector(EditorScene &scene);
	void display_entity(EditorScene &scene, Entity entity, const std::string &name);
	void imgui_scene(EditorScene &scene);
	void imgui_viewport(EditorScene &scene, uint32_t scene_index);
	void display_folder(const std::string &path);
	void imgui_content_browser();
	void imgui_settings();
	void imgui_layers_settings();
	void remove_entity_and_children(World &world, EditorScene &editor_scene, Entity entity);
};

#endif //SILENCE_EDITOR_H