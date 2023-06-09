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
	if (first) {
		first = false;
		world.get_parent_scene()->bsp_tree = CollisionSystem::build_tree(world, world.get_parent_scene()->entities, 10);
	}

	resolve_collision_dynamic(world);

	BSPNode *root = world.get_parent_scene()->bsp_tree.get();
	for (const Entity entity : entities) {
		if (world.get_component<ColliderTag>(entity).is_active) {
			resolve_bsp_collision(world, root, entity);
		}
	}
}

void CollisionSystem::resolve_collision_dynamic(World &world) {
	ZoneScopedN("CollisionSystem::update::resolve_collision_dynamic");
	auto physics_manager = PhysicsManager::get();

	CollisionFlag first, second;
	for (auto it1 = entities.begin(); it1 != entities.end(); ++it1) {
		Entity e1 = std::ref(*it1);
		if (!world.get_component<ColliderTag>(e1).is_active) {
			continue;
		}
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
			continue;
		}
		for (auto it2 = std::next(it1); it2 != entities.end(); ++it2) {
			Entity e2 = std::ref(*it2);
			if (!world.get_component<ColliderTag>(e2).is_active) {
				continue;
			}

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
				case CollisionFlag::CAPSULE_SPHERE:
					physics_manager.resolve_capsule_sphere(world, e1, e2);
					break;
				case CollisionFlag::SPHERE_CAPSULE:
					physics_manager.resolve_capsule_sphere(world, e2, e1);
					break;
				case CollisionFlag::CAPSULE_AABB:
					physics_manager.resolve_capsule_aabb(world, e1, e2);
					break;
				case CollisionFlag::AABB_CAPSULE:
					physics_manager.resolve_capsule_aabb(world, e2, e1);
					break;
				case CollisionFlag::CAPSULE_OBB:
					physics_manager.resolve_capsule_obb(world, e1, e2);
					break;
				case CollisionFlag::OBB_CAPSULE:
					physics_manager.resolve_capsule_obb(world, e2, e1);
					break;
				default:
					SPDLOG_WARN("Entities has not supported collision type {} {}", e1, e2);
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

	if (force) {
		resolve_bsp_collision(world, node->front.get(), entity, force);
		resolve_bsp_collision(world, node->back.get(), entity, force);
		return;
	}

	if (node->back == nullptr && node->front == nullptr) {
		return;
	}

	Side side;
	Transform &t = world.get_component<Transform>(entity);
	const glm::vec3 &scale = t.get_global_scale();
	const glm::vec3 &position = t.get_global_position();

	if (world.has_component<ColliderOBB>(entity)) {
		ColliderOBB c = world.get_component<ColliderOBB>(entity);
		c.set_orientation(t.get_global_orientation());
		const glm::mat3 &o = c.get_orientation_matrix();
		c.range = c.range * scale;
		c.center = position + o * (c.center * scale);
		side = process_collider(node->plane, c);
	} else if (world.has_component<ColliderAABB>(entity)) {
		ColliderAABB c = world.get_component<ColliderAABB>(entity);
		c.range = c.range * scale;
		c.center = position + c.center * scale;
		side = process_collider(node->plane, c);
	} else if (world.has_component<ColliderSphere>(entity)) {
		ColliderSphere c = world.get_component<ColliderSphere>(entity);
		c.radius = c.radius * scale.x;
		c.center = position + c.center * scale;
		side = process_collider(node->plane, c);
	} else if (world.has_component<ColliderCapsule>(entity)) {
		ColliderCapsule c = world.get_component<ColliderCapsule>(entity);
		c.radius = c.radius * scale.x;
		c.start = position + c.start * scale;
		c.start = position + c.end * scale;
		side = process_collider(node->plane, c);
	} else {
		SPDLOG_WARN("Movable object has not supported collider");
		return;
	}

	if (side == Side::FRONT) {
		resolve_bsp_collision(world, node->front.get(), entity, force);
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
			if (world.has_component<Platform>(entity)) {
				auto platform = world.get_component<Platform>(entity);
				if (!platform.is_door) {
					continue;
				}
			}
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
		const glm::vec3 &position = t.get_global_position();
		const glm::vec3 &scale = t.get_global_scale();
		if (world.has_component<ColliderAABB>(entity)) {
			ColliderAABB &aabb = world.get_component<ColliderAABB>(entity);
			ColliderAABB c;
			c.range = aabb.range * scale;
			c.center = position + aabb.center * scale;
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
			const glm::quat &orientation = t.get_global_orientation();
			c.set_orientation(orientation);
			c.range = obb.range * scale;
			c.center = position + orientation * (obb.center * scale);
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
			c.radius = sphere.radius * scale.x;
			c.center = position + sphere.center * scale;
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
		} else if (world.has_component<ColliderCapsule>(entity)) {
			ColliderCapsule &sphere = world.get_component<ColliderCapsule>(entity);
			ColliderCapsule c;
			c.radius = sphere.radius * scale.x;
			c.start = position + sphere.start * scale;
			c.end = position + sphere.end * scale;
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
		} else {
			SPDLOG_WARN("Entity has not supported collision type {}", entity);
		}
	}

	if (current.size() == objects.size() || front.size() == objects.size() || back.size() == objects.size()) {
		node->entities = objects;
		return;
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
		const glm::vec3 &position = t.get_global_position();
		const glm::vec3 &scale = t.get_global_scale();

		if (world.has_component<ColliderAABB>(collider)) {
			ColliderAABB &aabb = world.get_component<ColliderAABB>(collider);
			plane.point += position + aabb.center * scale;
		} else if (world.has_component<ColliderSphere>(collider)) {
			ColliderSphere &sphere = world.get_component<ColliderSphere>(collider);
			plane.point += position + sphere.center * scale;
		} else if (world.has_component<ColliderOBB>(collider)) {
			ColliderOBB &obb = world.get_component<ColliderOBB>(collider);
			const glm::quat &orientation = t.get_global_orientation();
			plane.point += position + orientation * (obb.center * scale);
		} else if (world.has_component<ColliderCapsule>(collider)) {
			ColliderCapsule &capsule = world.get_component<ColliderCapsule>(collider);
			const glm::vec3 &start = position + capsule.start * scale;
			const glm::vec3 &end = position + capsule.end * scale;
			plane.point += start + (end - start) * 0.5f;
		} else {
			SPDLOG_WARN("Entity has not supported collision type {}", collider);
		}
	}
	plane.point /= colliders.size();

	for (const Entity collider : colliders) {
		Transform &t = world.get_component<Transform>(collider);
		const glm::vec3 &position = t.get_global_position();
		const glm::vec3 &scale = t.get_global_scale();

		if (world.has_component<ColliderAABB>(collider)) {
			ColliderAABB &aabb = world.get_component<ColliderAABB>(collider);
			ColliderAABB c;
			c.range = aabb.range * scale;
			c.center = position + aabb.center * scale;

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
			c.radius = sphere.radius * scale.x;
			c.center = position + sphere.center * scale;

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
			const glm::quat &orientation = t.get_global_orientation();
			c.set_orientation(orientation);
			glm::mat3 o = c.get_orientation_matrix();
			c.range = obb.range * scale;
			c.center = position + orientation * (obb.center * scale);

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
		} else if (world.has_component<ColliderCapsule>(collider)) {
			ColliderCapsule &capsule = world.get_component<ColliderCapsule>(collider);
			ColliderSphere c;
			c.radius = capsule.radius * scale.x;
			const glm::vec3 &start = position + capsule.start * scale;
			const glm::vec3 &end = position + capsule.end * scale;
			c.center = start + (end - start) * 0.5f;

			glm::vec3 direction = plane.point - c.center;
			if (glm::dot(direction, direction) > 0.0f) {
				glm::vec3 test = plane.normal + glm::normalize(direction);
				if (glm::dot(test, test) > 0.0f) {
					plane.normal = test;
				}
			}
		} else {
			SPDLOG_WARN("Entity has not supported collision type {}", collider);
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

Side CollisionSystem::process_collider(const Plane &plane, const ColliderCapsule &collider) {
	const glm::vec3 &dir_start = plane.point - collider.start;
	const glm::vec3 &dir_end = plane.point - collider.end;

	float dot_start = glm::dot(dir_start, plane.normal);
	float dot_end = glm::dot(dir_end, plane.normal);
	float radius2 = collider.radius * collider.radius;
	if (glm::length2(dir_start) > radius2 || glm::length2(dir_end) > radius2 || dot_start * dot_end < 0.0f) {
		return Side::INTERSECT;
	} else if (dot_start < 0.0f) {
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

		if (std::find(ray.ignore_list.begin(), ray.ignore_list.end(), entity) != ray.ignore_list.end()) {
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
			if (physics_manager.intersect_ray_aabb(ray, c, info)) {
				does_hit = true;
			}
		} else if (world.has_component<ColliderOBB>(entity)) {
			ColliderOBB c = world.get_component<ColliderOBB>(entity);
			c.center = position + c.get_orientation_matrix() * (c.center * scale);
			c.range *= scale;
			if (physics_manager.intersect_ray_obb(ray, c, info)) {
				does_hit = true;
			}
		} else if (world.has_component<ColliderSphere>(entity)) {
			ColliderSphere c = world.get_component<ColliderSphere>(entity);
			c.center = position + c.center * scale;
			c.radius *= scale.x;
			if (physics_manager.intersect_ray_sphere(ray, c, info)) {
				does_hit = true;
			}
		} else if (world.has_component<ColliderCapsule>(entity)) {
			ColliderCapsule c = world.get_component<ColliderCapsule>(entity);
			c.start = position + c.start * scale;
			c.end = position + c.end * scale;
			c.radius *= scale.x;
			if (physics_manager.intersect_ray_capsule(ray, c, info)) {
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
	bool result_set = false;

	for (auto entity : entities) {
		if (!world.has_component<ColliderTag>(entity) || !world.has_component<Transform>(entity)) {
			continue;
		}

		if (std::find(ray.ignore_list.begin(), ray.ignore_list.end(), entity) != ray.ignore_list.end()) {
			continue;
		}

		const ColliderTag &tag = world.get_component<ColliderTag>(entity);
		if (!tag.is_active || !physics_manager.are_layers_collide(ray.layer_name, tag.layer_name)) {
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
			if (physics_manager.intersect_ray_aabb(ray, c, info)) {
				does_hit = true;
			}
		} else if (world.has_component<ColliderOBB>(entity)) {
			ColliderOBB c = world.get_component<ColliderOBB>(entity);
			const glm::quat &orientation = transform.get_global_orientation();
			c.set_orientation(orientation);
			c.center = position + orientation * (c.center * scale);
			c.range *= scale;
			if (physics_manager.intersect_ray_obb(ray, c, info)) {
				does_hit = true;
			}
		} else if (world.has_component<ColliderSphere>(entity)) {
			ColliderSphere c = world.get_component<ColliderSphere>(entity);
			c.center = position + c.center * scale;
			c.radius *= scale.x;
			if (physics_manager.intersect_ray_sphere(ray, c, info)) {
				does_hit = true;
			}
		} else if (world.has_component<ColliderCapsule>(entity)) {
			ColliderCapsule c = world.get_component<ColliderCapsule>(entity);
			c.start = position + c.start * scale;
			c.end = position + c.end * scale;
			c.radius *= scale.x;
			if (physics_manager.intersect_ray_capsule(ray, c, info)) {
				does_hit = true;
			}
		}

		if (info.distance < result.distance) {
			result = info;
			result_set = true;
		}
	}
	return does_hit && result_set;
}
