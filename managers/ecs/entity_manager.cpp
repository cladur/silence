#include "entity_manager.h"
#include <cassert>

bool EntityManager::startup() {
	for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) {
		available_entities.push(entity);
	}
}

void EntityManager::shutdown() {
}

Entity EntityManager::createEntity() {
	assert(living_entities_count < MAX_ENTITIES && "Too many entities alive");

	// Take id of first entity and then remove it from the queue
	Entity createdEntity = available_entities.front();
	available_entities.pop();
	living_entities_count++;

	// Return found id
	return createdEntity;
}

void EntityManager::destroyEntity(Entity entity) {
	assert(entity < MAX_ENTITIES && "Entity bigger than max value");

	// Reset signature of entity
	signatures[entity].reset();

	// Push it back to queue
	available_entities.push(entity);
	living_entities_count--;
}

void EntityManager::setSignature(Entity entity, Signature signature) {
	assert(entity < MAX_ENTITIES && "Entity bigger than max value");

	// Set signature for entity (signature = bitset representing which components are on gameObject)
	signatures[entity] = signature;
}

Signature EntityManager::getSignature(Entity entity) {
	assert(entity < MAX_ENTITIES && "Entity bigger than max value");
	
	return signatures[entity];
}
