#include "scene_manager.h"
#include "ecs/ecs_manager.h"
#include <core/components/children_component.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

extern ECSManager ecs_manager;

void SceneManager::load_scene(const std::string &scene_name) {
}
nlohmann::json SceneManager::save_scene(const std::string &scene_name, const std::vector<Entity> &entities) {
	nlohmann::json scene_json = nlohmann::json::array();
	for (auto const &entity : entities) {
		scene_json.push_back(nlohmann::json::object());
		ecs_manager.serialize_entity_json(scene_json.back(), entity);
	}
	return scene_json;
}
void SceneManager::save_json_to_file(const std::string &file_name, const nlohmann::json &json) {
	std::ofstream file(file_name);
	file << json.dump();
	file.close();
}
