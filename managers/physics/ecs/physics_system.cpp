#include "physics_system.h"
#include "components/rigidbody_component.h"
#include "components/static_tag_component.h"
#include "components/transform_component.h"
#include "ecs/world.h"
#include "managers/physics/physics_manager.h"

AutoCVarInt cvar_enable_physics("physics.enable", "enable physics", 1, CVarFlags::EditCheckbox);

void PhysicsSystem::startup(World &world) {
	Signature white_signature;
	white_signature.set(world.get_component_type<RigidBody>());
	white_signature.set(world.get_component_type<Transform>());
	world.set_system_component_whitelist<PhysicsSystem>(white_signature);

	Signature black_signature;
	black_signature.set(world.get_component_type<StaticTag>());
	world.set_system_component_blacklist<PhysicsSystem>(black_signature);
}

void PhysicsSystem::update(World &world, float dt) {
	ZoneScopedN("PhysicsSystem::update");
	if (!cvar_enable_physics.get()) {
		return;
	}

	PhysicsManager &physics_manager = PhysicsManager::get();
	float epsilon = physics_manager.get_epsilon();
	glm::vec3 gravity = physics_manager.get_gravity();
	for (auto const &entity : entities) {
		auto &rigid_body = world.get_component<RigidBody>(entity);
		auto &transform = world.get_component<Transform>(entity);

		if (rigid_body.mass <= epsilon) {
			SPDLOG_WARN("Object has value of mass near 0");
		}

		rigid_body.velocity += (gravity * dt) / rigid_body.mass;

		transform.add_position(rigid_body.velocity * dt);
	}
}
