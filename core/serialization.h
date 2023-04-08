#ifndef SILENCE_SERIALIZATION_H
#define SILENCE_SERIALIZATION_H

#include "components/children_component.h"
#include "components/fmod_listener_component.h"
#include "components/gravity_component.h"
#include "components/mesh_instance_component.h"
#include "components/parent_component.h"
#include "components/rigidbody_component.h"
#include "components/transform_component.h"

#include <functional>
#include <map>
#include <string>
#include <variant>

namespace serialization {

template <typename T>
concept Serializable = requires(T t, nlohmann::json &j) {
	{ t.serialize_json(j) };
};

template <typename T>
concept Deserializable = requires(T t, nlohmann::json &j) {
	{ t.deserialize_json(j) };
};

typedef std::variant<Children, Parent, Transform, RigidBody, FmodListener, Gravity, MeshInstance> variant_type;

template <typename T>
T create_instance(nlohmann::json &j)
	requires Deserializable<T>
{
	variant_type component_variant = variant_type(T());
	T component = std::get<T>(component_variant);
	component.deserialize_json(j);
	return component;
}

typedef std::map<ComponentType, std::function<variant_type(nlohmann::json &j)>> IdToClassConstructor;

} //namespace serialization

#endif //SILENCE_SERIALIZATION_H