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

	// Maps with add/has component function calls that use id of component instead of object. Component has the same id
	// as its index in the vector of component_names which are added when component is registered
	std::unordered_map<int, std::function<void(Entity entity)>> add_component_map;
	std::unordered_map<int, std::function<bool(Entity entity)>> has_component_map;

	std::vector<std::string> component_names;
	int registered_components = 0;

public:
	static ECSManager &get();
	void startup();

	// Entity methods
	Entity create_entity();
	Entity create_entity(Entity entity);
	void destroy_entity(Entity entity);

	// Component methods
	template <typename T> void register_component() {
		component_manager->register_component<T>();

		std::string type_name = typeid(T).name();
		// Remove number prefix from type name
		while (type_name[0] >= '0' && type_name[0] <= '9') {
			type_name.erase(0, 1);
		}
		SPDLOG_INFO("Registering component {}", type_name);
		// Remove whitespace from type name
		size_t pos = type_name.find(" ");
		type_name = type_name.substr(pos + 1);
		int type_id = component_names.size();
		component_names.emplace_back(type_name);

		add_component_map[type_id] = [this](Entity entity) { add_component(entity, T{}); };
		has_component_map[type_id] = [this](Entity entity) { return has_component<T>(entity); };

		registered_components++;
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
	bool add_child(Entity parent, Entity child, bool keep_transform = false);
	bool remove_child(Entity parent, Entity child, bool keep_transform = false);
	bool has_child(Entity parent, Entity child);
	bool reparent(Entity new_parent, Entity child, bool keep_transform = false);
	void serialize_entity_json(nlohmann::json &json, Entity entity);
	void deserialize_entities_json(nlohmann::json &json, std::vector<Entity> &entities);
	void print_components();

	void add_component(Entity entity, int component_id);
	bool has_component(Entity entity, int component_id);

	Signature get_entity_signature(Entity entity);
	std::vector<std::string> &get_component_names();
	int get_registered_components();
};

#endif //SILENCE_ECSMANAGER_H