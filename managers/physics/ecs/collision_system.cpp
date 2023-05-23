#include "collision_system.h"
#include <physics/physics_manager.h>
#include <spdlog/spdlog.h>

#include "components/collider_aabb.h"
#include "components/collider_sphere.h"
#include "components/collider_tag_component.h"
#include "components/static_tag_component.h"
#include "ecs/world.h"
#include "engine/scene.h"

void CollisionSystem::startup(World &world) {
	Signature white_signature;
	white_signature.set(world.get_component_type<ColliderTag>());
	world.set_system_component_whitelist<CollisionSystem>(white_signature);

	Signature black_signature;
	black_signature.set(world.get_component_type<StaticTag>());
	world.set_system_component_blacklist<CollisionSystem>(black_signature);

	PhysicsManager &physics_manager = PhysicsManager::get();

	physics_manager.add_collision_layer("default");
}

void CollisionSystem::update(World &world, float dt) {
	ZoneScopedN("CollisionSystem::update");

	// HACK: We're building the BSP tree here at the last moment, because colliders get their positions / orientations /
	// scales from the global matrices, which are only updated after the first iteration of the transform ECS systems
	static bool first = true;
	if (first) {
		first = false;
		world.get_parent_scene()->bsp_tree = CollisionSystem::build_tree(world, world.get_parent_scene()->entities, 10);
	}

	resolve_collision_dynamic(world);

	BSPNode *root = world.get_parent_scene()->bsp_tree.get();
	for (const Entity entity : entities) {
		resolve_bsp_collision(world, root, entity);
	}
}

void CollisionSystem::resolve_collision_dynamic(World &world) {
	ZoneScopedN("CollisionSystem::update::resolve_collision_dynamic");
	auto physics_manager = PhysicsManager::get();

	CollisionFlag first, second;
	for (auto it1 = entities.begin(); it1 != entities.end(); ++it1) {
		Entity e1 = std::ref(*it1);
		if (world.has_component<ColliderOBB>(e1)) {
			first = CollisionFlag::FIRST_OBB;
		} else if (world.has_component<ColliderAABB>(e1)) {
			first = CollisionFlag::FIRST_AABB;
		} else if (world.has_component<ColliderSphere>(e1)) {
			first = CollisionFlag::FIRST_SPHERE;
		} else if (world.has_component<ColliderCapsule>(e1)) {
			first = CollisionFlag::FIRST_CAPSULE;
		} else {
			continue;
		}
		for (auto it2 = std::next(it1); it2 != entities.end(); ++it2) {
			Entity e2 = std::ref(*it2);

			if (world.has_component<ColliderOBB>(e2)) {
				second = CollisionFlag::SECOND_OBB;
			} else if (world.has_component<ColliderAABB>(e2)) {
				second = CollisionFlag::SECOND_AABB;
			} else if (world.has_component<ColliderSphere>(e2)) {
				second = CollisionFlag::SECOND_SPHERE;
			} else if (world.has_component<ColliderCapsule>(e2)) {
				second = CollisionFlag::SECOND_CAPSULE;
			} else {
				continue;
			}

			switch (first | second) {
				case CollisionFlag::SPHERE_SPHERE:
					physics_manager.resolve_collision_sphere(world, e1, e2);

					break;
				case CollisionFlag::AABB_AABB:
					physics_manager.resolve_collision_aabb(world, e1, e2);
					break;
				case CollisionFlag::SPHERE_AABB:
					physics_manager.resolve_aabb_sphere(world, e2, e1);
					break;
				case CollisionFlag::AABB_SPHERE:
					physics_manager.resolve_aabb_sphere(world, e1, e2);
					break;
				case CollisionFlag::OBB_OBB:
					physics_manager.resolve_collision_obb(world, e1, e2);
					break;
				case CollisionFlag::SPHERE_OBB:
					physics_manager.resolve_obb_sphere(world, e2, e1);
					break;
				case CollisionFlag::OBB_SPHERE:
					physics_manager.resolve_obb_sphere(world, e1, e2);
					break;
				case CollisionFlag::AABB_OBB:
					physics_manager.resolve_obb_aabb(world, e2, e1);
					break;
				case CollisionFlag::OBB_AABB:
					physics_manager.resolve_obb_aabb(world, e1, e2);
					break;
				case CollisionFlag::CAPSULE_CAPSULE:
					physics_manager.resolve_collision_capsule(world, e1, e2);
					break;
				case CollisionFlag::CAPSULE_AABB:
				case CollisionFlag::CAPSULE_OBB:
				case CollisionFlag::CAPSULE_SPHERE:
					//TODO: implement
					break;
				default:
					break;
			}
		}
	}
}

