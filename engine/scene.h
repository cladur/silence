#ifndef SILENCE_SCENE_H
#define SILENCE_SCENE_H

#include "ecs/world.h"
#include "imgui.h"
#include "render/render_scene.h"

struct BSPNode;

struct Scene {
	std::string name;
	std::string path;
	bool is_prefab;

	uint32_t frame_number = 0;

	World world;

	std::vector<Entity> entities;

	uint32_t render_scene_idx;

	std::shared_ptr<BSPNode> bsp_tree;

	Scene();
	void register_game_systems();
	void register_main_systems();
	virtual void update(float dt);
	[[nodiscard]] RenderScene &get_render_scene() const;

	// Serialization
	virtual void save_to_file(const std::string &path);
	void load_from_file(const std::string &path);
};

#endif //SILENCE_SCENE_H