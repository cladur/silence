#ifndef SILENCE_COMPONENTMANAGER_H
#define SILENCE_COMPONENTMANAGER_H

#include "component_array.h"

class ComponentManager {
private:
	// Map from type string pointer to a component type
	std::unordered_map<const char *, ComponentType> component_types{};

	// Map from type string pointer to a component array
	std::unordered_map<const char *, std::shared_ptr<IComponentArray>> component_arrays{};

	// The component type to be assigned to the next registered component - starting at 0
	ComponentType next_component_type{};

	// Convenience function to get the statically cast pointer to the ComponentArray of type T.
	template <typename T> std::shared_ptr<ComponentArray<T>> get_component_array() {
		const char *type_name = typeid(T).name();

		assert(component_types.find(type_name) != component_types.end() && "Component not registered before use.");

		return std::static_pointer_cast<ComponentArray<T>>(component_arrays[type_name]);
	}

public:
	template <typename T> void register_component() {
		const char *type_name = typeid(T).name();

		assert(component_types.find(type_name) == component_types.end() &&
				"Registering component type more than once.");

		// Add this component type to the component type map
		component_types.insert({ type_name, next_component_type });

		// Create a ComponentArray pointer and add it to the component arrays map
		component_arrays.insert({ type_name, std::make_shared<ComponentArray<T>>() });

		// Increment the value so that the next component registered will be different
		next_component_type++;
	}

	template <typename T> ComponentType get_component_type() {
		const char *type_name = typeid(T).name();

		assert(component_types.find(type_name) != component_types.end() && "Component not registered before use.");

		// Return this component's type - used for creating signatures
		return component_types[type_name];
	}

	template <typename T> void add_component(Entity entity, T component) {
		// Add a component to the array for an entity
		get_component_array<T>()->insert_data(entity, component);
	}

	template <typename T> void remove_component(Entity entity) {
		// Remove a component from the array for an entity
		get_component_array<T>()->remove_data(entity);
	}

	template <typename T> T &get_component(Entity entity) {
		// Get a reference to a component from the array for an entity
		return get_component_array<T>()->get_data(entity);
	}

	template <typename T> bool has_component(Entity entity) {
		// Check if entity has a component
		return get_component_array<T>()->has_component(entity);
	}

	void entity_destroyed(Entity entity) {
		// Notify each component array that an entity has been destroyed
		// If it has a component for that entity, it will remove it
		for (auto const &pair : component_arrays) {
			auto const &component = pair.second;

			component->entity_destroyed(entity);
		}
	}

	void serialize_entity(nlohmann::json &json, Entity entity) {
		for (auto const &pair : component_arrays) {
			auto const &component = pair.second;

			component->serialize_entity_json(json, entity);
		}
	}
};

#endif //SILENCE_COMPONENTMANAGER_H