#include "collision_system.h"

#include "components/collider_aabb.h"
#include "components/collider_sphere.h"
#include "components/collider_tag_component.h"
#include "ecs/ecs_manager.h"

void CollisionSystem::startup() {
	ECSManager &ecs_manager = ECSManager::get();
	Signature signature;
	signature.set(ecs_manager.get_component_type<ColliderTag>());
	ecs_manager.set_system_component_whitelist<CollisionSystem>(signature);
}

void CollisionSystem::update() { //TODO: Optimize update because now it's O(n^2) :(
	ZoneScopedN("CollisionSystem::update");
	ECSManager &ecs_manager = ECSManager::get();

	CollisionFlag first, second;
	for (auto it1 = entities.begin(); it1 != entities.end(); ++it1) {
		Entity e1 = std::ref(*it1);
		if (ecs_manager.has_component<ColliderOBB>(e1)) {
			first = CollisionFlag::FIRST_OBB;
		} else if (ecs_manager.has_component<ColliderAABB>(e1)) {
			first = CollisionFlag::FIRST_AABB;
		} else if (ecs_manager.has_component<ColliderSphere>(e1)) {
			first = CollisionFlag::FIRST_SPHERE;
		} else {
			continue;
		}
		for (auto it2 = std::next(it1); it2 != entities.end(); ++it2) {
			Entity e2 = std::ref(*it2);

			if (ecs_manager.has_component<ColliderOBB>(e2)) {
				second = CollisionFlag::SECOND_OBB;
			} else if (ecs_manager.has_component<ColliderAABB>(e2)) {
				second = CollisionFlag::SECOND_AABB;
			} else if (ecs_manager.has_component<ColliderSphere>(e2)) {
				second = CollisionFlag::SECOND_SPHERE;
			} else {
				continue;
			}

			switch (first | second) {
				case CollisionFlag::SPHERE_SPHERE:
					resolve_collision_sphere(e1, e2);
					break;
				case CollisionFlag::AABB_AABB:
					resolve_collision_aabb(e1, e2);
					break;
				case CollisionFlag::SPHERE_AABB:
					resolve_aabb_sphere(e2, e1);
					break;
				case CollisionFlag::AABB_SPHERE:
					resolve_aabb_sphere(e1, e2);
					break;
				case CollisionFlag::OBB_OBB:
					resolve_collision_obb(e1, e2);
					break;
				case CollisionFlag::SPHERE_OBB:
					resolve_obb_sphere(e2, e1);
					break;
				case CollisionFlag::OBB_SPHERE:
					resolve_obb_sphere(e1, e2);
					break;
				case CollisionFlag::AABB_OBB:
					resolve_obb_aabb(e2, e1);
					break;
				case CollisionFlag::OBB_AABB:
					resolve_obb_aabb(e1, e2);
					break;
				default:
					break;
			}
		}
	}
}

bool CollisionSystem::is_overlap(ColliderSphere &a, ColliderSphere &b) {
	glm::vec3 distance = a.center - b.center;
	float distance_squared = glm::dot(distance, distance);
	float radius_sum = a.radius + b.radius;

	return distance_squared <= (radius_sum * radius_sum);
}

void CollisionSystem::resolve_collision_sphere(Entity e1, Entity e2) {
	ECSManager &ecs_manager = ECSManager::get();
	ColliderSphere &c1 = ecs_manager.get_component<ColliderSphere>(e1);
	ColliderSphere &c2 = ecs_manager.get_component<ColliderSphere>(e2);
	Transform *t1;
	Transform *t2;

	if (!(c1.is_movable || c2.is_movable)) {
		return;
	} else if (c1.is_movable && c2.is_movable) {
		t1 = &ecs_manager.get_component<Transform>(e1);
		c1.center = t1->get_position();

		t2 = &ecs_manager.get_component<Transform>(e2);
		c2.center = t2->get_position();
	} else if (c1.is_movable) {
		t1 = &ecs_manager.get_component<Transform>(e1);
		c1.center = t1->get_position();
	} else {
		t2 = &ecs_manager.get_component<Transform>(e2);
		c2.center = t2->get_position();
	}

	if (!is_overlap(c1, c2)) {
		return;
	}

	const glm::vec3 direction = c2.center - c1.center;
	const float distance = c1.radius + c2.radius - glm::length(direction);
	const glm::vec3 offset = glm::normalize(direction) * distance;

	if (c1.is_movable && c2.is_movable) {
		// Value defining how fast the body will move other body
		// 1 means that the bodies has same speed while pushing
		const float slide_speed = 0.5f;
		t1->add_position(-offset * slide_speed);
		t2->add_position(offset * (1.0f - slide_speed));
	} else if (c1.is_movable) {
		t1->add_position(-offset);
	} else {
		t2->add_position(offset);
	}
}

