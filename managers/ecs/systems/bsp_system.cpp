#include "bsp_system.h"

#include "collision_system.h"
#include "components/collider_tag_component.h"
#include "components/static_tag_component.h"
#include "ecs/ecs_manager.h"

extern ECSManager ecs_manager;

void BSPSystem::startup() {
	Signature signature;
	signature.set(ecs_manager.get_component_type<ColliderTag>());
	signature.set(ecs_manager.get_component_type<StaticTag>());
	ecs_manager.set_system_component_whitelist<BSPSystem>(signature);
}

void BSPSystem::update(CollisionSystem &collision_system) {
	for (const Entity entity : collision_system.entities) {
		resolve_collision(root.get(), entity, collision_system);
	}
}

void BSPSystem::resolve_collision(BSPNode *node, Entity entity, CollisionSystem &collision_system, bool force) {
	if (node == nullptr) {
		return;
	}
	collision_system.resolve_collision(entity, node->entities);

	if (force) {
		resolve_collision(node->front.get(), entity, collision_system, force);
		resolve_collision(node->back.get(), entity, collision_system, force);
	}

	Side side;
	Transform &t = ecs_manager.get_component<Transform>(entity);
	if (ecs_manager.has_component<ColliderOBB>(entity)) {
		ColliderOBB &obb = ecs_manager.get_component<ColliderOBB>(entity);
		ColliderOBB c;
		c.set_orientation(t.get_euler_rot());
		glm::mat3 o = c.get_orientation_matrix();
		c.range = obb.range * t.get_scale();
		c.center = t.get_position() + o * (obb.center * c.range);
		side = process_collider(node->plane, c);
	} else if (ecs_manager.has_component<ColliderAABB>(entity)) {
		ColliderAABB &aabb = ecs_manager.get_component<ColliderAABB>(entity);
		ColliderAABB c;
		c.range = aabb.range * t.get_scale();
		c.center = t.get_position() + aabb.center * c.range;
		side = process_collider(node->plane, c);
	} else if (ecs_manager.has_component<ColliderSphere>(entity)) {
		ColliderSphere &sphere = ecs_manager.get_component<ColliderSphere>(entity);
		ColliderSphere c;
		c.radius = sphere.radius * t.get_scale().x;
		c.center = t.get_position() + sphere.center * c.radius;
		side = process_collider(node->plane, c);
	} else {
		SPDLOG_WARN("Movable object has invalid collider");
		return;
	}

	if (side == Side::FRONT) {
		resolve_collision(node->front.get(), entity, collision_system);
	} else if (side == Side::BACK) {
		resolve_collision(node->back.get(), entity, collision_system, force);
	} else {
		resolve_collision(node->front.get(), entity, collision_system, true);
		resolve_collision(node->back.get(), entity, collision_system, true);
	}
}

void BSPSystem::build_tree(int32_t depth) {
	root = std::make_shared<BSPNode>();
	process_node(entities, root.get(), depth);
}

