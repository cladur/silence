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
	std::list<Entity> entities_with_collider;

	for (auto entity : entities) {
		if (ecs_manager.has_component<ColliderTag>(entity)) {
			entities_with_collider.push_back(entity);
		}
	}

	for (auto it1 = entities_with_collider.begin(); it1 != entities_with_collider.end(); ++it1) {
		for (auto it2 = std::next(it1); it2 != entities_with_collider.end(); ++it2) {
			resolve_collision(ecs_manager.get_component<ColliderAABB>(std::ref(*it1)),
					ecs_manager.get_component<ColliderAABB>(std::ref(*it2)));
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

void PhysicsSystem::resolve_collision(ColliderAABB &a, ColliderAABB &b) {
	if (!(a.is_movable || b.is_movable) || !is_overlap(a, b)) {
		return;
	}

	const float l1 = a.center.x - b.range.x, l2 = b.center.x - b.range.x;
	const float r1 = a.center.x + b.range.x, r2 = b.center.x + b.range.x;
	const float b1 = a.center.z - b.range.z, b2 = b.center.z - b.range.z;
	const float f1 = a.center.z + b.range.z, f2 = b.center.z + b.range.z;
	const float d1 = a.center.y - b.range.y, d2 = b.center.y - b.range.y;
	const float u1 = a.center.y + b.range.y, u2 = b.center.y + b.range.y;

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

	if (a.is_movable && b.is_movable) {
		// Value defining how fast b object will move object a
		// 1 means that object b has same speed while pushing object a
		const float slide_speed = 1.0f / 2.0f;
		a.center += direction * slide_speed;
		b.center -= direction * (1.0f - slide_speed);
	} else if (a.is_movable) {
		a.center += direction;
	} else {
		b.center += direction;
	}
}
