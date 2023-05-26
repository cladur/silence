#include "interactable_system.h"
#include "components/enemy_data_component.h"
#include "components/exploding_box_component.h"
#include "components/interactable_component.h"
#include "components/transform_component.h"
#include "cvars/cvars.h"
#include "ecs/systems/interactable_system.h"
#include "ecs/world.h"
#include "enemy_system.h"
#include "engine/scene.h"
#include "physics/ecs/physics_system.h"
#include "physics/physics_manager.h"
#include <spdlog/spdlog.h>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

void draw_explosion_radius(World &world, glm::vec3 centre, float radius, glm::vec3 color) {
	auto &dd = world.get_parent_scene()->get_render_scene().debug_draw;
	dd.draw_sphere(centre, radius, color);
}

void InteractableSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<Interactable>());
	world.set_system_component_whitelist<InteractableSystem>(signature);
}

void InteractableSystem::update(World &world, float dt) {
	for (auto const &entity : entities) {
		auto &interactable = world.get_component<Interactable>(entity);
		if (interactable.interaction == Exploding && *CVarSystem::get()->get_int_cvar("debug_draw.collision.draw")) {
			auto box = world.get_component<ExplodingBox>(interactable.interaction_target);
			auto center = world.get_component<Transform>(interactable.interaction_target).get_position();
			draw_explosion_radius(world, center, box.explosion_radius, { 255, 0, 0 });
			draw_explosion_radius(world, center, box.distraction_radius, { 255, 128, 0 });
		}

		if (interactable.triggered) {
			interactable.triggered = false;

			switch (interactable.interaction) {
				case Interaction::NoInteraction:
					no_interaction(world, interactable, entity);
					break;
				case HackerCameraJump:
					interactable.triggered = false;
					break;
				case HackerPlatform: {
					auto &platform = world.get_component<Platform>(interactable.interaction_target);
					if (!platform.is_moving) {
						platform.is_moving = true;
					}
					SPDLOG_INFO("{}", "Hacker platform triggered");
					break;
				}
				case Exploding: {
					SPDLOG_INFO("Explosion triggered");
					explosion(world, interactable, entity);
					//TODO: uncomment when explosion is no longer being tested
					//interactable.can_interact = false;
					break;
				}
			}
		}
	}
}

void InteractableSystem::no_interaction(World &world, Interactable &interactable, Entity entity) {
	SPDLOG_WARN("No interaction set for entity {}, turning off interactions on this object", entity);
	//interactable.can_interact = false;
}

void InteractableSystem::explosion(World &world, Interactable &interactable, Entity entity) {
	auto box = world.get_component<ExplodingBox>(interactable.interaction_target);
	ColliderSphere sphere;
	float larger_radius = box.distraction_radius;
	float smaller_radius = box.explosion_radius;
	if (larger_radius < smaller_radius) {
		float temp = larger_radius;
		larger_radius = smaller_radius;
		smaller_radius = temp;
	}
	sphere.radius = larger_radius;
	sphere.center = world.get_component<Transform>(interactable.interaction_target).get_position();
	auto colliders = PhysicsManager::get().overlap_sphere(world, sphere, "default");
	for (Entity entity : colliders) {
		if (world.has_component<EnemyData>(entity)) {
			auto enemy_data = world.get_component<EnemyData>(entity);
			float distance = glm::distance(sphere.center, world.get_component<Transform>(entity).get_position());
			if (distance < smaller_radius) {
				//TODO: set enemy state as dead
				//auto &enemy_sys = world.get_system<EnemySystem>(); // this would be a good way to do it i think
				auto &enemy = world.get_component<EnemyData>(entity);
				enemy.state_machine.set_state("dying");
				SPDLOG_INFO("{} is dead", entity);
			} else if (distance < larger_radius) {
				//TODO: set enemy state as distracted
				SPDLOG_INFO("{} is distracted", entity);
			}
		}
	}
}
