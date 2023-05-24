#include "enemy_looking.h"
#include "ai/state_machine/state_machine.h"
#include "components/enemy_data_component.h"
#include "components/enemy_path_component.h"
#include "components/transform_component.h"
#include "engine/scene.h"
#include "managers/ecs/systems/collision_system.h"
#include "managers/ecs/world.h"
#include "managers/gameplay/gameplay_manager.h"
#include "managers/physics/physics_manager.h"
#include "managers/render/render_scene.h"
#include <animation/animation_manager.h>
#include <render/transparent_elements/ui_manager.h>

void EnemyLooking::startup(StateMachine *machine, std::string name) {
	SPDLOG_INFO("EnemyLooking::startup");
	this->name = name;
	set_state_machine(machine);
}

void EnemyLooking::enter() {
	first_frame = true;
	SPDLOG_INFO("EnemyLooking::enter");
}

void EnemyLooking::update(World *world, uint32_t entity_id, float dt) {
	auto &transform = world->get_component<Transform>(entity_id);
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &res = ResourceManager::get();
	auto &anim = world->get_component<AnimationInstance>(entity_id);
	auto &enemy_data = world->get_component<EnemyData>(entity_id);
	auto &dd = world->get_parent_scene()->get_render_scene().debug_draw;

	// some necessary variables
	auto agent_pos = GameplayManager::get().get_agent_position(world->get_parent_scene());
	auto forward = glm::normalize(transform.get_global_forward());
	auto current_no_y = glm::vec3(transform.position.x, 0.0f, transform.position.z);
	glm::vec3 target_look = glm::vec3(agent_pos.x, 0.0f, agent_pos.z);
	glm::vec3 direction = glm::normalize(target_look - current_no_y);
	glm::vec3 forward_no_y = glm::normalize(glm::vec3(transform.get_global_forward().x, 0.0f, transform.get_global_forward().z));

	if (first_frame) {
		float angle = glm::acos(glm::dot(forward_no_y, direction));
		glm::vec3 axis = glm::cross(forward_no_y, direction);
		rotation_end = (angle * axis);

		if (anim.animation_handle.id !=
				res.get_animation_handle("agent/agent_ANIM_GLTF/agent_idle.anim").id) {
			animation_manager.change_animation(entity_id, "agent/agent_ANIM_GLTF/agent_idle.anim");
		}

		first_frame = false;
	}

	// smoothly look at the last known player position, stop if already looking
	float dot = glm::dot(glm::normalize(target_look - transform.position), forward);
	if ( dot < 0.99f) {
		transform.add_global_euler_rot(rotation_end * dt * 2.0f);
	}
	dd.draw_line(transform.position + glm::vec3(0.0f, 0.1f, 0.0f), target_look, glm::vec3(1.0f, 0.0f, 0.0f));
	dd.draw_line(transform.position + glm::vec3(0.0f, 0.1f, 0.0f), transform.position + forward, glm::vec3(0.0f, 1.0f, 0.0f));

	bool can_see_player = false;
	// check if agent is in cone of vision described by view_cone_angle and view_cone_distance
	if (glm::distance(transform.position, agent_pos) < enemy_data.view_cone_distance) {
		auto agent_dir = glm::normalize(agent_pos - transform.position);

		auto angle = glm::acos(glm::dot(agent_dir, forward));
		if (angle < enemy_data.view_cone_angle) {
			Ray ray{};
			ray.origin = transform.position + agent_dir + glm::vec3(0.0f, 0.5f, 0.0f);
			ray.direction = agent_dir;
			glm::vec3 ray_end = ray.origin + ray.direction * enemy_data.view_cone_distance;
			dd.draw_arrow(ray.origin, ray_end, glm::vec3(1.0f, 0.0f, 0.0f));

			HitInfo hit_info;

			// todo: make sure this checks properly for hitting a player and stops at terrain in between
			if (CollisionSystem::ray_cast(*world, ray, hit_info)) {
				auto hit_name = world->get_component<Name>(hit_info.entity);
				if (hit_info.entity == GameplayManager::get().get_agent_entity()) {
					dd.draw_arrow(ray.origin, ray_end, glm::vec3(1.0f, 0.0f, 0.0f));
					can_see_player = true;
				}
			}
		}
	}

	if (can_see_player) {
		enemy_data.detection_level += dt / enemy_data.detection_speed;
	} else {
		enemy_data.detection_level -= dt / enemy_data.detection_speed;
	}

	enemy_data.detection_level = glm::clamp(enemy_data.detection_level, 0.0f, 1.0f);

	auto &slider = UIManager::get().get_ui_slider(std::to_string(entity_id) + "_detection", "detection_slider");
	slider.value = enemy_data.detection_level;
	// lerp from white to red
	slider.color = glm::lerp(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), slider.value);
	slider.position = transform.get_global_position() + glm::vec3(0.0f, 2.0f, 0.0f);

	// thresholded detection level, switches to previous state when it goes very low, to avoid jittering and give it more reealism
	if (enemy_data.detection_level < 0.2) {
		state_machine->set_state("patrolling");
	}
}

void EnemyLooking::exit() {
	first_frame = true;
	SPDLOG_INFO("EnemyLooking::exit");
}


