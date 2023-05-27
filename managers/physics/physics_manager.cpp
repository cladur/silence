#include "physics_manager.h"
#include "ecs/world.h"
#include "engine/scene.h"

// Most times we cannot compare float value to 0, than we epsilon
AutoCVarFloat cvar_epsilon("physics.epsilon", "float error value", glm::epsilon<float>(), CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_gravity_x("physics.gravity.x", "gravity value", 0.0f, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_gravity_y("physics.gravity.y", "gravity value", -9.84f, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_gravity_z("physics.gravity.z", "gravity value", 0.0f, CVarFlags::EditFloatDrag);

PhysicsManager &PhysicsManager::get() {
	static PhysicsManager instance;
	return instance;
}

const glm::vec3 &PhysicsManager::get_gravity() {
	gravity = glm::vec3(cvar_gravity_x.get(), cvar_gravity_y.get(), cvar_gravity_z.get());
	return gravity;
}

const float &PhysicsManager::get_epsilon() {
	epsilon = cvar_epsilon.get();
	return epsilon;
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
	} else if (world.has_component<ColliderCapsule>(e1)) {
		first = CollisionFlag::FIRST_CAPSULE;
	} else {
		SPDLOG_WARN("Entity {} has not supported collider", e1);
		return;
	}

	for (const Entity e2 : static_entities) {
		if (world.has_component<ColliderOBB>(e2)) {
			second = CollisionFlag::SECOND_OBB;
		} else if (world.has_component<ColliderAABB>(e2)) {
			second = CollisionFlag::SECOND_AABB;
		} else if (world.has_component<ColliderSphere>(e2)) {
			second = CollisionFlag::SECOND_SPHERE;
		} else if (world.has_component<ColliderCapsule>(e2)) {
			second = CollisionFlag::SECOND_CAPSULE;
		} else {
			SPDLOG_WARN("Entity {} has not supported collider", e2);
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
			case CollisionFlag::CAPSULE_CAPSULE:
				resolve_collision_capsule(world, e1, e2);
				break;
			case CollisionFlag::CAPSULE_SPHERE:
				resolve_capsule_sphere(world, e1, e2);
				break;
			case CollisionFlag::SPHERE_CAPSULE:
				resolve_capsule_sphere(world, e2, e1);
				break;
			case CollisionFlag::CAPSULE_AABB:
				resolve_capsule_aabb(world, e1, e2);
				break;
			case CollisionFlag::AABB_CAPSULE:
				resolve_capsule_aabb(world, e2, e1);
				break;
			case CollisionFlag::CAPSULE_OBB:
				resolve_capsule_obb(world, e1, e2);
				break;
			case CollisionFlag::OBB_CAPSULE:
				resolve_capsule_obb(world, e2, e1);
				break;
			default:
				SPDLOG_WARN("Entities has not supported collision type {} {}", e1, e2);
				break;
		}
	}
}

glm::vec3 PhysicsManager::is_overlap(const ColliderSphere &a, const ColliderSphere &b) {
	glm::vec3 direction = a.center - b.center;
	float distance_squared = glm::length2(direction);
	float radius_sum = a.radius + b.radius;

	float distance = radius_sum - glm::sqrt(distance_squared);
	if (distance_squared < radius_sum * radius_sum) {
		if (glm::length2(direction) <= cvar_epsilon.get()) {
			return glm::vec3(0.0f, 1.0f, 0.0f) * distance;
		} else {
			return glm::normalize(direction) * distance;
		}
	}
	return glm::vec3(0.0f);
}

void PhysicsManager::resolve_collision_sphere(World &world, Entity e1, Entity e2) {
	ColliderSphere &temp_c1 = world.get_component<ColliderSphere>(e1);
	ColliderSphere &temp_c2 = world.get_component<ColliderSphere>(e2);
	Transform &t1 = world.get_component<Transform>(e1);
	Transform &t2 = world.get_component<Transform>(e2);
	ColliderTag &tag1 = world.get_component<ColliderTag>(e1);
	ColliderTag &tag2 = world.get_component<ColliderTag>(e2);
	if (!are_layers_collide(tag1.layer_name, tag2.layer_name)) {
		return;
	}

	ColliderSphere c1;
	ColliderSphere c2;
	const glm::vec3 &scale1 = t1.get_global_scale();
	c1.radius = temp_c1.radius * scale1.x;
	c1.center = t1.get_global_position() + temp_c1.center * scale1.x;

	const glm::vec3 &scale2 = t2.get_global_scale();
	c2.radius = temp_c2.radius * scale2.x;
	c2.center = t2.get_global_position() + temp_c2.center * scale2.x;

	if (!is_collision_candidate(c1.center, glm::vec3(c1.radius), c2.center, glm::vec3(c2.radius))) {
		return;
	}
	glm::vec3 offset = is_overlap(c1, c2);
	if (glm::length2(offset) <= cvar_epsilon.get()) {
		return;
	}

	make_shift(world, e1, e2, offset);
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
	//todo: replace calculate offset into proper is_overlap
	ColliderAABB &temp_c1 = world.get_component<ColliderAABB>(e1);
	ColliderAABB &temp_c2 = world.get_component<ColliderAABB>(e2);
	Transform &t1 = world.get_component<Transform>(e1);
	Transform &t2 = world.get_component<Transform>(e2);
	ColliderTag &tag1 = world.get_component<ColliderTag>(e1);
	ColliderTag &tag2 = world.get_component<ColliderTag>(e2);
	if (!are_layers_collide(tag1.layer_name, tag2.layer_name)) {
		return;
	}

	ColliderAABB c1;
	ColliderAABB c2;
	const glm::vec3 &scale1 = t1.get_global_scale();
	c1.range = temp_c1.range * scale1;
	c1.center = t1.get_global_position() + temp_c1.center * scale1;

	const glm::vec3 &scale2 = t2.get_global_scale();
	c2.range = temp_c2.range * scale2;
	c2.center = t2.get_global_position() + temp_c2.center * scale2;

	if (!is_collision_candidate(c1.center, c1.range, c2.center, c2.range) || !is_overlap(c1, c2)) {
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

	make_shift(world, e1, e2, offset);
}

glm::vec3 PhysicsManager::is_overlap(const ColliderAABB &a, const ColliderSphere &b) {
	const glm::vec3 min = a.min();
	const glm::vec3 max = a.max();
	// Calculate nearest point AABB and Sphere center
	const glm::vec3 closest_point = glm::clamp(b.center, min, max);
	const glm::vec3 direction = closest_point - b.center;
	const float distance2 = glm::length2(direction);

	if (distance2 < b.radius * b.radius) {
		const float length = b.radius - glm::sqrt(distance2);
		if (glm::length2(direction) <= cvar_epsilon.get()) {
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
	ColliderTag &tag1 = world.get_component<ColliderTag>(aabb);
	ColliderTag &tag2 = world.get_component<ColliderTag>(sphere);
	if (!are_layers_collide(tag1.layer_name, tag2.layer_name)) {
		return;
	}

	ColliderAABB c1;
	ColliderSphere c2;
	const glm::vec3 &scale1 = t1.get_global_scale();
	c1.range = temp_c1.range * scale1;
	c1.center = t1.get_global_position() + temp_c1.center * scale1;

	const glm::vec3 &scale2 = t2.get_global_scale();
	c2.radius = temp_c2.radius * scale2.x;
	c2.center = t2.get_global_position() + temp_c2.center * scale2.x;

	if (!is_collision_candidate(c1.center, c1.range, c2.center, glm::vec3(c2.radius))) {
		return;
	}

	glm::vec3 offset = is_overlap(c1, c2);
	if (glm::length2(offset) <= cvar_epsilon.get()) {
		return;
	}

	make_shift(world, aabb, sphere, offset);
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
		if (glm::length2(axis) <= cvar_epsilon.get()) {
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
	ColliderTag &tag1 = world.get_component<ColliderTag>(e1);
	ColliderTag &tag2 = world.get_component<ColliderTag>(e2);
	if (!are_layers_collide(tag1.layer_name, tag2.layer_name)) {
		return;
	}

	ColliderOBB c1{};
	ColliderOBB c2{};
	const glm::vec3 &scale1 = t1.get_global_scale();
	const glm::quat &orientation1 = t1.get_global_orientation();
	temp_c1.set_orientation(orientation1);
	c1.orientation[0] = temp_c1.orientation[0];
	c1.orientation[1] = temp_c1.orientation[1];
	c1.range = temp_c1.range * scale1;
	c1.center = t1.get_global_position() + orientation1 * (temp_c1.center * scale1);

	const glm::vec3 &scale2 = t2.get_global_scale();
	const glm::quat &orientation2 = t2.get_global_orientation();
	temp_c2.set_orientation(orientation2);
	c2.orientation[0] = temp_c2.orientation[0];
	c2.orientation[1] = temp_c2.orientation[1];
	c2.range = temp_c2.range * scale2;
	c2.center = t2.get_global_position() + orientation2 * (temp_c2.center * scale2);

	if (!is_collision_candidate(c1.center, c1.range, c2.center, c2.range)) {
		return;
	}

	glm::vec3 offset = is_overlap(c1, c2);
	if (glm::length2(offset) <= cvar_epsilon.get()) {
		return;
	}

	make_shift(world, e1, e2, offset);
}

glm::vec3 PhysicsManager::is_overlap(const ColliderOBB &a, const ColliderSphere &b) {
	const glm::mat3 &orientation = a.get_orientation_matrix();
	const glm::vec3 &sphere_center_local = glm::transpose(orientation) * (b.center - a.center);

	glm::vec3 d = b.center - a.center;
	glm::vec3 point = a.center;

	for (int i = 0; i < 3; i++) {
		float dist = glm::dot(d, orientation[i]);

		if (dist > a.range[i]) {
			dist = a.range[i];
		}
		if (dist < -a.range[i]) {
			dist = -a.range[i];
		}

		point += dist * orientation[i];
	}

	// Calculate distance between b center and closest point on a
	const glm::vec3 &direction = point - b.center;
	const float distance2 = glm::length2(direction);

	if (distance2 < b.radius * b.radius) {
		const float length = b.radius - glm::sqrt(distance2);
		if (glm::length2(direction) <= cvar_epsilon.get()) {
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
	ColliderTag &tag1 = world.get_component<ColliderTag>(obb);
	ColliderTag &tag2 = world.get_component<ColliderTag>(sphere);
	if (!are_layers_collide(tag1.layer_name, tag2.layer_name)) {
		return;
	}

	ColliderOBB c1{};
	ColliderSphere c2{};
	const glm::vec3 &scale1 = t1.get_global_scale();
	const glm::quat &orientation1 = t1.get_global_orientation();
	temp_c1.set_orientation(orientation1);
	c1.orientation[0] = temp_c1.orientation[0];
	c1.orientation[1] = temp_c1.orientation[1];
	c1.range = temp_c1.range * scale1;
	c1.center = t1.get_global_position() + orientation1 * (temp_c1.center * scale1);

	const glm::vec3 &scale2 = t2.get_global_scale();
	c2.radius = temp_c2.radius * scale2.x;
	c2.center = t2.get_global_position() + temp_c2.center * scale2.x;

	if (!is_collision_candidate(c1.center, c1.range, c2.center, glm::vec3(c2.radius))) {
		return;
	}

	glm::vec3 offset = is_overlap(c1, c2);
	if (glm::length2(offset) <= cvar_epsilon.get()) {
		return;
	}

	make_shift(world, obb, sphere, offset);
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
		if (glm::length2(axis) <= cvar_epsilon.get()) {
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
	ColliderTag &tag1 = world.get_component<ColliderTag>(obb);
	ColliderTag &tag2 = world.get_component<ColliderTag>(aabb);
	if (!are_layers_collide(tag1.layer_name, tag2.layer_name)) {
		return;
	}

	ColliderOBB c1{};
	ColliderAABB c2{};
	const glm::vec3 &scale1 = t1.get_global_scale();
	const glm::quat &orientation1 = t1.get_global_orientation();
	temp_c1.set_orientation(orientation1);
	c1.orientation[0] = temp_c1.orientation[0];
	c1.orientation[1] = temp_c1.orientation[1];
	c1.range = temp_c1.range * scale1;
	c1.center = t1.get_global_position() + orientation1 * (temp_c1.center * scale1);

	const glm::vec3 &scale2 = t2.get_global_scale();
	c2.range = temp_c2.range * scale2;
	c2.center = t2.get_global_position() + temp_c2.center * scale2;

	if (!is_collision_candidate(c1.center, c1.range, c2.center, c2.range)) {
		return;
	}

	glm::vec3 offset = is_overlap(c1, c2);
	if (glm::length2(offset) <= cvar_epsilon.get()) {
		return;
	}

	make_shift(world, obb, aabb, offset);
}

glm::vec3 PhysicsManager::is_overlap(const ColliderCapsule &a, const ColliderCapsule &b) {
	float offset1, offset2;
	glm::vec3 capsule_point1, capsule_point2;

	float distance2;
	const glm::vec3 &d1 = a.end - a.start;
	const glm::vec3 &d2 = b.end - b.start;
	const glm::vec3 &r = a.start - b.start;
	const float l1 = glm::length2(d1);
	const float l2 = glm::length2(d2);
	const float f = glm::dot(d2, r);
	if (l1 <= cvar_epsilon.get() && l2 <= cvar_epsilon.get()) {
		offset1 = 0.0f;
		offset2 = 0.0f;
	} else if (l1 <= cvar_epsilon.get()) {
		offset1 = 0.0f;
		offset2 = std::clamp(f / l2, 0.0f, 1.0f);
	} else {
		float c = glm::dot(d1, r);
		if (l2 <= cvar_epsilon.get()) {
			offset1 = std::clamp(-c / l1, 0.0f, 1.0f);
			offset2 = 0.0f;
		} else {
			float l = glm::dot(d1, d2);
			float denom = l1 * l2 - l * l;

			if (denom != 0.0f) {
				offset1 = std::clamp((l * f - c * l2) / denom, 0.0f, 1.0f);
			} else {
				offset1 = 0.0f;
			}

			float tnom = l * offset1 + f;
			if (tnom < 0.0f) {
				offset1 = std::clamp(-c / l1, 0.0f, 1.0f);
				offset2 = 0.0f;
			} else if (tnom > l2) {
				offset1 = std::clamp((l - c) / l1, 0.0f, 1.0f);
				offset2 = 1.0f;
			} else {
				offset2 = tnom / l2;
			}
		}
	}
	capsule_point1 = a.start + d1 * offset1;
	capsule_point2 = b.start + d2 * offset2;
	distance2 = glm::length2(capsule_point1 - capsule_point2);

	float radius = a.radius + b.radius;
	if (distance2 < radius * radius) {
		float length = glm::sqrt(distance2) - radius;
		if (distance2 < cvar_epsilon.get()) {
			return glm::vec3(0.0f, 1.0f, 0.0f) * length;
		} else {
			return glm::normalize(capsule_point2 - capsule_point1) * length;
		}
	}

	return glm::vec3(0.0f);
}

void PhysicsManager::resolve_collision_capsule(World &world, Entity e1, Entity e2) {
	ColliderCapsule &temp_c1 = world.get_component<ColliderCapsule>(e1);
	ColliderCapsule &temp_c2 = world.get_component<ColliderCapsule>(e2);
	Transform &t1 = world.get_component<Transform>(e1);
	Transform &t2 = world.get_component<Transform>(e2);
	ColliderTag &tag1 = world.get_component<ColliderTag>(e1);
	ColliderTag &tag2 = world.get_component<ColliderTag>(e2);
	if (!are_layers_collide(tag1.layer_name, tag2.layer_name)) {
		return;
	}

	ColliderCapsule c1{};
	ColliderCapsule c2{};
	const glm::vec3 &scale1 = t1.get_global_scale();
	const glm::vec3 &position1 = t1.get_global_position();
	c1.radius = temp_c1.radius * scale1.x;
	c1.start = position1 + temp_c1.start * scale1;
	c1.end = position1 + temp_c1.end * scale1;

	const glm::vec3 &scale2 = t2.get_global_scale();
	const glm::vec3 &position2 = t2.get_global_position();
	c2.radius = temp_c2.radius * scale2.x;
	c2.start = position2 + temp_c2.start * scale2;
	c2.end = position2 + temp_c2.end * scale2;

	glm::vec3 offset = is_overlap(c1, c2);
	if (glm::length2(offset) < cvar_epsilon.get()) {
		return;
	}

	make_shift(world, e1, e2, offset);
}

glm::vec3 PhysicsManager::is_overlap(const ColliderCapsule &a, const ColliderSphere &b) {
	glm::vec3 c1, c2;

	glm::vec3 d = a.end - a.start;
	glm::vec3 r = a.start - b.center;
	c2 = b.center;
	float d2 = glm::length2(d);
	if (d2 <= cvar_epsilon.get()) {
		c1 = a.start;
	} else {
		float c = glm::dot(d, r);
		float s = std::clamp(-c / d2, 0.0f, 1.0f);
		c1 = a.start + d * s;
	}

	float dist2 = glm::length2(c1 - c2);

	float radius = a.radius + b.radius;
	if (dist2 < radius * radius) {
		float length = (glm::sqrt(dist2) - radius);
		if (dist2 <= cvar_epsilon.get()) {
			return glm::vec3(0.0f, 1.0f, 0.0f) * length;
		} else {
			return glm::normalize(c2 - c1) * length;
		}
	}

	return glm::vec3(0.0f);
}

void PhysicsManager::resolve_capsule_sphere(World &world, Entity capsule, Entity sphere) {
	ColliderCapsule &temp_c1 = world.get_component<ColliderCapsule>(capsule);
	ColliderSphere &temp_c2 = world.get_component<ColliderSphere>(sphere);
	Transform &t1 = world.get_component<Transform>(capsule);
	Transform &t2 = world.get_component<Transform>(sphere);
	ColliderTag &tag1 = world.get_component<ColliderTag>(capsule);
	ColliderTag &tag2 = world.get_component<ColliderTag>(sphere);
	if (!are_layers_collide(tag1.layer_name, tag2.layer_name)) {
		return;
	}

	ColliderCapsule c1{};
	ColliderSphere c2{};
	const glm::vec3 &scale1 = t1.get_global_scale();
	const glm::vec3 &position1 = t1.get_global_position();
	c1.radius = temp_c1.radius * scale1.x;
	c1.start = position1 + temp_c1.start * scale1;
	c1.end = position1 + temp_c1.end * scale1;

	const glm::vec3 &scale2 = t2.get_global_scale();
	c2.radius = temp_c2.radius * scale2.x;
	c2.center = t2.get_global_position() + temp_c2.center * scale2;

	glm::vec3 offset = is_overlap(c1, c2);
	if (glm::length2(offset) <= cvar_epsilon.get()) {
		return;
	}

	make_shift(world, capsule, sphere, offset);
}

glm::vec3 PhysicsManager::is_overlap(const ColliderCapsule &a, const ColliderAABB &b) {
	// Find the closest point on the segment to the AABB
	glm::vec3 capsule_point;

	glm::vec3 d = a.end - a.start;
	float d2 = glm::length2(d);
	if (d2 <= cvar_epsilon.get()) {
		capsule_point = a.start;
	} else {
		const glm::vec3 min = b.min();
		const glm::vec3 max = b.max();
		d = glm::normalize(d);
		float nearest_distance = 0.0f;

		for (int i = 0; i < 3; i++) {
			if (glm::abs(d[i]) <= cvar_epsilon.get()) {
				continue;
			}
			float ood = 1.0f / d[i];
			float t1 = (min[i] - a.start[i]) * ood;
			float t2 = (max[i] - a.start[i]) * ood;

			if (t1 > t2) {
				std::swap(t1, t2);
			}

			if (t1 > nearest_distance) {
				nearest_distance = t1;
			}
		}

		capsule_point = a.start + d * glm::clamp(nearest_distance, 0.0f, glm::sqrt(d2));
	}

	ColliderSphere sphere;
	sphere.center = capsule_point;
	sphere.radius = a.radius;

	return -is_overlap(b, sphere);
}

void PhysicsManager::resolve_capsule_aabb(World &world, Entity capsule, Entity aabb) {
	ColliderCapsule &temp_c1 = world.get_component<ColliderCapsule>(capsule);
	ColliderAABB &temp_c2 = world.get_component<ColliderAABB>(aabb);
	Transform &t1 = world.get_component<Transform>(capsule);
	Transform &t2 = world.get_component<Transform>(aabb);
	ColliderTag &tag1 = world.get_component<ColliderTag>(capsule);
	ColliderTag &tag2 = world.get_component<ColliderTag>(aabb);
	if (!are_layers_collide(tag1.layer_name, tag2.layer_name)) {
		return;
	}

	ColliderCapsule c1{};
	ColliderAABB c2{};
	const glm::vec3 &scale1 = t1.get_global_scale();
	const glm::vec3 &position1 = t1.get_global_position();
	c1.radius = temp_c1.radius * scale1.x;
	c1.start = position1 + temp_c1.start * scale1;
	c1.end = position1 + temp_c1.end * scale1;

	const glm::vec3 &scale2 = t2.get_global_scale();
	c2.range = temp_c2.range * scale2;
	c2.center = t2.get_global_position() + temp_c2.center * scale2;

	glm::vec3 offset = is_overlap(c1, c2);
	if (glm::length2(offset) <= cvar_epsilon.get()) {
		return;
	}

	make_shift(world, capsule, aabb, offset);
}

glm::vec3 PhysicsManager::is_overlap(const ColliderCapsule &a, const ColliderOBB &b) {
	glm::vec3 capsule_point;

	glm::vec3 d = a.end - a.start;
	float d2 = glm::length2(d);
	if (d2 <= cvar_epsilon.get()) {
		capsule_point = a.start;
	} else {
		const glm::mat3 &o = glm::inverse(b.get_orientation_matrix());
		d = glm::normalize(d);
		const glm::vec3 &local_start = o * (a.start - b.center);
		const glm::vec3 &local_direction = o * d;

		float nearest_distance = 0.0f;

		for (int i = 0; i < 3; i++) {
			if (glm::abs(local_direction[i]) <= cvar_epsilon.get()) {
				continue;
			}

			float ood = 1.0f / local_direction[i];
			float t1 = (-b.range[i] - local_start[i]) * ood;
			float t2 = (b.range[i] - local_start[i]) * ood;

			if (t1 > t2) {
				std::swap(t1, t2);
			}

			if (t1 > nearest_distance) {
				nearest_distance = t1;
			}
		}

		capsule_point = a.start + d * glm::clamp(nearest_distance, 0.0f, glm::sqrt(d2));
	}

	ColliderSphere sphere;
	sphere.center = capsule_point;
	sphere.radius = a.radius;
	return -is_overlap(b, sphere);
}

void PhysicsManager::resolve_capsule_obb(World &world, Entity capsule, Entity obb) {
	ColliderCapsule &temp_c1 = world.get_component<ColliderCapsule>(capsule);
	ColliderOBB &temp_c2 = world.get_component<ColliderOBB>(obb);
	Transform &t1 = world.get_component<Transform>(capsule);
	Transform &t2 = world.get_component<Transform>(obb);
	ColliderTag &tag1 = world.get_component<ColliderTag>(capsule);
	ColliderTag &tag2 = world.get_component<ColliderTag>(obb);
	if (!are_layers_collide(tag1.layer_name, tag2.layer_name)) {
		return;
	}

	ColliderCapsule c1{};
	ColliderOBB c2{};
	const glm::vec3 &scale1 = t1.get_global_scale();
	const glm::vec3 &position1 = t1.get_global_position();
	c1.radius = temp_c1.radius * scale1.x;
	c1.start = position1 + temp_c1.start * scale1;
	c1.end = position1 + temp_c1.end * scale1;

	const glm::quat &orientation2 = t2.get_global_orientation();
	const glm::vec3 &scale2 = t2.get_global_scale();
	temp_c2.set_orientation(orientation2);
	c2.orientation[0] = temp_c2.orientation[0];
	c2.orientation[1] = temp_c2.orientation[1];
	c2.range = temp_c2.range * scale2;
	c2.center = t2.get_global_position() + orientation2 * (temp_c2.center * scale2);

	glm::vec3 offset = is_overlap(c1, c2);
	if (glm::length2(offset) <= cvar_epsilon.get()) {
		return;
	}

	make_shift(world, capsule, obb, offset);
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

void PhysicsManager::make_shift(World &world, Entity e1, Entity e2, const glm::vec3 &offset) {
	bool is_movable1 = !world.has_component<StaticTag>(e1);
	bool is_movable2 = !world.has_component<StaticTag>(e2);
	Transform &t1 = world.get_component<Transform>(e1);
	Transform &t2 = world.get_component<Transform>(e2);

	if (world.has_component<RigidBody>(e1) && world.has_component<RigidBody>(e2)) {
		RigidBody &b1 = world.get_component<RigidBody>(e1);
		RigidBody &b2 = world.get_component<RigidBody>(e2);
		physical_shift(t1, t2, b1, b2, is_movable1, is_movable2, offset);
	} else if (world.has_component<RigidBody>(e1)) {
		RigidBody &b1 = world.get_component<RigidBody>(e1);
		RigidBody b2{};
		physical_shift(t1, t2, b1, b2, is_movable1, is_movable2, offset);
	} else if (world.has_component<RigidBody>(e2)) {
		RigidBody b1{};
		RigidBody &b2 = world.get_component<RigidBody>(e2);
		physical_shift(t1, t2, b1, b2, is_movable1, is_movable2, offset);
	} else {
		non_physical_shift(t1, t2, is_movable1, is_movable2, offset);
	}
}

void PhysicsManager::non_physical_shift(
		Transform &t1, Transform &t2, bool is_movable1, bool is_movable2, const glm::vec3 &offset) {
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

void PhysicsManager::physical_shift(Transform &t1, Transform &t2, RigidBody &b1, RigidBody &b2, bool is_movable1,
		bool is_movable2, const glm::vec3 &offset) {
	glm::vec3 velocity_projection1;
	glm::vec3 velocity_projection2;
	glm::vec3 collision_normal1 = glm::normalize(offset);
	glm::vec3 collision_normal2 = -collision_normal1;

	if (is_movable1) {
		float projection1 = glm::dot(b1.velocity, collision_normal1) / glm::dot(collision_normal1, collision_normal1);
		velocity_projection1 = collision_normal1 * projection1;
	}
	if (is_movable2) {
		float projection2 = glm::dot(b2.velocity, collision_normal2) / glm::dot(collision_normal2, collision_normal2);
		velocity_projection2 = collision_normal2 * projection2;
	}

	if (is_movable1 && is_movable2) {
		const float slide_speed = b1.mass / (b1.mass + b2.mass);
		// Value defining how fast the body will move other body
		// 1 means that the bodies has same speed while pushing
		t1.add_position(offset * slide_speed);
		t2.add_position(-offset * (1.0f - slide_speed));
		b1.velocity -= velocity_projection1;
		b2.velocity -= velocity_projection2;
	} else if (is_movable1) {
		t1.add_position(offset);
		b1.velocity -= velocity_projection1;
	} else {
		t2.add_position(-offset);
		b2.velocity -= velocity_projection2;
	}
}

bool PhysicsManager::intersect_ray_sphere(const Ray &ray, const ColliderSphere &sphere, HitInfo &result) {
	glm::vec3 m = ray.origin - sphere.center;
	float b = glm::dot(m, ray.direction);
	float c = glm::length2(m) - sphere.radius * sphere.radius;
	// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0)
	if (c > 0.0f && b > 0.0f) {
		return false;
	}
	float discriminant = b * b - c;
	// A negative discriminant corresponds to ray missing sphere
	if (discriminant < 0.0f) {
		return false;
	}
	// Ray now found to intersect sphere, compute smallest t value of intersection
	float distance = -b - sqrt(discriminant);
	// If t is negative, ray started inside sphere so clamp t to zero
	if (distance < 0.0f) {
		distance = 0.0f;
	}
	result.distance = distance;
	result.point = ray.origin + distance * ray.direction;
	result.normal = glm::normalize(result.point - sphere.center);
	return true;
}

bool PhysicsManager::intersect_ray_aabb(const Ray &ray, const ColliderAABB &aabb, HitInfo &result) {
	float nearest_distance = 0.0f;
	float ray_range = std::numeric_limits<float>::max();

	const glm::vec3 &min = aabb.min();
	const glm::vec3 &max = aabb.max();

	glm::vec3 hit_normal(0.0f);

	for (int i = 0; i < 3; i++) {
		if (glm::abs(ray.direction[i]) <= cvar_epsilon.get()) {
			if (ray.origin[i] < min[i] || ray.origin[i] > max[i]) {
				return false;
			}
			continue;
		}

		float ood = 1.0f / ray.direction[i];
		float t1 = (min[i] - ray.origin[i]) * ood;
		float t2 = (max[i] - ray.origin[i]) * ood;
		if (t1 > t2) {
			std::swap(t1, t2);
		}

		if (t1 > nearest_distance) {
			nearest_distance = t1;
			hit_normal = glm::vec3(0.0f);
			hit_normal[i] = -1.0f;
		}

		if (t2 < ray_range) {
			ray_range = t2;
			hit_normal = glm::vec3(0.0f);
			hit_normal[i] = 1.0f;
		}

		if (ray_range < nearest_distance) {
			return false;
		}
	}

	if (glm::dot(hit_normal, ray.direction) > 0.0f) {
		hit_normal = -hit_normal;
	}

	result.distance = nearest_distance;
	result.point = ray.origin + ray.direction * nearest_distance;
	result.normal = hit_normal;

	return true;
}

bool PhysicsManager::intersect_ray_obb(const Ray &ray, const ColliderOBB &obb, HitInfo &result) {
	const glm::mat3 &o = obb.get_orientation_matrix();
	const glm::mat3 &inv_o = glm::inverse(o);

	Ray local_ray{};
	local_ray.origin = inv_o * (ray.origin - obb.center);
	local_ray.direction = inv_o * ray.direction;
	glm::vec3 hit_normal(0.0f);

	float nearest_distance = 0.0f;
	float ray_range = std::numeric_limits<float>::max();

	for (int i = 0; i < 3; i++) {
		if (glm::abs(local_ray.direction[i]) <= cvar_epsilon.get()) {
			if (local_ray.origin[i] < -obb.range[i] || local_ray.origin[i] > obb.range[i]) {
				return false;
			}
			continue;
		}

		float ood = 1.0f / local_ray.direction[i];
		float t1 = (-obb.range[i] - local_ray.origin[i]) * ood;
		float t2 = (obb.range[i] - local_ray.origin[i]) * ood;

		if (t1 > t2) {
			std::swap(t1, t2);
		}

		if (t1 > nearest_distance) {
			nearest_distance = t1;
			hit_normal = o[i];
		}

		if (t2 < ray_range) {
			ray_range = t2;
			// This thing made bug
			//	hit_normal = o[i];
		}

		if (nearest_distance > ray_range) {
			return false;
		}
	}

	if (glm::dot(hit_normal, ray.direction) > 0.0f) {
		hit_normal = -hit_normal;
	}

	result.distance = nearest_distance;
	result.point = ray.origin + ray.direction * nearest_distance;
	result.normal = hit_normal;

	return true;
}

bool PhysicsManager::intersect_ray_capsule(const Ray &ray, const ColliderCapsule &capsule, HitInfo &result) {
	Segment segment;
	segment.start = ray.origin;
	// this is a secret value, bcs it's almost infinity, to imitate ray
	segment.end = ray.origin + ray.direction * 694202137.8f;
	return intersect_segment_capsule(segment, capsule, result);
}

bool PhysicsManager::intersect_segment_capsule(
		const Segment &segment, const ColliderCapsule &capsule, HitInfo &result) {
	Ray ray;
	ray.origin = segment.start;
	ray.direction = segment.end - segment.start;
	if (glm::length2(ray.direction) > 0.0f) {
		ray.direction = glm::normalize(ray.direction);
	}

	ColliderSphere start{};
	start.center = capsule.start;
	start.radius = capsule.radius;
	if (intersect_ray_sphere(ray, start, result)) {
		return true;
	}

	ColliderSphere end{};
	end.center = capsule.end;
	end.radius = capsule.radius;
	if (intersect_ray_sphere(ray, end, result)) {
		return true;
	}

	float distance;
	glm::vec3 d = capsule.end - capsule.start, m = ray.origin - capsule.start, n = segment.end - segment.start;
	float md = glm::dot(m, d);
	float nd = glm::dot(n, d);
	float dd = glm::length2(d);
	// Test if segment fully outside either endcap of cylinder
	if (md < 0.0f && md + nd < 0.0f) {
		return false; // Segment outside ’p’ side of cylinder
	}
	if (md > dd && md + nd > dd) {
		return false; // Segment outside ’q’ side of cylinder
	}
	float nn = glm::length2(n);
	float mn = glm::dot(m, n);
	float a = dd * nn - nd * nd;
	float k = glm::length2(m) - capsule.radius * capsule.radius;
	float c = dd * k - md * md;
	if (glm::abs(a) <= cvar_epsilon.get()) {
		// Segment runs parallel to cylinder axis
		if (c > 0.0f) {
			return false; // ’a’ and thus the segment lie outside cylinder
		}
		// Now known that segment intersects cylinder; figure out how it intersects
		if (md < 0.0f) {
			distance = -mn / nn; // Intersect segment against ’p’ endcap
		} else if (md > dd) {
			distance = (nd - mn) / nn; // Intersect segment against ’q’ endcap
		} else {
			distance = 0.0f; // ’a’ lies inside cylinder
		}
		result.distance = distance;
		result.normal = -ray.direction;
		result.point = ray.origin + ray.direction * distance;
		return true;
	}
	float b = dd * mn - nd * md;
	float discr = b * b - a * c;
	if (discr < 0.0f) {
		return false; // No real roots; no intersection
	}
	distance = (-b - glm::sqrt(discr)) / a;
	if (distance < 0.0f || distance > 1.0f) {
		return false; // Intersection lies outside segment
	} else if (md + distance * nd < 0.0f) {
		// Intersection outside cylinder on ’p’ side
		if (nd <= 0.0f) {
			return false; // Segment pointing away from endcap
		}
		distance = -md / nd;
		// Keep intersection if Dot(S(t) - p, S(t) - p) <= r∧2
		if (k + 2 * distance * (mn + distance * nn) <= 0.0f) {
			result.distance = distance;
			result.normal = -ray.direction;
			result.point = ray.origin + ray.direction * distance;
			return true;
		} else {
			return false;
		}
	} else if (md + distance * nd > dd) {
		// Intersection outside cylinder on ’q’ side
		if (nd >= 0.0f) {
			return false; // Segment pointing away from endcap
		}
		distance = (dd - md) / nd;
		// Keep intersection if Dot(S(t) - q, S(t) - q) <= r∧2
		if (k + dd - 2 * md + distance * (2 * (mn - nd) + distance * nn) <= 0.0f) {
			result.distance = distance;
			result.normal = -ray.direction;
			result.point = ray.origin + ray.direction * distance;
			return true;
		} else {
			return false;
		}
	}
	// Segment intersects cylinder between the endcaps; t is correct
	result.distance = distance;
	result.normal = -ray.direction;
	result.point = ray.origin + ray.direction * distance;
	return true;
}

void PhysicsManager::add_collision_layer(const std::string &layer_name) {
	if (layers_map.find(layer_name) == layers_map.end()) {
		layers_map[layer_name] = std::unordered_set<std::string>();
		return;
	}
	SPDLOG_INFO("Layer {} is already exist.", layer_name);
}

void PhysicsManager::remove_collision_layer(const std::string &layer_name) {
	layers_map.erase(layer_name);
	for (auto &pair : layers_map) {
		pair.second.erase(layer_name);
	}
}

void PhysicsManager::set_layers_no_collision(const std::string &layer1, const std::string &layer2) {
	if (layer1 == layer2) {
		SPDLOG_WARN("Layers have equal name");
		return;
	}

	const auto &it1 = layers_map.find(layer1);
	const auto &it2 = layers_map.find(layer2);
	if (it1 == layers_map.end()) {
		SPDLOG_WARN("Layer name not found: {}", layer1);
		return;
	}
	if (it2 == layers_map.end()) {
		SPDLOG_WARN("Layer name not found: {}", layer2);
		return;
	}
	it1->second.insert(layer2);
	it2->second.insert(layer1);
}

void PhysicsManager::set_layers_collision(const std::string &layer1, const std::string &layer2) {
	if (layer1 == layer2) {
		SPDLOG_WARN("Layers have equal name");
		return;
	}

	const auto &it1 = layers_map.find(layer1);
	const auto &it2 = layers_map.find(layer2);
	if (it1 == layers_map.end()) {
		SPDLOG_WARN("Layer name not found: {}", layer1);
		return;
	}
	if (it2 == layers_map.end()) {
		SPDLOG_WARN("Layer name not found: {}", layer2);
		return;
	}
	it1->second.erase(layer2);
	it2->second.erase(layer1);
}

bool PhysicsManager::are_layers_collide(const std::string &layer1, const std::string &layer2) {
	if (layer1 == layer2) {
		return true;
	}

	const auto &it1 = layers_map.find(layer1);

	if (it1->second.contains(layer2)) {
		return false;
	}

	return true;
}

const std::unordered_map<std::string, std::unordered_set<std::string>> &PhysicsManager::get_layers_map() {
	return layers_map;
}

std::vector<Entity> PhysicsManager::overlap_sphere(
		World &world, const ColliderSphere &sphere, const std::string &layer_name) {
	std::vector<Entity> entities = world.get_parent_scene()->entities;
	std::vector<Entity> result;
	// Reserve for optimization I didn't expect that more than 20 enemies will be in range
	result.reserve(32);

	for (auto entity : entities) {
		if (!world.has_component<ColliderTag>(entity) || !world.has_component<Transform>(entity)) {
			continue;
		}

		auto &tag = world.get_component<ColliderTag>(entity);
		if (!are_layers_collide(layer_name, tag.layer_name)) {
			continue;
		}
		const Transform &transform = world.get_component<Transform>(entity);
		const glm::vec3 &position = transform.get_global_position();
		const glm::vec3 &scale = transform.get_global_scale();
		HitInfo info;
		info.entity = entity;
		if (world.has_component<ColliderAABB>(entity)) {
			ColliderAABB c = world.get_component<ColliderAABB>(entity);
			c.center = position + c.center * scale;
			c.range *= scale;
			if (glm::length2(is_overlap(c, sphere)) > 0.0f) {
				result.push_back(entity);
			}
		} else if (world.has_component<ColliderOBB>(entity)) {
			ColliderOBB c = world.get_component<ColliderOBB>(entity);
			c.center = position + c.get_orientation_matrix() * c.center * scale;
			c.range *= scale;
			if (glm::length2(is_overlap(c, sphere)) > 0.0f) {
				result.push_back(entity);
			}
		} else if (world.has_component<ColliderSphere>(entity)) {
			ColliderSphere c = world.get_component<ColliderSphere>(entity);
			c.center = position + c.center * scale;
			c.radius *= scale.x;
			if (glm::length2(is_overlap(c, sphere)) > 0.0f) {
				result.push_back(entity);
			}
		} else if (world.has_component<ColliderCapsule>(entity)) {
			ColliderCapsule c = world.get_component<ColliderCapsule>(entity);
			c.start = position + c.start * scale;
			c.end = position + c.end * scale;
			c.radius *= scale.x;
			if (glm::length2(is_overlap(c, sphere)) > 0.0f) {
				result.push_back(entity);
			}
		}
	}
	return result;
}
