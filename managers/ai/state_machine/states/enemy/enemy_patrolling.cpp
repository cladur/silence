#include "enemy_patrolling.h"
#include "ai/state_machine/state_machine.h"
#include "components/enemy_data_component.h"
#include "components/enemy_path_component.h"
#include "components/transform_component.h"
#include "engine/scene.h"
#include "managers/ecs/world.h"
#include "managers/gameplay/gameplay_manager.h"
#include "managers/physics/ecs/collision_system.h"
#include "managers/physics/physics_manager.h"
#include "managers/render/render_scene.h"
#include <animation/animation_manager.h>
#include <render/transparent_elements/ui_manager.h>

void look_at(EnemyPath &path, Transform &t, glm::vec3 &target, float &dt);

void EnemyPatrolling::startup(StateMachine *machine, std::string name) {
	SPDLOG_INFO("EnemyPatrolling::startup");
	this->name = name;
	set_state_machine(machine);
}

void EnemyPatrolling::enter() {
	SPDLOG_INFO("EnemyPatrolling::enter");
}

void EnemyPatrolling::update(World *world, uint32_t entity_id, float dt) {
	auto &transform = world->get_component<Transform>(entity_id);
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &res = ResourceManager::get();
	auto &anim = world->get_component<AnimationInstance>(entity_id);
	auto &enemy_path = world->get_component<EnemyPath>(entity_id);
	auto &enemy_data = world->get_component<EnemyData>(entity_id);
	auto &dd = world->get_parent_scene()->get_render_scene().debug_draw;

	if (anim.animation_handle.id != res.get_animation_handle("enemy/enemy_ANIM_GLTF/enemy_walk_with_gun.anim").id) {
		animation_manager.change_animation(entity_id, "enemy/enemy_ANIM_GLTF/enemy_walk_with_gun.anim");
	}

	glm::vec3 current_position = transform.position;
	glm::vec3 target_position = enemy_path.path[enemy_path.next_position];
	int idx = ((enemy_path.next_position - 1) % (int)enemy_path.path.size() == -1)
			? enemy_path.path.size() - 1
			: (enemy_path.next_position - 1) % enemy_path.path.size();
	enemy_path.prev_position = enemy_path.path[idx];

	if (glm::distance(current_position, target_position) > 0.1f) {
		transform.add_position(glm::normalize(target_position - current_position) * enemy_path.speed * dt);
	} else {
		enemy_path.next_position = (enemy_path.next_position + 1) % enemy_path.path.size();
	}

	// this huge if just means "when near a node on either side" start rotating
	if (glm::distance(current_position, target_position) <
					(glm::distance(enemy_path.prev_position, target_position)) * 0.1f ||
			glm::distance(current_position, enemy_path.prev_position) <
					(glm::distance(enemy_path.prev_position, target_position)) * 0.1f) {
		if (!enemy_path.is_rotating) {
			enemy_path.first_rotation_frame = true;
		}
		enemy_path.is_rotating = true;
	}

	// smoothly rotate the entity to face the next node
	if (enemy_path.is_rotating) {
		look_at(enemy_path, transform, target_position, dt);
	}

	// if the entity is facing the next node, stop rotating
	if (glm::dot(glm::normalize(target_position - transform.position), glm::normalize(transform.get_global_forward())) >
			0.99f) {
		enemy_path.is_rotating = false;
	}

	auto agent_pos = GameplayManager::get().get_agent_position(world->get_parent_scene());
	bool can_see_player = false;
	// check if agent is in cone of vision described by view_cone_angle and view_cone_distance
	if (glm::distance(transform.position, agent_pos) < enemy_data.view_cone_distance) {
		auto agent_dir = glm::normalize(agent_pos - transform.position);
		auto forward = glm::normalize(transform.get_global_forward());
		dd.draw_line(transform.position, transform.position + agent_dir, glm::vec3(1.0f, 0.0f, 0.0f));
		dd.draw_line(transform.position, transform.position + forward, glm::vec3(0.0f, 1.0f, 0.0f));
		auto angle = glm::acos(glm::dot(agent_dir, forward));
		if (angle < glm::radians(enemy_data.view_cone_angle) / 2.0f) {
			Ray ray{};
			ray.origin = transform.position + agent_dir + glm::vec3(0.0f, 0.5f, 0.0f);
			ray.direction = agent_dir;
			glm::vec3 ray_end = ray.origin + ray.direction * enemy_data.view_cone_distance;

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
	//SPDLOG_INFO("detection level: {}", enemy_data.detection_level);

	auto &slider = UIManager::get().get_ui_slider(std::to_string(entity_id) + "_detection", "detection_slider");
	slider.value = enemy_data.detection_level;
	// lerp from white to red
	slider.color = glm::lerp(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), slider.value);
	slider.position = transform.get_global_position() + glm::vec3(0.0f, 2.0f, 0.0f);

	if (enemy_data.detection_level > 0.75f) {
		state_machine->set_state("looking");
	}
}

void EnemyPatrolling::exit() {
	SPDLOG_INFO("EnemyPatrolling::exit");
}

void look_at(EnemyPath &path, Transform &t, glm::vec3 &target, float &dt) {
	if (path.first_rotation_frame) {
		auto current_no_y = glm::vec3(t.position.x, 0.0f, t.position.z);
		auto target_no_y = glm::vec3(target.x, 0.0f, target.z);

		glm::vec3 direction = glm::normalize(target_no_y - current_no_y);
		glm::vec3 forward = glm::normalize(glm::vec3(t.get_global_forward().x, 0.0f, t.get_global_forward().z));
		float angle = glm::acos(glm::dot(forward, direction));
		glm::vec3 axis = glm::cross(forward, direction);
		path.rotation_end = (angle * axis);
		path.first_rotation_frame = false;
	}
	t.add_global_euler_rot(path.rotation_end * dt * path.rotation_speed);
}