bool CollisionSystem::is_overlap(ColliderAABB &a, ColliderAABB &b) {
	// Check X axis
	if (abs(a.center.x - b.center.x) > (a.range.x + b.range.x)) {
		return false;
	}
	// Check Y axis
	if (abs(a.center.y - b.center.y) > (a.range.y + b.range.y)) {
		return false;
	}
	// Check Z axis
	if (abs(a.center.z - b.center.z) > (a.range.z + b.range.z)) {
		return false;
	}

	return true;
}

void CollisionSystem::resolve_collision_aabb(Entity e1, Entity e2) {
	ECSManager &ecs_manager = ECSManager::get();
	ColliderAABB &c1 = ecs_manager.get_component<ColliderAABB>(e1);
	ColliderAABB &c2 = ecs_manager.get_component<ColliderAABB>(e2);
	Transform *t1;
	Transform *t2;

	if (!(c1.is_movable || c2.is_movable)) {
		return;
	} else if (c1.is_movable && c2.is_movable) {
		t1 = &ecs_manager.get_component<Transform>(e1);
		c1.center = t1->get_position();

		t2 = &ecs_manager.get_component<Transform>(e2);
		c2.center = t2->get_position();
	} else if (c1.is_movable) {
		t1 = &ecs_manager.get_component<Transform>(e1);
		c1.center = t1->get_position();
	} else {
		t2 = &ecs_manager.get_component<Transform>(e2);
		c2.center = t2->get_position();
	}

	if (!is_overlap(c1, c2)) {
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
		// Value defining how fast the body will move other body
		// 1 means that the bodies has same speed while pushing
		const float slide_speed = 0.5f;
		t1->add_position(-offset * slide_speed);
		t2->add_position(offset * (1.0f - slide_speed));
	} else if (c1.is_movable) {
		t1->add_position(-offset);
	} else {
		t2->add_position(offset);
	}
}

glm::vec3 CollisionSystem::is_overlap(ColliderAABB &aabb, ColliderSphere &sphere) {
	const glm::vec3 min = aabb.min();
	const glm::vec3 max = aabb.max();
	// Calculate nearest point AABB and Sphere center
	const glm::vec3 closest_point = glm::vec3(std::clamp(sphere.center.x, min.x, max.x),
			std::clamp(sphere.center.y, min.y, max.y), std::clamp(sphere.center.z, min.z, max.z));

	const glm::vec3 direction = closest_point - sphere.center;
	const float distance = glm::length(direction);

	const float length = sphere.radius - distance;

	if (distance < sphere.radius) {
		if (direction != glm::vec3(0.0f)) {
			return glm::normalize(direction) * length;
		} else { //TODO: FIX this shit properly
			return glm::normalize(closest_point) * length;
		}
	}

	return glm::vec3(0.0f);
}

void CollisionSystem::resolve_aabb_sphere(Entity aabb, Entity sphere) {
	ECSManager &ecs_manager = ECSManager::get();
	ColliderAABB &c1 = ecs_manager.get_component<ColliderAABB>(aabb);
	ColliderSphere &c2 = ecs_manager.get_component<ColliderSphere>(sphere);
	Transform *t1;
	Transform *t2;

	if (!(c1.is_movable || c2.is_movable)) {
		return;
	} else if (c1.is_movable && c2.is_movable) {
		t1 = &ecs_manager.get_component<Transform>(aabb);
		c1.center = t1->get_position();

		t2 = &ecs_manager.get_component<Transform>(sphere);
		c2.center = t2->get_position();
	} else if (c1.is_movable) {
		t1 = &ecs_manager.get_component<Transform>(aabb);
		c1.center = t1->get_position();
	} else {
		t2 = &ecs_manager.get_component<Transform>(sphere);
		c2.center = t2->get_position();
	}

	glm::vec3 offset = is_overlap(c1, c2);
	if (offset == glm::vec3(0.0f)) {
		return;
	}

	// Resolve the collision
	if (c1.is_movable && c2.is_movable) {
		// Value defining how fast the body will move other body
		// 1 means that the bodies has same speed while pushing
		const float slide_speed = 0.5f;
		t1->add_position(offset * slide_speed);
		t2->add_position(-offset * (1.0f - slide_speed));
	} else if (c1.is_movable) {
		t1->add_position(offset);
	} else {
		t2->add_position(-offset);
	}
}

