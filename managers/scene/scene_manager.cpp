#include "scene_manager.h"
#include "ecs/world.h"
#include "serialization.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

void SceneManager::load_scene_from_json_file(
		World &world, nlohmann::json &scene_json, const std::string &scene_name, std::vector<Entity> &entities) {
	entities.clear();
	world.deserialize_entities_json(scene_json, entities);
}
nlohmann::json SceneManager::save_scene(World &world, const std::vector<Entity> &entities) {
	nlohmann::json scene_json = nlohmann::json::array();
	for (auto const &entity : entities) {
		scene_json.push_back(nlohmann::json::object());
		world.serialize_entity_json(scene_json.back(), entity);
	}
	SPDLOG_INFO("Saved scene with {} entities", entities.size());
	return scene_json;
}
void SceneManager::save_json_to_file(const std::string &file_name, const nlohmann::json &json) {
	// check if file exists
	std::filesystem::path path(file_name);
	if (std::filesystem::exists(path)) {
		std::filesystem::remove(path);
	}
	std::ofstream file(file_name);
	file << json.dump(4);
	file.close();
	SPDLOG_INFO("Saved scene to file {}", file_name);
}
SceneManager &SceneManager::get() {
	std::lock_guard<std::mutex> lock(mutex);
	static SceneManager instance;
	return instance;
}

SceneManager *SceneManager::instance = nullptr;
serialization::IdToClassConstructor &SceneManager::get_class_map() {
	return get().class_map;
}
std::mutex SceneManager::mutex;
