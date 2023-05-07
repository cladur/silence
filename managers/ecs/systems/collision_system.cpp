#include "collision_system.h"
#include <physics/physics_manager.h>

#include "components/collider_aabb.h"
#include "components/collider_sphere.h"
#include "components/collider_tag_component.h"
#include "components/static_tag_component.h"
#include "ecs/world.h"

void CollisionSystem::startup(World &world) {
	Signature white_signature, black_signature;
	white_signature.set(world.get_component_type<ColliderTag>());
	world.set_system_component_whitelist<CollisionSystem>(white_signature);

	black_signature.set(world.get_component_type<StaticTag>());
	world.set_system_component_blacklist<CollisionSystem>(black_signature);
}

void CollisionSystem::update(World &world, float dt) {
	ZoneScopedN("CollisionSystem::update");

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
				default:
					break;
			}
		}
	}
}