glm::vec3 CollisionSystem::is_overlap(ColliderOBB &a, ColliderOBB &b) {
	const glm::vec3 distance = a.center - b.center;
	const glm::mat3 o1 = a.get_orientation_matrix();
	const glm::mat3 o2 = b.get_orientation_matrix();
	const glm::vec3 axes[15] = {
		o1[0],
		o1[1],
		o1[2],
		o2[0],
		o2[1],
		o2[2],
		glm::cross(o1[0], o2[0]),
		glm::cross(o1[0], o2[1]),
		glm::cross(o1[0], o2[2]),
		glm::cross(o1[1], o2[0]),
		glm::cross(o1[1], o2[1]),
		glm::cross(o1[1], o2[2]),
		glm::cross(o1[2], o2[0]),
		glm::cross(o1[2], o2[1]),
		glm::cross(o1[2], o2[2]),
	};

	float max_overlap = -std::numeric_limits<float>::max();
	glm::vec3 separation(0.0f);
	for (const glm::vec3 &axis : axes) {
		if (axis == glm::vec3(0.0f)) {
			continue;
		}
		const float overlap = fabs(glm::dot(distance, axis)) -
				(fabs(glm::dot(o1[0] * a.range.x, axis)) + fabs(glm::dot(o1[1] * a.range.y, axis)) +
						fabs(glm::dot(o1[2] * a.range.z, axis)) + fabs(glm::dot(o2[0] * b.range.x, axis)) +
						fabs(glm::dot(o2[1] * b.range.y, axis)) + fabs(glm::dot(o2[2] * b.range.z, axis)));

		if (overlap >= 0.0f) {
			return glm::vec3(0.0f);
		}

		if (overlap > max_overlap) {
			max_overlap = overlap;
			separation = axis;
		}
	}

	if (glm::dot(separation, distance) >= 0.0f) {
		separation = -separation;
	}
	return separation * max_overlap;
}

void CollisionSystem::resolve_collision_obb(Entity e1, Entity e2) {
	ECSManager &ecs_manager = ECSManager::get();
	ColliderOBB &c1 = ecs_manager.get_component<ColliderOBB>(e1);
	ColliderOBB &c2 = ecs_manager.get_component<ColliderOBB>(e2);
	Transform *t1;
	Transform *t2;

	if (!(c1.is_movable || c2.is_movable)) {
		return;
	} else if (c1.is_movable && c2.is_movable) {
		t1 = &ecs_manager.get_component<Transform>(e1);
		c1.center = t1->get_position();
		c1.set_orientation(t1->get_euler_rot());

		t2 = &ecs_manager.get_component<Transform>(e2);
		c2.center = t2->get_position();
		c2.set_orientation(t2->get_euler_rot());
	} else if (c1.is_movable) {
		t1 = &ecs_manager.get_component<Transform>(e1);
		c1.center = t1->get_position();
		c1.set_orientation(t1->get_euler_rot());
	} else {
		t2 = &ecs_manager.get_component<Transform>(e2);
		c2.center = t2->get_position();
		c2.set_orientation(t2->get_euler_rot());
	}

	glm::vec3 offset = is_overlap(c1, c2);
	if (offset == glm::vec3(0.0f)) {
		return;
	}

	// Resolve the collision
	if (c1.is_movable && c2.is_movable) {
		// Value defining how fast the sphere will move the OBB
		// 1 means that the sphere has same speed while pushing the OBB
		const float slide_speed = 0.5f;
		t1->add_position(offset * 0.5f);
		t2->add_position(-offset * 0.5f);
	} else if (c1.is_movable) {
		t1->add_position(offset);
	} else {
		t2->add_position(-offset);
	}
}

glm::vec3 CollisionSystem::is_overlap(ColliderOBB &a, ColliderSphere &b) {
	const glm::mat3 orientation = a.get_orientation_matrix();
	const glm::vec3 sphere_center_local = glm::transpose(orientation) * (b.center - a.center);

	// Closest point on a to b center
	const glm::vec3 closest_point_on_obb = glm::clamp(sphere_center_local, -a.range, a.range);

	// Convert closest point back to world space
	const glm::vec3 closest_point_world = orientation * closest_point_on_obb + a.center;

	// Calculate distance between b center and closest point on a
	const glm::vec3 direction = closest_point_world - b.center;
	const float distance = glm::length(direction);

	const float length = b.radius - distance;

	if (distance < b.radius) {
		if (direction != glm::vec3(0.0f)) {
			return glm::normalize(direction) * length;
		} else { //TODO: FIX this shit properly
			return glm::normalize(closest_point_world) * length;
		}
	}
	return glm::vec3(0.0f);
}

