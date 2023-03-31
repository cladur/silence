#include "physics_system.h"
#include "components/gravity_component.h"
#include "components/rigidbody_component.h"
#include "components/transform_component.h"
#include "ecs/ecs_manager.h"

extern ECSManager ecs_manager;

void PhysicsSystem::startup() {
}
void PhysicsSystem::update(float dt) {
	for (auto const &entity : entities) {
		auto &rigid_body = ecs_manager.get_component<RigidBody>(entity);
		auto &transform = ecs_manager.get_component<Transform>(entity);

		// Forces
		auto const &gravity = ecs_manager.get_component<Gravity>(entity);

		rigid_body.velocity += gravity.force * dt;

		transform.add_position(rigid_body.velocity * dt);
	}
}
