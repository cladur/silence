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
	Entity selected_entity = 0;
	// Deletion queue
	std::queue<Entity> entity_deletion_queue;
};

#endif //SILENCE_EDITOR_SCENE_H