#include "scene_manager.h"
#include "ecs/ecs_manager.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

extern ECSManager ecs_manager;

void SceneManager::load_scene_from_json_file(const std::string &scene_name) {
	std::ifstream file(scene_name);
	nlohmann::json scene_json = nlohmann::json::parse(file);
	file.close();
	SPDLOG_INFO("Loaded scene from file {}", scene_name);
	ecs_manager.deserialize_entities_json(scene_json);
}
nlohmann::json SceneManager::save_scene(const std::vector<Entity> &entities) {
	nlohmann::json scene_json = nlohmann::json::array();
	for (auto const &entity : entities) {
		scene_json.push_back(nlohmann::json::object());
		ecs_manager.serialize_entity_json(scene_json.back(), entity);
	}
	SPDLOG_INFO("Saved scene with {} entities", entities.size());
	return scene_json;
}
void SceneManager::save_json_to_file(const std::string &file_name, const nlohmann::json &json) {
	std::ofstream file(file_name);
	file << json.dump();
	file.close();
	SPDLOG_INFO("Saved scene to file {}", file_name);
}

void SceneManager::show_map() {
	for (auto &[name, component] : class_map) {
		SPDLOG_INFO("{}", name);
	}
}