void CollisionSystem::resolve_obb_sphere(Entity obb, Entity sphere) {
	ECSManager &ecs_manager = ECSManager::get();
	ColliderOBB &c1 = ecs_manager.get_component<ColliderOBB>(obb);
	ColliderSphere &c2 = ecs_manager.get_component<ColliderSphere>(sphere);
	Transform *t1;
	Transform *t2;

	if (!(c1.is_movable || c2.is_movable)) {
		return;
	} else if (c1.is_movable && c2.is_movable) {
		t1 = &ecs_manager.get_component<Transform>(obb);
		c1.center = t1->get_position();
		c1.set_orientation(t1->get_euler_rot());

		t2 = &ecs_manager.get_component<Transform>(sphere);
		c2.center = t2->get_position();
	} else if (c1.is_movable) {
		t1 = &ecs_manager.get_component<Transform>(obb);
		c1.center = t1->get_position();
		c1.set_orientation(t1->get_euler_rot());
	} else {
		t2 = &ecs_manager.get_component<Transform>(sphere);
		c2.center = t2->get_position();
	}

	glm::vec3 offset = is_overlap(c1, c2);
	if (offset == glm::vec3(0.0f)) {
		return;
	}

	// Resolve the collision
	if (c1.is_movable && c2.is_movable) {
		// Value defining how fast the sphere will move the OBB
		// 1 means that the sphere has same speed while pushing the OBB
		const float slide_speed = 0.5f;
		t1->add_position(offset * slide_speed);
		t2->add_position(-offset * (1.0f - slide_speed));
	} else if (c1.is_movable) {
		t1->add_position(offset);
	} else {
		t2->add_position(-offset);
	}
}

glm::vec3 CollisionSystem::is_overlap(ColliderOBB &a, ColliderAABB &b) {
	const glm::vec3 distance = a.center - b.center;
	const glm::mat3 o1 = a.get_orientation_matrix();
	const glm::mat3 o2 =
			glm::mat3(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	const glm::vec3 axes[15] = {
		o1[0],
		o1[1],
		o1[2],
		o2[0],
		o2[1],
		o2[2],
		glm::cross(o1[0], o2[0]),
		glm::cross(o1[0], o2[1]),
		glm::cross(o1[0], o2[2]),
		glm::cross(o1[1], o2[0]),
		glm::cross(o1[1], o2[1]),
		glm::cross(o1[1], o2[2]),
		glm::cross(o1[2], o2[0]),
		glm::cross(o1[2], o2[1]),
		glm::cross(o1[2], o2[2]),
	};

	float max_overlap = -std::numeric_limits<float>::max();
	glm::vec3 separation(0.0f);
	for (const glm::vec3 &axis : axes) {
		if (axis == glm::vec3(0.0f)) {
			continue;
		}
		const float overlap = fabs(glm::dot(distance, axis)) -
				(fabs(glm::dot(o1[0] * a.range.x, axis)) + fabs(glm::dot(o1[1] * a.range.y, axis)) +
						fabs(glm::dot(o1[2] * a.range.z, axis)) + fabs(glm::dot(o2[0] * b.range.x, axis)) +
						fabs(glm::dot(o2[1] * b.range.y, axis)) + fabs(glm::dot(o2[2] * b.range.z, axis)));

		if (overlap > 0.0f) {
			return glm::vec3(0.0f);
		}

		if (overlap > max_overlap) {
			max_overlap = overlap;
			separation = axis;
		}
	}

	if (glm::dot(separation, distance) >= 0.0f) {
		separation = -separation;
	}
	return separation * max_overlap;
}

void CollisionSystem::resolve_obb_aabb(Entity obb, Entity aabb) {
	ECSManager &ecs_manager = ECSManager::get();
	ColliderOBB &c1 = ecs_manager.get_component<ColliderOBB>(obb);
	ColliderAABB &c2 = ecs_manager.get_component<ColliderAABB>(aabb);
	Transform *t1;
	Transform *t2;

	if (!(c1.is_movable || c2.is_movable)) {
		return;
	} else if (c1.is_movable && c2.is_movable) {
		t1 = &ecs_manager.get_component<Transform>(obb);
		c1.center = t1->get_position();
		c1.set_orientation(t1->get_euler_rot());

		t2 = &ecs_manager.get_component<Transform>(aabb);
		c2.center = t2->get_position();
	} else if (c1.is_movable) {
		t1 = &ecs_manager.get_component<Transform>(obb);
		c1.center = t1->get_position();
		c1.set_orientation(t1->get_euler_rot());
	} else {
		t2 = &ecs_manager.get_component<Transform>(aabb);
		c2.center = t2->get_position();
	}

	glm::vec3 offset = is_overlap(c1, c2);
	if (offset == glm::vec3(0.0f)) {
		return;
	}

	// Resolve the collision
	if (c1.is_movable && c2.is_movable) {
		// Value defining how fast the sphere will move the OBB
		// 1 means that the sphere has same speed while pushing the OBB
		const float slide_speed = 0.5f;
		t1->add_position(offset * slide_speed);
		t2->add_position(-offset * (1.0f - slide_speed));
	} else if (c1.is_movable) {
		t1->add_position(offset);
	} else {
		t2->add_position(-offset);
	}
}
