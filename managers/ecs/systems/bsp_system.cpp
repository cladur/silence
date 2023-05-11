#include "bsp_system.h"
#include <physics/physics_manager.h>

//#include "ecs/world.h"
#include "engine/scene.h"

struct StaticTag;
struct ColliderTag;

void BSPSystem::startup(World &world) {
	Signature white_signature;
	white_signature.set(world.get_component_type<ColliderTag>());
	//white_signature.set(world.get_component_type<StaticTag>());
	world.set_system_component_whitelist<BSPSystem>(white_signature);

	Signature black_signature;
	black_signature.set(world.get_component_type<StaticTag>());
	world.set_system_component_blacklist<BSPSystem>(black_signature);
}

void BSPSystem::update(World &world, float dt) {
	for (const Entity entity : entities) {
		resolve_collision(world, world.get_parent_scene()->bsp_tree.get(), entity);
	}
}

void BSPSystem::resolve_collision(World &world, BSPNode *node, Entity entity, bool force) {
	if ((node == nullptr) || (node->entities.empty() && node->back == nullptr && node->front == nullptr)) {
		return;
	}
	//Every node contains entities that intersects with it or if node is leaf
	PhysicsManager::get().resolve_collision(world, entity, node->entities);

	if (force) {
		resolve_collision(world, node->front.get(), entity, force);
		resolve_collision(world, node->back.get(), entity, force);
	}

	Side side;
	Transform &t = world.get_component<Transform>(entity);
	if (world.has_component<ColliderOBB>(entity)) {
		ColliderOBB &obb = world.get_component<ColliderOBB>(entity);
		ColliderOBB c;
		c.set_orientation(t.get_euler_rot());
		glm::mat3 o = c.get_orientation_matrix();
		c.range = obb.range * t.get_scale();
		c.center = t.get_position() + o * (obb.center * c.range);
		side = process_collider(world, node->plane, c);
	} else if (world.has_component<ColliderAABB>(entity)) {
		ColliderAABB &aabb = world.get_component<ColliderAABB>(entity);
		ColliderAABB c;
		c.range = aabb.range * t.get_scale();
		c.center = t.get_position() + aabb.center * c.range;
		side = process_collider(world, node->plane, c);
	} else if (world.has_component<ColliderSphere>(entity)) {
		ColliderSphere &sphere = world.get_component<ColliderSphere>(entity);
		ColliderSphere c;
		c.radius = sphere.radius * t.get_scale().x;
		c.center = t.get_position() + sphere.center * c.radius;
		side = process_collider(world, node->plane, c);
	} else {
		SPDLOG_WARN("Movable object has invalid collider");
		return;
	}

	if (side == Side::FRONT) {
		resolve_collision(world, node->front.get(), entity);
	} else if (side == Side::BACK) {
		resolve_collision(world, node->back.get(), entity, force);
	} else {
		resolve_collision(world, node->front.get(), entity, true);
		resolve_collision(world, node->back.get(), entity, true);
	}
}

std::shared_ptr<BSPNode> BSPSystem::build_tree(World &world, std::vector<Entity> world_entities, int32_t depth) {
	std::shared_ptr<BSPNode> root = std::make_shared<BSPNode>();

	// this class' signature consists of all colliders and statics
	// we need to filter get only statics to build the tree
	std::set<Entity> statics;
	for (auto &entity : world_entities) {
		if (world.has_component<StaticTag>(entity)) {
			statics.insert(entity);
		}
	}

	process_node(world, statics, root.get(), depth);
	return root;
}

