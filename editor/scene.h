#ifndef SILENCE_SCENE_H
#define SILENCE_SCENE_H

#include "camera/camera.h"
#include "ecs/world.h"
#include "imgui.h"
#include "render/render_scene.h"

struct Scene {
	std::string name;
	std::string path;
	bool is_prefab;

	World world;

	bool is_visible = false;
	bool viewport_hovered = false;
	bool controlling_camera = false;
	ImVec2 last_viewport_size = ImVec2(0, 0);

	// Selection
	std::vector<Entity> entities_selected;
	Entity last_entity_selected = 0;
	Entity multi_select_parent = 0;
	Transform dummy_transform;
	// Used for multi-select
	std::unordered_map<Entity, Entity> child_to_parent;
	std::vector<std::pair<Entity, Entity>> reparent_queue;
	std::vector<std::pair<Entity, Entity>> add_child_queue;

	Camera camera;

	std::vector<Entity> entities;

	uint32_t render_scene_idx;

	Scene();
	void update(float dt);
	RenderScene &get_render_scene();

	// Serialization
	void save_to_file(const std::string &path = "");
	void load_from_file(const std::string &path);

	// Selection
	void add_to_selection(Entity entity);
	void remove_from_selection(Entity entity);
	void clear_selection();
	void calculate_multi_select_parent();
	void execute_reparent_queue();
};

#endif //SILENCE_SCENE_H