#include "collision_system.h"

#include "components/collider_aabb.h"
#include "components/collider_sphere.h"
#include "components/collider_tag_component.h"
#include "ecs/ecs_manager.h"

extern ECSManager ecs_manager;

void CollisionSystem::startup() {
	Signature signature;
	signature.set(ecs_manager.get_component_type<ColliderTag>());
	ecs_manager.set_system_component_whitelist<CollisionSystem>(signature);
}

void CollisionSystem::update() {
	for (auto it1 = entities.begin(); it1 != entities.end(); ++it1) {
		Entity e1 = std::ref(*it1);
		for (auto it2 = std::next(it1); it2 != entities.end(); ++it2) {
			Entity e2 = std::ref(*it2);

			if (ecs_manager.has_component<ColliderAABB>(e1) && ecs_manager.has_component<ColliderAABB>(e2)) {
				resolve_collision_aabb(std::ref(*it1), std::ref(*it2));
			} else if (ecs_manager.has_component<ColliderSphere>(e1) && ecs_manager.has_component<ColliderSphere>(e2)) {
				resolve_collision_sphere(std::ref(*it1), std::ref(*it2));
			} else if (ecs_manager.has_component<ColliderAABB>(e1) && ecs_manager.has_component<ColliderSphere>(e2)) {
				resolve_aabb_sphere(std::ref(*it1), std::ref(*it2));
			} else if (ecs_manager.has_component<ColliderSphere>(e1) && ecs_manager.has_component<ColliderAABB>(e2)) {
				resolve_aabb_sphere(std::ref(*it2), std::ref(*it1));
			}
		}
	}
}

bool CollisionSystem::is_overlap(ColliderAABB &a, ColliderAABB &b) {
	// Check X axis
	if (abs(a.center.x - b.center.x) >= (a.range.x + b.range.x)) {
		return false;
	}
	// Check Y axis
	if (abs(a.center.y - b.center.y) >= (a.range.y + b.range.y)) {
		return false;
	}
	// Check Z axis
	if (abs(a.center.z - b.center.z) >= (a.range.z + b.range.z)) {
		return false;
	}

	return true;
}

void CollisionSystem::resolve_collision_aabb(Entity e1, Entity e2) {
	ColliderAABB &c1 = ecs_manager.get_component<ColliderAABB>(e1);
	ColliderAABB &c2 = ecs_manager.get_component<ColliderAABB>(e2);
	Transform &t1 = ecs_manager.get_component<Transform>(e1);
	Transform &t2 = ecs_manager.get_component<Transform>(e2);
	c1.center = t1.get_position();
	c2.center = t2.get_position();

	if (!(c1.is_movable || c2.is_movable) || !is_overlap(c1, c2)) {
		return;
	}

	const float l1 = c1.center.x - c1.range.x, l2 = c2.center.x - c2.range.x;
	const float r1 = c1.center.x + c1.range.x, r2 = c2.center.x + c2.range.x;
	const float b1 = c1.center.z - c1.range.z, b2 = c2.center.z - c2.range.z;
	const float f1 = c1.center.z + c1.range.z, f2 = c2.center.z + c2.range.z;
	const float d1 = c1.center.y - c1.range.y, d2 = c2.center.y - c2.range.y;
	const float u1 = c1.center.y + c1.range.y, u2 = c2.center.y + c2.range.y;

	const float left = r1 - l2, right = r2 - l1;
	const float up = u1 - d2, down = u2 - d1;
	const float front = f1 - b2, back = f2 - b1;

	glm::vec3 offset(0.0f);

	left < right ? offset.x = left : offset.x = -right;
	up < down ? offset.y = up : offset.y = -down;
	front < back ? offset.z = front : offset.z = -back;

	offset.x *offset.x < offset.y *offset.y ? offset.y = 0.0f : offset.x = 0.0f;

	if (offset.x == 0.0f) {
		offset.z *offset.z < offset.y *offset.y ? offset.y = 0.0f : offset.z = 0.0f;
	} else {
		offset.z *offset.z < offset.x *offset.x ? offset.x = 0.0f : offset.z = 0.0f;
	}

	if (c1.is_movable && c2.is_movable) {
		// Value defining how fast c2 object will move object c1
		// 1 means that object c2 has same speed while pushing object c1
		const float slide_speed = 0.5f;
		t1.add_position(-offset * slide_speed);
		t2.add_position(offset * (1.0f - slide_speed));
	} else if (c1.is_movable) {
		t1.add_position(-offset);
	} else {
		t2.add_position(offset);
	}
}

