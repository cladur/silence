#ifndef SILENCE_EDITOR_SCENE_H
#define SILENCE_EDITOR_SCENE_H

#include "engine/scene.h"

enum SceneType { GameScene, Prefab };

struct EditorScene : public Scene {
	SceneType type;
	bool is_visible = false;
	bool viewport_hovered = false;
	bool controlling_camera = false;
	ImVec2 last_viewport_size = ImVec2(0, 0);

	explicit EditorScene(SceneType type = SceneType::GameScene);
	void update(float dt) override;

	void save_to_file(const std::string &path) override;

	// Selection
	std::vector<Entity> entities_selected;
	Entity last_entity_selected = 0;
	Entity multi_select_parent = 0;
	Transform dummy_transform;
	// Used for multi-select
	std::unordered_map<Entity, Entity> child_to_parent;
	std::vector<std::pair<Entity, Entity>> reparent_queue;
	std::vector<std::pair<Entity, Entity>> add_child_queue;

	// Selection
	void add_to_selection(Entity entity);
	void remove_from_selection(Entity entity);
	void clear_selection();
	void calculate_multi_select_parent();
	void execute_reparent_queue();
};

#endif //SILENCE_EDITOR_SCENE_H