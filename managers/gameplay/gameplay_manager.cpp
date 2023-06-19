#include "gameplay_manager.h"
#include <engine/scene.h>

AutoCVarFloat cv_enemy_near_player_radius(
		"gameplay.enemy_near_radius", "radius that checks for enemies near player", 15.0f, CVarFlags::EditCheckbox);

GameplayManager &GameplayManager::get() {
	static GameplayManager instance;
	return instance;
}

std::default_random_engine GameplayManager::random_generator;

void GameplayManager::startup(Scene *scene) {
	World &world = scene->world;
	for (auto &e : scene->entities) {
		if (world.has_component<AgentData>(e)) {
			agent_entity = e;
		}
		if (world.has_component<HackerData>(e)) {
			hacker_entity = e;
		}
	}
}

void GameplayManager::shutdown() {
}

void GameplayManager::update(World &world, float dt) {
	if (disabled) {
		return;
	}
	//world.get_parent_scene()->get_render_scene().debug_draw.draw_sphere(get_agent_position(world.get_parent_scene()),
	//cv_enemy_near_player_radius.get(), glm::vec3(1.0f, 1.0f, 0.0f), 32);
	// calculate highest detection level
	if (!detection_levels.empty()) {
		highest_detection = *std::max_element(detection_levels.begin(), detection_levels.end());
	} else {
		highest_detection = 0.0f;
	}

	detection_levels.clear();

	// get number of enemies near player
	enemies_near_player = 0;
	for (auto &entity : enemy_entities) {
		auto &player_transform = world.get_component<Transform>(agent_entity);
		auto &enemy_transform = world.get_component<Transform>(entity);
		;
		if (glm::distance(player_transform.position, enemy_transform.position) < cv_enemy_near_player_radius.get()) {
			enemies_near_player++;
		}
	}
	//SPDLOG_INFO("enemies near {}", enemies_near_player);
}

glm::vec3 GameplayManager::get_agent_position(Scene *scene) const {
	World &world = scene->world;
	auto &transform = world.get_component<Transform>(agent_entity);
	return transform.get_global_position();
}

glm::vec3 GameplayManager::get_hacker_position(Scene *scene) const {
	World &world = scene->world;
	auto &transform = world.get_component<Transform>(hacker_entity);
	return transform.get_global_position();
}

uint32_t GameplayManager::get_agent_entity() const {
	return agent_entity;
}

uint32_t GameplayManager::get_hacker_entity() const {
	return hacker_entity;
}

void GameplayManager::add_enemy_entity(uint32_t entity) {
	enemy_entities.push_back(entity);
}
void GameplayManager::add_detection_level(float detection_level) {
	detection_levels.push_back(detection_level);
}
void GameplayManager::enable() {
	disabled = false;
}

void GameplayManager::set_agent_crouch(bool crouching) {
	is_agent_crouching = crouching;
}

bool GameplayManager::get_agent_crouch() const {
	return is_agent_crouching;
}
float GameplayManager::get_highest_detection() const {
	return highest_detection;
}
uint32_t GameplayManager::get_enemies_near_player() const {
	return enemies_near_player;
}

uint32_t GameplayManager::get_agent_camera(Scene *scene) const {
	return scene->world.get_component<AgentData>(agent_entity).camera;
}

uint32_t GameplayManager::get_hacker_camera(Scene *scene) const {
	return scene->world.get_component<HackerData>(hacker_entity).camera;
}

void GameplayManager::set_agent_system(std::shared_ptr<AgentSystem> system) {
	agent_system = system;
}

void GameplayManager::set_hacker_system(std::shared_ptr<HackerSystem> system) {
	hacker_system = system;
}

std::shared_ptr<AgentSystem> GameplayManager::get_agent_system() {
	return agent_system;
}

std::shared_ptr<HackerSystem> GameplayManager::get_hacker_system() {
	return hacker_system;
}

