#include "physics_system.h"
#include "components/collider_aabb.h"
#include "components/collider_tag_component.h"
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

void PhysicsSystem::update_collision() {
	for (auto it1 = entities_with_collider.begin(); it1 != entities_with_collider.end(); ++it1) {
		for (auto it2 = std::next(it1); it2 != entities_with_collider.end(); ++it2) {
			resolve_collision_aabb(std::ref(*it1), std::ref(*it2));
		}
	}
}

bool PhysicsSystem::is_overlap(ColliderAABB &a, ColliderAABB &b) {
	// Check X axis
	if (abs(a.center.x - b.center.x) >= (b.range.x + b.range.x)) {
		return false;
	}
	// Check Y axis
	if (abs(a.center.y - b.center.y) >= (b.range.y + b.range.y)) {
		return false;
	}
	// Check Z axis
	if (abs(a.center.z - b.center.z) >= (b.range.z + b.range.z)) {
		return false;
	}

	return true;
}

void PhysicsSystem::resolve_collision_aabb(Entity e1, Entity e2) {
	ColliderAABB &c1 = ecs_manager.get_component<ColliderAABB>(e1);
	ColliderAABB &c2 = ecs_manager.get_component<ColliderAABB>(e2);
	Transform &t1 = ecs_manager.get_component<Transform>(e1);
	Transform &t2 = ecs_manager.get_component<Transform>(e2);
	c1.center = t1.get_position();
	c2.center = t2.get_position();

	if (!(c1.is_movable || c2.is_movable) || !is_overlap(c1, c2)) {
		return;
	}

	const float l1 = c1.center.x - c2.range.x, l2 = c2.center.x - c2.range.x;
	const float r1 = c1.center.x + c2.range.x, r2 = c2.center.x + c2.range.x;
	const float b1 = c1.center.z - c2.range.z, b2 = c2.center.z - c2.range.z;
	const float f1 = c1.center.z + c2.range.z, f2 = c2.center.z + c2.range.z;
	const float d1 = c1.center.y - c2.range.y, d2 = c2.center.y - c2.range.y;
	const float u1 = c1.center.y + c2.range.y, u2 = c2.center.y + c2.range.y;

	const float left = r1 - l2, right = r2 - l1;
	const float up = u1 - d2, down = u2 - d1;
	const float front = f1 - b2, back = f2 - b1;

	glm::vec3 direction(0.0f);

	left < right ? direction.x = -left : direction.x = right;
	up < down ? direction.y = -up : direction.y = down;
	front < back ? direction.z = -front : direction.z = back;

	direction.x *direction.x < direction.y *direction.y ? direction.y = 0.0f : direction.x = 0.0f;

	if (direction.x == 0.0f) {
		direction.z *direction.z < direction.y *direction.y ? direction.y = 0.0f : direction.z = 0.0f;
	} else {
		direction.z *direction.z < direction.x *direction.x ? direction.x = 0.0f : direction.z = 0.0f;
	}

	if (c1.is_movable && c2.is_movable) {
		// Value defining how fast c2 object will move object c1
		// 1 means that object c2 has same speed while pushing object c1
		const float slide_speed = 1.0f / 2.0f;
		c1.center += direction * slide_speed;
		c2.center -= direction * (1.0f - slide_speed);
		t1.set_position(c1.center);
		t2.set_position(c2.center);
	} else if (c1.is_movable) {
		c1.center += direction;
		t1.set_position(c1.center);
	} else {
		c2.center += direction;
		t2.set_position(c2.center);
	}
}
