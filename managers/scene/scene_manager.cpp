#include "scene_manager.h"
#include "ecs/ecs_manager.h"
#include "serialization.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

extern ECSManager ecs_manager;

SceneManager::SceneManager() {
	class_map = serialization::IdToClassConstructor{};
}

void SceneManager::load_scene_from_json_file(const std::string &scene_name, std::vector<Entity> &entities) {
	std::ifstream file(scene_name);
	nlohmann::json scene_json = nlohmann::json::parse(file);
	file.close();
	SPDLOG_INFO("Loaded scene from file {}", scene_name);
	entities.clear();
	ecs_manager.deserialize_entities_json(scene_json, entities);
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
SceneManager *SceneManager::get_instance() {
	std::lock_guard<std::mutex> lock(mutex);
	if (instance == nullptr) {
		if (instance == nullptr) {
			instance = new SceneManager();
		}
	}
	return instance;
}

SceneManager *SceneManager::instance = nullptr;
serialization::IdToClassConstructor &SceneManager::get_class_map() {
	return get_instance()->class_map;
}
std::mutex SceneManager::mutex;
