#include "scene_manager.h"
#include "ecs/ecs_manager.h"
#include <core/components/children_component.h>
#include <nlohmann/json.hpp>

extern ECSManager ecs_manager;

void SceneManager::load_scene(std::string scene_name) {
}
std::string SceneManager::save_scene(std::string scene_name) {
	nlohmann::json scene_json;
}