void BSPSystem::process_node(const std::set<Entity> &objects, BSPNode *node, int32_t depth) {
	std::set<Entity> current, front, back;

	if (depth == 0 || objects.empty()) {
		node = nullptr;
		return;
	}

	node->plane = calculate_plane(objects);

	for (const Entity entity : objects) {
		Transform &t = ecs_manager.get_component<Transform>(entity);
		if (ecs_manager.has_component<ColliderAABB>(entity)) {
			ColliderAABB &aabb = ecs_manager.get_component<ColliderAABB>(entity);
			ColliderAABB c;
			c.range = aabb.range * t.get_scale();
			c.center = t.get_position() + aabb.center * c.range;
			switch (process_collider(node->plane, c)) {
				case Side::FRONT:
					front.insert(entity);
					break;
				case Side::BACK:
					back.insert(entity);
					break;
				case Side::INTERSECT:
					current.insert(entity);
					break;
			}
		} else if (ecs_manager.has_component<ColliderOBB>(entity)) {
			ColliderOBB &obb = ecs_manager.get_component<ColliderOBB>(entity);
			ColliderOBB c;
			c.set_orientation(t.get_euler_rot());
			glm::mat3 o = c.get_orientation_matrix();
			c.range = obb.range * t.get_scale();
			c.center = t.get_position() + o * (obb.center * c.range);
			switch (process_collider(node->plane, c)) {
				case Side::FRONT:
					front.insert(entity);
					break;
				case Side::BACK:
					back.insert(entity);
					break;
				case Side::INTERSECT:
					current.insert(entity);
					break;
			}

		} else if (ecs_manager.has_component<ColliderSphere>(entity)) {
			ColliderSphere &sphere = ecs_manager.get_component<ColliderSphere>(entity);
			ColliderSphere c;
			c.radius = sphere.radius * t.get_scale().x;
			c.center = t.get_position() + sphere.center * c.radius;
			switch (process_collider(node->plane, c)) {
				case Side::FRONT:
					front.insert(entity);
					break;
				case Side::BACK:
					back.insert(entity);
					break;
				case Side::INTERSECT:
					current.insert(entity);
					break;
			}
		}
	}

	node->entities = current;

	node->front = std::make_shared<BSPNode>();
	process_node(front, node->front.get(), depth - 1);

	node->back = std::make_shared<BSPNode>();
	process_node(back, node->back.get(), depth - 1);
}

Plane BSPSystem::calculate_plane(const std::set<Entity> &colliders) {
	Plane plane{ glm::vec3(0.0f), glm::vec3(0.0f) };

	if (colliders.empty()) {
		SPDLOG_WARN("Plane has no colliders to process");
		return plane;
	}

	for (const Entity collider : colliders) {
		Transform &t = ecs_manager.get_component<Transform>(collider);

		if (ecs_manager.has_component<ColliderAABB>(collider)) {
			ColliderAABB &aabb = ecs_manager.get_component<ColliderAABB>(collider);
			ColliderAABB c;
			c.range = aabb.range * t.get_scale();
			c.center = t.get_position() + aabb.center * c.range;

			plane.point += c.center;
		} else if (ecs_manager.has_component<ColliderSphere>(collider)) {
			ColliderSphere &sphere = ecs_manager.get_component<ColliderSphere>(collider);
			ColliderSphere c;
			c.radius = sphere.radius * t.get_scale().x;
			c.center = t.get_position() + sphere.center * c.radius;

			plane.point += c.center;
		} else if (ecs_manager.has_component<ColliderOBB>(collider)) {
			ColliderOBB &obb = ecs_manager.get_component<ColliderOBB>(collider);
			ColliderOBB c;
			c.set_orientation(t.get_euler_rot());
			c.range = obb.range * t.get_scale();
			c.center = t.get_position() + c.get_orientation_matrix() * (obb.center * c.range);

			plane.point += c.center;
		}
	}
	plane.point /= colliders.size();

	for (const Entity collider : colliders) {
		Transform &t = ecs_manager.get_component<Transform>(collider);

		if (ecs_manager.has_component<ColliderAABB>(collider)) {
			ColliderAABB &aabb = ecs_manager.get_component<ColliderAABB>(collider);
			ColliderAABB c;
			c.range = aabb.range * t.get_scale();
			c.center = t.get_position() + aabb.center * c.range;

			const glm::vec3 direction = plane.point - c.center;
			if (direction != glm::vec3(0.0f)) {
				const glm::vec3 direction_abs = glm::abs(direction);
				glm::vec3 normal(0.0f);

				if (direction_abs.x > direction_abs.y) {
					if (direction_abs.x > direction_abs.z) {
						normal.x = direction.x < 0.0f ? -1.0f : 1.0f;
					} else {
						normal.z = direction.z < 0.0f ? -1.0f : 1.0f;
					}
				} else {
					if (direction_abs.y > direction_abs.z) {
						normal.y = direction.y < 0.0f ? -1.0f : 1.0f;
					} else {
						normal.z = direction.z < 0.0f ? -1.0f : 1.0f;
					}
				}

				plane.normal += normal;
			}

		} else if (ecs_manager.has_component<ColliderSphere>(collider)) {
			ColliderSphere &sphere = ecs_manager.get_component<ColliderSphere>(collider);
			ColliderSphere c;
			c.radius = sphere.radius * t.get_scale().x;
			c.center = t.get_position() + sphere.center * c.radius;

			glm::vec3 direction = plane.point - c.center;
			if (direction != glm::vec3(0.0f)) {
				plane.normal += glm::normalize(direction);
			}

		} else if (ecs_manager.has_component<ColliderOBB>(collider)) {
			ColliderOBB &obb = ecs_manager.get_component<ColliderOBB>(collider);
			ColliderOBB c;
			c.set_orientation(t.get_euler_rot());
			glm::mat3 o = c.get_orientation_matrix();
			c.range = obb.range * t.get_scale();
			c.center = t.get_position() + o * (obb.center * c.range);

			glm::vec3 direction = glm::transpose(o) * (plane.point - c.center);
			glm::vec3 normal(0.0f);

			for (int i = 0; i < 3; ++i) {
				float dot = glm::dot(direction, o[i]);

				if (dot > 0.0f) {
					normal += o[i];
				} else if (dot < 0.0f) {
					normal -= o[i];
				}
			}

			if (normal != glm::vec3(0.0f)) {
				plane.normal += glm::normalize(normal);
			}
		}
	}

	plane.normal /= colliders.size();

	plane.normal = glm::normalize(plane.normal);

	return plane;
}

