#include "physics_manager.h"
#include "ecs/world.h"

PhysicsManager &PhysicsManager::get() {
	static PhysicsManager instance;
	return instance;
}

void PhysicsManager::resolve_collision(World &world, Entity movable_object, const std::set<Entity> &static_entities) {
	CollisionFlag first, second;
	Entity e1 = movable_object;
	if (world.has_component<ColliderOBB>(e1)) {
		first = CollisionFlag::FIRST_OBB;
	} else if (world.has_component<ColliderAABB>(e1)) {
		first = CollisionFlag::FIRST_AABB;
	} else if (world.has_component<ColliderSphere>(e1)) {
		first = CollisionFlag::FIRST_SPHERE;
	} else {
		SPDLOG_WARN("Movable object has invalid collider");
		return;
	}

	for (const Entity e2 : static_entities) {
		if (world.has_component<ColliderOBB>(e2)) {
			second = CollisionFlag::SECOND_OBB;
		} else if (world.has_component<ColliderAABB>(e2)) {
			second = CollisionFlag::SECOND_AABB;
		} else if (world.has_component<ColliderSphere>(e2)) {
			second = CollisionFlag::SECOND_SPHERE;
		} else {
			continue;
		}

		switch (first | second) {
			case CollisionFlag::SPHERE_SPHERE:
				resolve_collision_sphere(world, e1, e2);
				break;
			case CollisionFlag::AABB_AABB:
				resolve_collision_aabb(world, e1, e2);
				break;
			case CollisionFlag::SPHERE_AABB:
				resolve_aabb_sphere(world, e2, e1);
				break;
			case CollisionFlag::AABB_SPHERE:
				resolve_aabb_sphere(world, e1, e2);
				break;
			case CollisionFlag::OBB_OBB:
				resolve_collision_obb(world, e1, e2);
				break;
			case CollisionFlag::SPHERE_OBB:
				resolve_obb_sphere(world, e2, e1);
				break;
			case CollisionFlag::OBB_SPHERE:
				resolve_obb_sphere(world, e1, e2);
				break;
			case CollisionFlag::AABB_OBB:
				resolve_obb_aabb(world, e2, e1);
				break;
			case CollisionFlag::OBB_AABB:
				resolve_obb_aabb(world, e1, e2);
				break;
			default:
				break;
		}
	}
}

bool PhysicsManager::is_overlap(const ColliderSphere &a, const ColliderSphere &b) {
	glm::vec3 distance = a.center - b.center;
	float distance_squared = glm::dot(distance, distance);
	float radius_sum = a.radius + b.radius;

	return distance_squared <= (radius_sum * radius_sum);
}

void PhysicsManager::resolve_collision_sphere(World &world, Entity e1, Entity e2) {
	ColliderSphere &temp_c1 = world.get_component<ColliderSphere>(e1);
	ColliderSphere &temp_c2 = world.get_component<ColliderSphere>(e2);
	Transform &t1 = world.get_component<Transform>(e1);
	Transform &t2 = world.get_component<Transform>(e2);

	ColliderSphere c1;
	ColliderSphere c2;
	c1.radius = temp_c1.radius * t1.get_scale().x;
	c1.center = t1.get_position() + temp_c1.center * c1.radius;
	c2.radius = temp_c2.radius * t2.get_scale().x;
	c2.center = t2.get_position() + temp_c2.center * c2.radius;

	bool is_movable1 = !world.has_component<StaticTag>(e1);
	bool is_movable2 = !world.has_component<StaticTag>(e2);

	if (!(is_movable1 || is_movable2) ||
			!is_collision_candidate(c1.center, glm::vec3(c1.radius), c2.center, glm::vec3(c2.radius)) ||
			!is_overlap(c1, c2)) {
		return;
	}

	const glm::vec3 direction = c2.center - c1.center;
	const float distance = c1.radius + c2.radius - glm::length(direction);
	const glm::vec3 offset = glm::normalize(direction) * distance;

	if (is_movable1 && is_movable2) {
		// Value defining how fast the body will move other body
		// 1 means that the bodies has same speed while pushing
		const float slide_speed = 0.5f;
		t1.add_position(-offset * slide_speed);
		t2.add_position(offset * (1.0f - slide_speed));
	} else if (is_movable1) {
		t1.add_position(-offset);
	} else {
		t2.add_position(offset);
	}
}

