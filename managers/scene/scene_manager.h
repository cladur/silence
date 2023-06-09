#ifndef SILENCE_SCENEMANAGER_H
#define SILENCE_SCENEMANAGER_H

#include "core/serialization.h"

class World;

class SceneManager {
private:
	static SceneManager *instance;
	static std::mutex mutex;
	serialization::IdToClassConstructor class_map;

public:
	static SceneManager &get();

	// Serialization
	static void load_scene_from_json_file(
			World &world, nlohmann::json &scene_json, const std::string &scene_name, std::vector<Entity> &entities);
	static nlohmann::json save_scene(World &world, const std::vector<Entity> &entities);
	static void save_json_to_file(const std::string &file_name, const nlohmann::json &json);
	static serialization::IdToClassConstructor &get_class_map();

	template <typename T> static void add_component_to_map(ComponentType id) {
		get().class_map[id] = &serialization::create_instance<T>;
	}
};

#endif //SILENCE_SCENEMANAGER_H
