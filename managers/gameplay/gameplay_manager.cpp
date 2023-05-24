#include "gameplay_manager.h"
#include <engine/scene.h>
GameplayManager &GameplayManager::get() {
	static GameplayManager instance;
	return instance;
}

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

void GameplayManager::update() {
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
