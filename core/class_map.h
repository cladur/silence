#ifndef SILENCE_CLASS_MAP_H
#define SILENCE_CLASS_MAP_H

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

typedef std::variant<Children, Parent, Transform, RigidBody, FmodListener, Gravity, MeshInstance> variant_type;
template <typename T> variant_type create_instance() {
	return variant_type(T());
}

typedef std::map<std::string, std::function<variant_type()>> map_type;

#endif //SILENCE_CLASS_MAP_H
