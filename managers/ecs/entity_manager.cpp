#include "entity_manager.h"
#include <algorithm>
#include <glm/common.hpp>

void EntityManager::startup() {
	for (Entity entity = 1; entity < MAX_ENTITIES; ++entity) {
		available_entities.push_back(entity);
	}
}

void EntityManager::shutdown() {
}

Entity EntityManager::create_entity() {
	if (living_entities_count > MAX_ENTITIES) {
		SPDLOG_WARN("Too many entities alive ({}).", living_entities_count);
	}

	// Take id of first entity and then remove it from the queue
	Entity created_entity = available_entities.front();
	available_entities.erase(available_entities.begin());
	living_entities_count++;

	// Return found id
	return created_entity;
}

Entity EntityManager::create_entity(Entity entity) {
	assert(living_entities_count < MAX_ENTITIES && "Too many entities alive");

	// If entity is in available entities, remove it from there
	if (std::find(available_entities.begin(), available_entities.end(), entity) != available_entities.end()) {
		available_entities.erase(
				std::remove(available_entities.begin(), available_entities.end(), entity), available_entities.end());
	}

	living_entities_count++;
	return entity;
}

void EntityManager::destroy_entity(Entity entity) {
	assert(entity < MAX_ENTITIES && "Entity bigger than max value");

	// Reset signature of entity
	signatures[entity].reset();

	// Push it back to queue
	available_entities.push_back(entity);
	if (living_entities_count > 0) {
		living_entities_count--;
	}
}

void EntityManager::set_entity_signature(Entity entity, Signature signature) {
	assert(entity < MAX_ENTITIES && "Entity bigger than max value");

	// Set signature for entity (signature = bitset representing which components are on gameObject or which components
	// system needs)
	signatures[entity] = signature;
}

Signature EntityManager::get_signature(Entity entity) {
	assert(entity < MAX_ENTITIES && "Entity bigger than max value");

	return signatures[entity];
}
