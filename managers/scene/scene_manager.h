#ifndef SILENCE_SCENEMANAGER_H
#define SILENCE_SCENEMANAGER_H

#include "core/serialization.h"

class SceneManager {
private:
	map_type class_map;

public:
	template <typename T> void add_component_to_map(std::string name) {
		class_map[name] = &create_instance<T>;
	}

	void show_map();
	void load_scene_from_json_file(const std::string &scene_name);
	static nlohmann::json save_scene(const std::vector<Entity> &entities);
	static void save_json_to_file(const std::string &file_name, const nlohmann::json &json);
};

#endif //SILENCE_SCENEMANAGER_H
