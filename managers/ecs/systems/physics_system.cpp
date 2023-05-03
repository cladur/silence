#include "physics_system.h"
#include "components/gravity_component.h"
#include "components/rigidbody_component.h"
#include "components/transform_component.h"
#include "ecs/world.h"

void PhysicsSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<Gravity>());
	signature.set(world.get_component_type<RigidBody>());
	signature.set(world.get_component_type<Transform>());
	world.set_system_component_whitelist<PhysicsSystem>(signature);
}

void PhysicsSystem::update(World &world, float dt) {
	ZoneScopedN("PhysicsSystem::update");
	for (auto const &entity : entities) {
		auto &rigid_body = world.get_component<RigidBody>(entity);
		auto &transform = world.get_component<Transform>(entity);

		// Forces
		auto const &gravity = world.get_component<Gravity>(entity);

		rigid_body.velocity += gravity.force * dt;

		transform.add_position(rigid_body.velocity * dt);
	}
}
