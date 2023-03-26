#include "entity_manager.h"
#include <cassert>

void EntityManager::startup() {
	for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) {
		available_entities.push(entity);
	}
}

void EntityManager::shutdown() {
}

Entity EntityManager::create_entity() {
	assert(living_entities_count < MAX_ENTITIES && "Too many entities alive");

	// Take id of first entity and then remove it from the queue
	Entity created_entity = available_entities.front();
	available_entities.pop();
	living_entities_count++;

	// Return found id
	return created_entity;
}

void EntityManager::destroy_entity(Entity entity) {
	assert(entity < MAX_ENTITIES && "Entity bigger than max value");

	// Reset signature of entity
	signatures[entity].reset();

	// Push it back to queue
	available_entities.push(entity);
	living_entities_count--;
}

void EntityManager::set_signature(Entity entity, Signature signature) {
	assert(entity < MAX_ENTITIES && "Entity bigger than max value");

	// Set signature for entity (signature = bitset representing which components are on gameObject or which components
	// system needs)
	signatures[entity] = signature;
}

Signature EntityManager::get_signature(Entity entity) {
	assert(entity < MAX_ENTITIES && "Entity bigger than max value");

	return signatures[entity];
}