void BSPSystem::process_node(World &world, const std::set<Entity> &objects, BSPNode *node, int32_t depth) {
	std::set<Entity> current, front, back;
	if (depth == 0 || objects.size() <= 1) {
		node->entities = objects;
		node->front = nullptr;
		node->back = nullptr;
		return;
	}

	node->plane = calculate_plane(world, objects);
	for (const Entity entity : objects) {
		Transform &t = world.get_component<Transform>(entity);
		if (world.has_component<ColliderAABB>(entity)) {
			ColliderAABB &aabb = world.get_component<ColliderAABB>(entity);
			ColliderAABB c;
			c.range = aabb.range * t.get_scale();
			c.center = t.get_position() + aabb.center * c.range;
			switch (process_collider(world, node->plane, c)) {
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
		} else if (world.has_component<ColliderOBB>(entity)) {
			ColliderOBB &obb = world.get_component<ColliderOBB>(entity);
			ColliderOBB c;
			c.set_orientation(t.get_euler_rot());
			glm::mat3 o = c.get_orientation_matrix();
			c.range = obb.range * t.get_scale();
			c.center = t.get_position() + o * (obb.center * c.range);
			switch (process_collider(world, node->plane, c)) {
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

		} else if (world.has_component<ColliderSphere>(entity)) {
			ColliderSphere &sphere = world.get_component<ColliderSphere>(entity);
			ColliderSphere c;
			c.radius = sphere.radius * t.get_scale().x;
			c.center = t.get_position() + sphere.center * c.radius;
			switch (process_collider(world, node->plane, c)) {
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
	process_node(world, front, node->front.get(), depth - 1);

	node->back = std::make_shared<BSPNode>();
	process_node(world, back, node->back.get(), depth - 1);
}

Plane BSPSystem::calculate_plane(World &world, const std::set<Entity> &colliders) {
	Plane plane{ glm::vec3(0.0f), glm::vec3(0.0f) };

	if (colliders.size() <= 1) {
		SPDLOG_WARN("Plane has no colliders to process");
		return plane;
	}
	for (const Entity collider : colliders) {
		Transform &t = world.get_component<Transform>(collider);

		if (world.has_component<ColliderAABB>(collider)) {
			ColliderAABB &aabb = world.get_component<ColliderAABB>(collider);
			ColliderAABB c;
			c.range = aabb.range * t.get_scale();
			c.center = t.get_position() + aabb.center * c.range;
			plane.point += c.center;
		} else if (world.has_component<ColliderSphere>(collider)) {
			ColliderSphere &sphere = world.get_component<ColliderSphere>(collider);
			ColliderSphere c;
			c.radius = sphere.radius * t.get_scale().x;
			c.center = t.get_position() + sphere.center * c.radius;

			plane.point += c.center;
		} else if (world.has_component<ColliderOBB>(collider)) {
			ColliderOBB &obb = world.get_component<ColliderOBB>(collider);
			ColliderOBB c;
			c.set_orientation(t.get_euler_rot());
			c.range = obb.range * t.get_scale();
			c.center = t.get_position() + c.get_orientation_matrix() * (obb.center * c.range);

			plane.point += c.center;
		}
	}
	plane.point /= colliders.size();

	for (const Entity collider : colliders) {
		Transform &t = world.get_component<Transform>(collider);

		if (world.has_component<ColliderAABB>(collider)) {
			ColliderAABB &aabb = world.get_component<ColliderAABB>(collider);
			ColliderAABB c;
			c.range = aabb.range * t.get_scale();
			c.center = t.get_position() + aabb.center * c.range;

			const glm::vec3 direction = plane.point - c.center;
			if (glm::dot(direction, direction) > 0.0f) {
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
				glm::vec3 test = plane.normal + normal;
				if (glm::dot(test, test) > 0.0f) {
					plane.normal = test;
				}
			}

		} else if (world.has_component<ColliderSphere>(collider)) {
			ColliderSphere &sphere = world.get_component<ColliderSphere>(collider);
			ColliderSphere c;
			c.radius = sphere.radius * t.get_scale().x;
			c.center = t.get_position() + sphere.center * c.radius;

			glm::vec3 direction = plane.point - c.center;
			if (glm::dot(direction, direction) > 0.0f) {
				glm::vec3 test = plane.normal + glm::normalize(direction);
				if (glm::dot(test, test) > 0.0f) {
					plane.normal = test;
				}
			}

		} else if (world.has_component<ColliderOBB>(collider)) {
			ColliderOBB &obb = world.get_component<ColliderOBB>(collider);
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

			if (glm::dot(direction, direction) > 0.0f) {
				glm::vec3 test = plane.normal + glm::normalize(direction);
				if (glm::dot(test, test) > 0.0f) {
					plane.normal = test;
				}
			}
		}
	}
	plane.normal /= colliders.size();

	plane.normal = glm::normalize(plane.normal);

	return plane;
}

Side BSPSystem::process_collider(World &world, const Plane &plane, const ColliderOBB &collider) {
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

void BSPSystem::log_tree(BSPNode *node) {
	if (node == nullptr) {
		return;
	}

	log_tree(node->front.get());
	log_tree(node->back.get());
}

Side BSPSystem::process_collider(World &world, const Plane &plane, const ColliderAABB &collider) {
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

Side BSPSystem::process_collider(World &world, const Plane &plane, const ColliderSphere &collider) {
	glm::vec3 dir = plane.point - collider.center;

	float dot = glm::dot(dir, plane.normal);
	if (glm::dot(dir, dir) > collider.radius * collider.radius) {
		return Side::INTERSECT;
	} else if (dot < 0.0f) {
		return Side::FRONT;
	} else {
		return Side::BACK;
	}
}