bool CollisionSystem::is_overlap(ColliderSphere &a, ColliderSphere &b) {
	glm::vec3 distance = a.center - b.center;
	float distance_squared = glm::dot(distance, distance);
	float radius_sum = a.radius + b.radius;

	return distance_squared <= (radius_sum * radius_sum);
}

void CollisionSystem::resolve_collision_sphere(Entity e1, Entity e2) {
	ColliderSphere &c1 = ecs_manager.get_component<ColliderSphere>(e1);
	ColliderSphere &c2 = ecs_manager.get_component<ColliderSphere>(e2);
	Transform &t1 = ecs_manager.get_component<Transform>(e1);
	Transform &t2 = ecs_manager.get_component<Transform>(e2);
	c1.center = t1.get_position();
	c2.center = t2.get_position();

	if (!(c1.is_movable || c2.is_movable) || !is_overlap(c1, c2)) {
		return;
	}

	const glm::vec3 direction = c2.center - c1.center;
	const float distance = c1.radius + c2.radius - glm::length(direction);
	const glm::vec3 offset = glm::normalize(direction) * distance;

	if (c1.is_movable && c2.is_movable) {
		// Value defining how fast c2 object will move object c1
		// 1 means that object c2 has same speed while pushing object c1
		const float slide_speed = 0.5f;
		t1.add_position(-offset * slide_speed);
		t2.add_position(offset * (1.0f - slide_speed));
	} else if (c1.is_movable) {
		t1.add_position(-offset);
	} else {
		t2.add_position(offset);
	}
}
bool CollisionSystem::is_overlap(ColliderAABB &aabb, ColliderSphere &sphere) {
	const glm::vec min = aabb.min();
	const glm::vec max = aabb.max();
	// Calculate nearest point AABB and Sphere center
	const float x = std::max(min.x, std::min(sphere.center.x, max.x));
	const float y = std::max(min.y, std::min(sphere.center.y, max.y));
	const float z = std::max(min.z, std::min(sphere.center.z, max.z));
	const float dx = (x - sphere.center.x);
	const float dy = (y - sphere.center.y);
	const float dz = (z - sphere.center.z);
	const float distance_squared = dx * dx + dy * dy + dz * dz;

	return distance_squared <= (sphere.radius * sphere.radius);
}

void CollisionSystem::resolve_aabb_sphere(Entity aabb, Entity sphere) {
	ColliderAABB &c1 = ecs_manager.get_component<ColliderAABB>(aabb);
	ColliderSphere &c2 = ecs_manager.get_component<ColliderSphere>(sphere);
	Transform &t1 = ecs_manager.get_component<Transform>(aabb);
	Transform &t2 = ecs_manager.get_component<Transform>(sphere);
	c1.center = t1.get_position();
	c2.center = t2.get_position();

	if (!(c1.is_movable || c2.is_movable) || !is_overlap(c1, c2)) {
		return;
	}

	const glm::vec3 min = c1.min();
	const glm::vec3 max = c1.max();

	// Calculate nearest point on AABB to sphere center
	glm::vec3 closest_point;
	closest_point.x = glm::clamp(c2.center.x, min.x, max.x);
	closest_point.y = glm::clamp(c2.center.y, min.y, max.y);
	closest_point.z = glm::clamp(c2.center.z, min.z, max.z);

	// Calculate the vector from the sphere center to the closest point on the AABB
	const glm::vec3 direction = closest_point - c2.center;
	const float length = c2.radius - glm::length(direction);
	const glm::vec3 offset = glm::normalize(direction) * length;

	// Resolve the collision
	if (c1.is_movable && c2.is_movable) {
		// Value defining how fast the sphere will move the AABB
		// 1 means that the sphere has same speed while pushing the AABB
		const float slide_speed = 0.5f;
		t1.add_position(offset * slide_speed);
		t2.add_position(-offset * (1.0f - slide_speed));
	} else if (c1.is_movable) {
		t1.add_position(offset);
	} else {
		t2.add_position(-offset);
	}
}
