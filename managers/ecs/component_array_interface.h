#include "../../core/types.h"
#include <cassert>
#include <unordered_map>
#ifndef SILENCE_COMPONENTARRAYINTERFACE_H
#define SILENCE_COMPONENTARRAYINTERFACE_H

#endif //SILENCE_COMPONENTARRAYINTERFACE_H

// An interface is needed so that the ComponentManager (seen later)
// can tell a generic ComponentArray that an entity has been destroyed
// and that it needs to update its array mappings.

class IComponentArray {
public:
	virtual ~IComponentArray() = default;
	virtual void entity_destroyed(Entity entity) = 0;
};

template <typename T> class ComponentArray : public IComponentArray {
public:
	void insert_data(Entity entity, T component) {
		assert(entityToIndexMap.find(entity) == entityToIndexMap.end() && "Component already added");

		// Put new entry at the end and update maps
		size_t new_index = size;
		entityToIndexMap[entity] = new_index;
		indexToEntityMap[new_index] = entity;
		componentArray[new_index] = component;
		size++;
	}

	void RemoveData(Entity entity) {
		assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Component does not exist");

		// Copy last element into place of removed element
		size_t index_of_removed_entity = entityToIndexMap[entity];
		size_t index_of_last_element = size - 1;
		componentArray[index_of_removed_entity] = componentArray[index_of_last_element];

		// Update maps to point to entity that's being removed
		Entity entity_of_last_element = indexToEntityMap[index_of_last_element];
		entityToIndexMap[entity_of_last_element] = index_of_removed_entity;
		indexToEntityMap[index_of_removed_entity] = entity_of_last_element;

		// Remove entity (while maintaining packed data)
		entityToIndexMap.erase(entity);
		indexToEntityMap.erase(index_of_last_element);

		size--;
	}

private:
	// The packed array of components (of generic type T),
	// set to a specified maximum amount, matching the maximum number
	// of entities allowed to exist simultaneously, so that each entity
	// has a unique spot.
	std::array<T, MAX_ENTITIES> componentArray;

	// Map from an entity ID to an array index.
	std::unordered_map<Entity, size_t> entityToIndexMap;

	// Map from an array index to an entity ID.
	std::unordered_map<size_t, Entity> indexToEntityMap;

	// Total size of valid entries in the array.
	size_t size;
};