void CollisionSystem::resolve_bsp_collision(World &world, BSPNode *node, Entity entity, bool force) {
	ZoneScopedN("CollisionSystem::update::resolve_bsp_collision");
	if ((node == nullptr) || (node->entities.empty() && node->back == nullptr && node->front == nullptr)) {
		return;
	}
	//Every node contains entities that intersects with it or if node is a leaf
	PhysicsManager::get().resolve_collision(world, entity, node->entities);

	if (node->back == nullptr && node->front == nullptr) {
		return;
	}

	if (force) {
		resolve_bsp_collision(world, node->front.get(), entity, force);
		resolve_bsp_collision(world, node->back.get(), entity, force);
	}

	Side side;
	Transform &t = world.get_component<Transform>(entity);
	if (world.has_component<ColliderOBB>(entity)) {
		ColliderOBB &obb = world.get_component<ColliderOBB>(entity);
		ColliderOBB c;
		c.set_orientation(t.get_global_orientation());
		glm::mat3 o = c.get_orientation_matrix();
		c.range = obb.range * t.get_global_scale();
		c.center = t.get_global_position() + o * (obb.center * c.range);
		side = process_collider(node->plane, c);
	} else if (world.has_component<ColliderAABB>(entity)) {
		ColliderAABB &aabb = world.get_component<ColliderAABB>(entity);
		ColliderAABB c;
		c.range = aabb.range * t.get_global_scale();
		c.center = t.get_global_position() + aabb.center * c.range;
		side = process_collider(node->plane, c);
	} else if (world.has_component<ColliderSphere>(entity)) {
		ColliderSphere &sphere = world.get_component<ColliderSphere>(entity);
		ColliderSphere c;
		c.radius = sphere.radius * t.get_global_scale().x;
		c.center = t.get_global_position() + sphere.center * c.radius;
		side = process_collider(node->plane, c);
	} else {
		SPDLOG_WARN("Movable object has invalid collider");
		return;
	}

	if (side == Side::FRONT) {
		resolve_bsp_collision(world, node->front.get(), entity);
	} else if (side == Side::BACK) {
		resolve_bsp_collision(world, node->back.get(), entity, force);
	} else {
		resolve_bsp_collision(world, node->front.get(), entity, true);
		resolve_bsp_collision(world, node->back.get(), entity, true);
	}
}