Side BSPSystem::process_collider(const Plane &plane, const ColliderOBB &collider) {
	int32_t front = 0, back = 0;

	const glm::vec3 min = collider.center - collider.range;
	const glm::vec3 max = collider.center + collider.range;

	glm::vec3 vertices[] = {
		min,
		{ min.x, max.y, min.z },
		{ min.x, max.y, max.z },
		{ min.x, min.y, max.z },

		max,
		{ max.x, min.y, max.z },
		{ max.x, min.y, min.z },
		{ max.x, max.y, min.z },
	};

	const glm::mat3 &rotation = glm::transpose(collider.get_orientation_matrix());

	for (const glm::vec3 &v : vertices) {
		glm::vec3 dir = plane.point - v;

		float dot = glm::dot(dir, plane.normal);
		if (dot < 0.0f) {
			front++;
		} else if (dot > 0.0f) {
			back++;
		}
	}

	if (back == 0) {
		return Side::FRONT;
	} else if (front == 0) {
		return Side::BACK;
	} else {
		return Side::INTERSECT;
	}
}

Side BSPSystem::process_collider(const Plane &plane, const ColliderAABB &collider) {
	int32_t front = 0, back = 0;

	glm::vec3 min = collider.min();
	glm::vec3 max = collider.max();

	glm::vec3 vertices[] = {
		min,
		{ min.x, max.y, min.z },
		{ min.x, max.y, max.z },
		{ min.x, min.y, max.z },

		max,
		{ max.x, min.y, max.z },
		{ max.x, min.y, min.z },
		{ max.x, max.y, min.z },
	};

	for (const glm::vec3 &v : vertices) {
		const glm::vec3 &dir = plane.point - v;
		float dot = glm::dot(dir, plane.normal);
		if (dot < 0.0f) {
			front++;
		} else if (dot > 0.0f) {
			back++;
		}
	}

	if (back == 0) {
		return Side::FRONT;
	} else if (front == 0) {
		return Side::BACK;
	} else {
		return Side::INTERSECT;
	}
}

Side BSPSystem::process_collider(const Plane &plane, const ColliderSphere &collider) {
	glm::vec3 dir = plane.point - collider.center;

	float dot = glm::dot(dir, plane.normal);
	if (glm::dot(dir, dir) > collider.radius) {
		return Side::INTERSECT;
	} else if (dot < 0.0f) {
		return Side::FRONT;
	} else {
		return Side::BACK;
	}
}