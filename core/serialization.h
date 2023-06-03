#ifndef SILENCE_SERIALIZATION_H
#define SILENCE_SERIALIZATION_H

#include "components/agent_data_component.h"
#include "components/attachment_component.h"
#include "components/camera_component.h"
#include "components/children_component.h"
#include "components/collider_aabb.h"
#include "components/collider_capsule.h"
#include "components/collider_obb.h"
#include "components/collider_sphere.h"
#include "components/collider_tag_component.h"
#include "components/enemy_data_component.h"
#include "components/enemy_path_component.h"
#include "components/exploding_box_component.h"
#include "components/fmod_listener_component.h"
#include "components/hacker_data_component.h"
#include "components/interactable_component.h"
#include "components/light_component.h"
#include "components/name_component.h"
#include "components/parent_component.h"
#include "components/platform_component.h"
#include "components/rigidbody_component.h"
#include "components/static_tag_component.h"
#include "components/transform_component.h"

#include "components/path_node_component.h"
#include "components/path_parent_component.h"
#include "components/taggable_component.h"
#include "core/components/exploding_box_component.h"
#include "managers/animation/ecs/animation_instance.h"
#include "managers/render/ecs/model_instance.h"
#include "managers/render/ecs/skinned_model_instance.h"
#include "render/ecs/billboard_component.h"
#include "core/components/fmod_emitter_component.h"

namespace serialization {

template <typename T>
concept Serializable = requires(T t, nlohmann::json &j) {
	{ t.serialize_json(j) };
};

template <typename T>
concept Deserializable = requires(T t, nlohmann::json &j) {
	{ t.deserialize_json(j) };
};

typedef std::variant<Children, Parent, Transform, RigidBody, FmodListener, Camera, ModelInstance, AnimationInstance,
		SkinnedModelInstance, Name, ColliderTag, StaticTag, ColliderSphere, ColliderAABB, ColliderOBB, ColliderCapsule,
		Light, AgentData, HackerData, EnemyPath, EnemyData, Interactable, Attachment, Platform, ExplodingBox, Billboard,
		PathNode, PathParent, Taggable, FMODEmitter>
		variant_type;

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
