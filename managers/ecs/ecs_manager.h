#ifndef SILENCE_ECSMANAGER_H
#define SILENCE_ECSMANAGER_H

#include "component_manager.h"
#include "entity_manager.h"
#include "serialization.h"
#include "system_manager.h"

class ECSManager {
private:
	std::unique_ptr<ComponentManager> component_manager;
	std::unique_ptr<EntityManager> entity_manager;
	std::unique_ptr<SystemManager> system_manager;

public:
	void startup();

	// Entity methods
	Entity create_entity();
	Entity create_entity(Entity entity);
	void destroy_entity(Entity entity);

	// Component methods
	template <typename T> void register_component() {
		component_manager->register_component<T>();
	}

	template <typename T> void add_component(Entity entity, T component) {
		component_manager->add_component<T>(entity, component);

		auto signature = entity_manager->get_signature(entity);
		signature.set(component_manager->get_component_type<T>(), true);
		entity_manager->set_entity_signature(entity, signature);

		system_manager->entity_signature_changed(entity, signature);
	}

	template <typename T> void update_component(Entity entity, T component) {
		component_manager->update_component<T>(entity, component);
	}

	template <typename T> void remove_component(Entity entity) {
		component_manager->remove_component<T>(entity);

		auto signature = entity_manager->get_signature(entity);
		signature.set(component_manager->get_component_type<T>(), false);
		entity_manager->set_entity_signature(entity, signature);

		system_manager->entity_signature_changed(entity, signature);
	}

	template <typename T> T &get_component(Entity entity) {
		return component_manager->get_component<T>(entity);
	}

	template <typename T> bool has_component(Entity entity) {
		return component_manager->has_component<T>(entity);
	}

	// Added to pass type as variable not in <>
	template <typename T> bool has_component(Entity entity, const T &component) {
		return component_manager->has_component<T>(entity);
	}

	template <typename T> ComponentType get_component_type() {
		return component_manager->get_component_type<T>();
	}

	// System methods
	template <typename T> std::shared_ptr<T> register_system() {
		return system_manager->register_system<T>();
	}

	template <typename T> void set_system_component_whitelist(Signature signature) {
		system_manager->set_component_whitelist<T>(signature);
	}

	template <typename T> void set_system_component_blacklist(Signature signature) {
		system_manager->set_component_blacklist<T>(signature);
	}

	// Specific parent system methods
	bool add_child(Entity parent, Entity child);
	bool remove_child(Entity parent, Entity child);
	bool has_child(Entity parent, Entity child);
	void serialize_entity_json(nlohmann::json &json, Entity entity);
	void deserialize_entities_json(nlohmann::json &json, std::vector<Entity> &entities);
};

#endif //SILENCE_ECSMANAGER_H