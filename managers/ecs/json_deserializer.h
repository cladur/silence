#ifndef SILENCE_JSONDESERIALIZER_H
#define SILENCE_JSONDESERIALIZER_H
#include "components/children_component.h"
#include "components/fmod_listener_component.h"
#include "components/gravity_component.h"
#include "components/mesh_instance_component.h"
#include "components/parent_component.h"
#include "components/rigidbody_component.h"
#include "components/transform_component.h"

#include "ecs_manager.h"
extern ECSManager ecs_manager;

typedef boost::variant<Children, Parent, Transform, RigidBody, FmodListener, Gravity> variant_type;
template <typename T> variant_type create_instance() {
	return variant_type(T());
}

typedef std::map<std::string, std::function<variant_type()>> map_type;

class JsonDeserializer {
public:
	void deserialize_entity_json(nlohmann::json &j, Entity entity);
	void deserialize_scene_json(nlohmann::json &j);

	template <typename T> void add_component_to_map(const std::string &name) {
		class_map[name] = create_instance<T>;
	}

private:
	void get_components_on_entity(nlohmann::json &j, std::queue<int> &components_on_entity);
	map_type class_map{};
};

#endif //SILENCE_JSONDESERIALIZER_H