std::shared_ptr<BSPNode> CollisionSystem::build_tree(World &world, std::vector<Entity> &world_entities, int32_t depth) {
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

void CollisionSystem::process_node(World &world, const std::set<Entity> &objects, BSPNode *node, int32_t depth) {
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
			c.range = aabb.range * t.get_global_scale();
			c.center = t.get_global_position() + aabb.center * c.range;
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
		} else if (world.has_component<ColliderOBB>(entity)) {
			ColliderOBB &obb = world.get_component<ColliderOBB>(entity);
			ColliderOBB c;
			c.set_orientation(t.get_global_orientation());
			glm::mat3 o = c.get_orientation_matrix();
			c.range = obb.range * t.get_global_scale();
			c.center = t.get_global_position() + o * (obb.center * c.range);
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

		} else if (world.has_component<ColliderSphere>(entity)) {
			ColliderSphere &sphere = world.get_component<ColliderSphere>(entity);
			ColliderSphere c;
			c.radius = sphere.radius * t.get_global_scale().x;
			c.center = t.get_global_position() + sphere.center * c.radius;
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
	process_node(world, front, node->front.get(), depth - 1);

	node->back = std::make_shared<BSPNode>();
	process_node(world, back, node->back.get(), depth - 1);
}

Plane CollisionSystem::calculate_plane(World &world, const std::set<Entity> &colliders) {
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
			c.range = aabb.range * t.get_global_scale();
			c.center = t.get_global_position() + aabb.center * c.range;
			plane.point += c.center;
		} else if (world.has_component<ColliderSphere>(collider)) {
			ColliderSphere &sphere = world.get_component<ColliderSphere>(collider);
			ColliderSphere c;
			c.radius = sphere.radius * t.get_global_scale().x;
			c.center = t.get_global_position() + sphere.center * c.radius;

			plane.point += c.center;
		} else if (world.has_component<ColliderOBB>(collider)) {
			ColliderOBB &obb = world.get_component<ColliderOBB>(collider);
			ColliderOBB c;
			c.set_orientation(t.get_global_orientation());
			c.range = obb.range * t.get_global_scale();
			c.center = t.get_global_position() + c.get_orientation_matrix() * (obb.center * c.range);

			plane.point += c.center;
		}
	}
	plane.point /= colliders.size();

	for (const Entity collider : colliders) {
		Transform &t = world.get_component<Transform>(collider);

		if (world.has_component<ColliderAABB>(collider)) {
			ColliderAABB &aabb = world.get_component<ColliderAABB>(collider);
			ColliderAABB c;
			c.range = aabb.range * t.get_global_scale();
			c.center = t.get_global_position() + aabb.center * c.range;

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
			c.radius = sphere.radius * t.get_global_scale().x;
			c.center = t.get_global_position() + sphere.center * c.radius;

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
			c.set_orientation(t.get_global_orientation());
			glm::mat3 o = c.get_orientation_matrix();
			c.range = obb.range * t.get_global_scale();
			c.center = t.get_global_position() + o * (obb.center * c.range);

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

Side CollisionSystem::process_collider(const Plane &plane, const ColliderAABB &collider) {
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

Side CollisionSystem::process_collider(const Plane &plane, const ColliderOBB &collider) {
	int32_t front = 0, back = 0;

	const glm::mat3 &o = collider.get_orientation_matrix();
	const glm::vec3 min = o * collider.min();
	const glm::vec3 max = o * collider.max();

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

Side CollisionSystem::process_collider(const Plane &plane, const ColliderSphere &collider) {
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

void CollisionSystem::log_tree(BSPNode *node) {
	if (node == nullptr) {
		return;
	}

	log_tree(node->front.get());
	log_tree(node->back.get());
}

bool CollisionSystem::ray_cast(World &world, const Ray &ray, HitInfo &result) {
	auto physics_manager = PhysicsManager::get();
	std::vector<Entity> entities = world.get_parent_scene()->entities;

	bool does_hit = false;
	for (auto entity : entities) {
		if (!world.has_component<ColliderTag>(entity) || !world.has_component<Transform>(entity)) {
			continue;
		}
		Transform &transform = world.get_component<Transform>(entity);
		HitInfo info;
		info.entity = entity;
		if (world.has_component<ColliderAABB>(entity)) {
			ColliderAABB c = world.get_component<ColliderAABB>(entity);
			c.center += transform.get_global_position();
			c.range *= transform.get_global_scale();
			if (physics_manager.intersect_ray_aabb(ray, c, info)) {
				does_hit = true;
			}
		} else if (world.has_component<ColliderOBB>(entity)) {
			ColliderOBB c = world.get_component<ColliderOBB>(entity);
			c.center = transform.get_global_position() +
					c.get_orientation_matrix() * (c.center * transform.get_global_scale());
			c.range *= transform.get_global_scale();
			if (physics_manager.intersect_ray_obb(ray, c, info)) {
				does_hit = true;
			}
		} else if (world.has_component<ColliderSphere>(entity)) {
			ColliderSphere c = world.get_component<ColliderSphere>(entity);
			c.center += transform.get_global_position();
			c.radius *= transform.get_global_scale().x;
			if (physics_manager.intersect_ray_sphere(ray, c, info)) {
				does_hit = true;
			}
		}

		if (info.distance < result.distance) {
			result = info;
		}
	}
	return does_hit;
}

bool CollisionSystem::ray_cast_layer(World &world, const Ray &ray, HitInfo &result) {
	auto physics_manager = PhysicsManager::get();
	std::vector<Entity> entities = world.get_parent_scene()->entities;

	bool does_hit = false;
	for (auto entity : entities) {
		if (!world.has_component<ColliderTag>(entity) || !world.has_component<Transform>(entity)) {
			continue;
		}

		if (std::find(ray.ignore_list.begin(), ray.ignore_list.end(), entity) != ray.ignore_list.end()) {
			continue;
		}

		auto &tag = world.get_component<ColliderTag>(entity);
		if (!physics_manager.are_layers_collide(ray.layer_name, tag.layer_name)) {
			continue;
		}

		auto &transform = world.get_component<Transform>(entity);
		HitInfo info;
		info.entity = entity;
		if (world.has_component<ColliderAABB>(entity)) {
			ColliderAABB c = world.get_component<ColliderAABB>(entity);
			c.center += transform.get_global_position();
			c.range *= transform.get_global_scale();
			if (physics_manager.intersect_ray_aabb(ray, c, info)) {
				does_hit = true;
			}
		} else if (world.has_component<ColliderOBB>(entity)) {
			ColliderOBB c = world.get_component<ColliderOBB>(entity);
			c.center = transform.get_global_position() +
					c.get_orientation_matrix() * (c.center * transform.get_global_scale());
			c.range *= transform.get_global_scale();
			if (physics_manager.intersect_ray_obb(ray, c, info)) {
				does_hit = true;
			}
		} else if (world.has_component<ColliderSphere>(entity)) {
			ColliderSphere c = world.get_component<ColliderSphere>(entity);
			c.center += transform.get_global_position();
			c.radius *= transform.get_global_scale().x;
			if (physics_manager.intersect_ray_sphere(ray, c, info)) {
				does_hit = true;
			}
		}

		if (info.distance < result.distance) {
			result = info;
		}
	}
	return does_hit;
}