bool PhysicsManager::is_overlap(const ColliderAABB &a, const ColliderAABB &b) {
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

void PhysicsManager::resolve_collision_aabb(World &world, Entity e1, Entity e2) {
	ColliderAABB &temp_c1 = world.get_component<ColliderAABB>(e1);
	ColliderAABB &temp_c2 = world.get_component<ColliderAABB>(e2);
	Transform &t1 = world.get_component<Transform>(e1);
	Transform &t2 = world.get_component<Transform>(e2);

	ColliderAABB c1;
	ColliderAABB c2;
	c1.range = temp_c1.range * t1.get_scale();
	c1.center = t1.get_position() + temp_c1.center * c1.range;
	c2.range = temp_c2.range * t2.get_scale();
	c2.center = t2.get_position() + temp_c2.center * c2.range;

	bool is_movable1 = !world.has_component<StaticTag>(e1);
	bool is_movable2 = !world.has_component<StaticTag>(e2);

	if (!(is_movable1 || is_movable2) || !is_collision_candidate(c1.center, c1.range, c2.center, c2.range) ||
			!is_overlap(c1, c2)) {
		return;
	}

	const glm::vec3 min1 = c1.min();
	const glm::vec3 max1 = c1.max();
	const glm::vec3 min2 = c2.min();
	const glm::vec3 max2 = c2.max();

	const float left = max1.x - min2.x, right = max2.x - min1.x;
	const float up = max1.y - min2.y, down = max2.y - min1.y;
	const float front = max1.z - min2.z, back = max2.z - min1.z;

	glm::vec3 offset(0.0f);

	left < right ? offset.x = -left : offset.x = right;
	up < down ? offset.y = -up : offset.y = down;
	front < back ? offset.z = -front : offset.z = back;

	offset.x *offset.x < offset.y *offset.y ? offset.y = 0.0f : offset.x = 0.0f;

	if (offset.x == 0.0f) {
		offset.z *offset.z < offset.y *offset.y ? offset.y = 0.0f : offset.z = 0.0f;
	} else {
		offset.z *offset.z < offset.x *offset.x ? offset.x = 0.0f : offset.z = 0.0f;
	}

	if (is_movable1 && is_movable2) {
		// Value defining how fast the body will move other body
		// 1 means that the bodies has same speed while pushing
		const float slide_speed = 0.5f;
		t1.add_position(offset * slide_speed);
		t2.add_position(-offset * (1.0f - slide_speed));
	} else if (is_movable1) {
		t1.add_position(offset);
	} else {
		t2.add_position(-offset);
	}
}

glm::vec3 PhysicsManager::is_overlap(const ColliderAABB &a, const ColliderSphere &b) {
	const glm::vec3 min = a.min();
	const glm::vec3 max = a.max();
	// Calculate nearest point AABB and Sphere center
	const glm::vec3 closest_point = glm::vec3(std::clamp(b.center.x, min.x, max.x),
			std::clamp(b.center.y, min.y, max.y), std::clamp(b.center.z, min.z, max.z));

	const glm::vec3 direction = closest_point - b.center;
	const float distance = glm::length(direction);

	const float length = b.radius - distance;

	if (distance < b.radius) {
		if (glm::dot(direction, direction) < 0.001f) {
			return glm::vec3(0.0f, 1.0f, 0.0f) * length;
		} else {
			return glm::normalize(direction) * length;
		}
	}

	return glm::vec3(0.0f);
}

void PhysicsManager::resolve_aabb_sphere(World &world, Entity aabb, Entity sphere) {
	ColliderAABB &temp_c1 = world.get_component<ColliderAABB>(aabb);
	ColliderSphere &temp_c2 = world.get_component<ColliderSphere>(sphere);
	Transform &t1 = world.get_component<Transform>(aabb);
	Transform &t2 = world.get_component<Transform>(sphere);

	ColliderAABB c1;
	ColliderSphere c2;
	c1.range = temp_c1.range * t1.get_scale();
	c1.center = t1.get_position() + temp_c1.center * c1.range;
	c2.radius = temp_c2.radius * t2.get_scale().x;
	c2.center = t2.get_position() + temp_c2.center * c2.radius;

	bool is_movable1 = !world.has_component<StaticTag>(aabb);
	bool is_movable2 = !world.has_component<StaticTag>(sphere);
	if (!(is_movable1 || is_movable2) ||
			!is_collision_candidate(c1.center, c1.range, c2.center, glm::vec3(c2.radius))) {
		return;
	}

	glm::vec3 offset = is_overlap(c1, c2);
	if (glm::dot(offset, offset) < 0.001f) {
		return;
	}

	if (is_movable1 && is_movable2) {
		// Value defining how fast the body will move other body
		// 1 means that the bodies has same speed while pushing
		const float slide_speed = 0.5f;
		t1.add_position(offset * slide_speed);
		t2.add_position(-offset * (1.0f - slide_speed));
	} else if (is_movable1) {
		t1.add_position(offset);
	} else {
		t2.add_position(-offset);
	}
}

glm::vec3 PhysicsManager::is_overlap(const ColliderOBB &a, const ColliderOBB &b) {
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
		if (glm::dot(axis, axis) < 0.001f) {
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

	if (glm::dot(separation, distance) > 0.0f) {
		separation = -separation;
	}
	return glm::normalize(separation) * max_overlap;
}

void PhysicsManager::resolve_collision_obb(World &world, Entity e1, Entity e2) {
	ColliderOBB &temp_c1 = world.get_component<ColliderOBB>(e1);
	ColliderOBB &temp_c2 = world.get_component<ColliderOBB>(e2);
	Transform &t1 = world.get_component<Transform>(e1);
	Transform &t2 = world.get_component<Transform>(e2);

	ColliderOBB c1{};
	ColliderOBB c2{};
	c1.set_orientation(t1.get_euler_rot());
	c1.range = temp_c1.range * t1.get_scale();
	c1.center = t1.get_position() + c1.get_orientation_matrix() * (temp_c1.center * c1.range);

	c2.set_orientation(t2.get_euler_rot());
	c2.range = temp_c2.range * t2.get_scale();
	c2.center = t2.get_position() + c2.get_orientation_matrix() * (temp_c2.center * c2.range);

	bool is_movable1 = !world.has_component<StaticTag>(e1);
	bool is_movable2 = !world.has_component<StaticTag>(e2);

	if (!(is_movable1 || is_movable2) || !is_collision_candidate(c1.center, c1.range, c2.center, c2.range)) {
		return;
	}

	glm::vec3 offset = is_overlap(c1, c2);
	if (glm::dot(offset, offset) < 0.001f) {
		return;
	}

	if (is_movable1 && is_movable2) {
		// Value defining how fast the body will move other body
		// 1 means that the bodies has same speed while pushing
		const float slide_speed = 0.5f;
		t1.add_position(offset * slide_speed);
		t2.add_position(-offset * (1.0f - slide_speed));
	} else if (is_movable1) {
		t1.add_position(offset);
	} else {
		t2.add_position(-offset);
	}
}

glm::vec3 PhysicsManager::is_overlap(const ColliderOBB &a, const ColliderSphere &b) {
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
		if (glm::dot(direction, direction) < 0.001f) {
			return glm::vec3(0.0f, 1.0f, 0.0f) * length;
		} else {
			return glm::normalize(direction) * length;
		}
	}

	return glm::vec3(0.0f);
}

void PhysicsManager::resolve_obb_sphere(World &world, Entity obb, Entity sphere) {
	ColliderOBB &temp_c1 = world.get_component<ColliderOBB>(obb);
	ColliderSphere &temp_c2 = world.get_component<ColliderSphere>(sphere);
	Transform &t1 = world.get_component<Transform>(obb);
	Transform &t2 = world.get_component<Transform>(sphere);

	ColliderOBB c1{};
	ColliderSphere c2{};
	c1.set_orientation(t1.get_euler_rot());
	c1.range = temp_c1.range * t1.get_scale();
	c1.center = t1.get_position() + c1.get_orientation_matrix() * (temp_c1.center * c1.range);

	c2.radius = temp_c2.radius * t2.get_scale().x;
	c2.center = t2.get_position() + temp_c2.center * c2.radius;

	bool is_movable1 = !world.has_component<StaticTag>(obb);
	bool is_movable2 = !world.has_component<StaticTag>(sphere);

	if (!(is_movable1 || is_movable2) ||
			!is_collision_candidate(c1.center, c1.range, c2.center, glm::vec3(c2.radius))) {
		return;
	}

	glm::vec3 offset = is_overlap(c1, c2);
	if (glm::dot(offset, offset) < 0.001f) {
		return;
	}

	if (is_movable1 && is_movable2) {
		// Value defining how fast the body will move other body
		// 1 means that the bodies has same speed while pushing
		const float slide_speed = 0.5f;
		t1.add_position(offset * slide_speed);
		t2.add_position(-offset * (1.0f - slide_speed));
	} else if (is_movable1) {
		t1.add_position(offset);
	} else {
		t2.add_position(-offset);
	}
}

glm::vec3 PhysicsManager::is_overlap(const ColliderOBB &a, const ColliderAABB &b) {
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
		if (glm::dot(axis, axis) < 0.001f) {
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

	if (glm::dot(separation, distance) > 0.0f) {
		separation = -separation;
	}

	return glm::normalize(separation) * max_overlap;
}

void PhysicsManager::resolve_obb_aabb(World &world, Entity obb, Entity aabb) {
	ColliderOBB &temp_c1 = world.get_component<ColliderOBB>(obb);
	ColliderAABB &temp_c2 = world.get_component<ColliderAABB>(aabb);
	Transform &t1 = world.get_component<Transform>(obb);
	Transform &t2 = world.get_component<Transform>(aabb);

	ColliderOBB c1{};
	ColliderAABB c2{};
	c1.set_orientation(t1.get_euler_rot());
	c1.range = temp_c1.range * t1.get_scale();
	c1.center = t1.get_position() + c1.get_orientation_matrix() * (temp_c1.center * c1.range);

	c2.range = temp_c2.range * t2.get_scale();
	c2.center = t2.get_position() + temp_c2.center * c2.range;
	bool is_movable1 = !world.has_component<StaticTag>(obb);
	bool is_movable2 = !world.has_component<StaticTag>(aabb);

	if (!(is_movable1 || is_movable2) || !is_collision_candidate(c1.center, c1.range, c2.center, c2.range)) {
		return;
	}

	glm::vec3 offset = is_overlap(c1, c2);
	if (glm::dot(offset, offset) < 0.001f) {
		return;
	}

	if (is_movable1 && is_movable2) {
		// Value defining how fast the body will move other body
		// 1 means that the bodies has same speed while pushing
		const float slide_speed = 0.5f;
		t1.add_position(offset * slide_speed);
		t2.add_position(-offset * (1.0f - slide_speed));
	} else if (is_movable1) {
		t1.add_position(offset);
	} else {
		t2.add_position(-offset);
	}
}

bool PhysicsManager::is_collision_candidate(
		const glm::vec3 &p1, const glm::vec3 &r1, const glm::vec3 &p2, const glm::vec3 &r2) {
	glm::vec3 vector_distance = p1 - p2;
	float distance_squared = glm::dot(vector_distance, vector_distance);

	glm::vec3 r = r1 + r2;
	float range_squared = glm::dot(r, r);

	if (distance_squared >= range_squared) {
		return false;
	}
	return true;
}
