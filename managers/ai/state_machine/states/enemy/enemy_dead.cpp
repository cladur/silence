#include "enemy_dead.h"
#include "ai/state_machine/state_machine.h"
#include "components/enemy_data_component.h"
#include "components/enemy_path_component.h"
#include "components/transform_component.h"
#include "engine/scene.h"
#include "managers/ecs/world.h"
#include "managers/render/render_scene.h"
#include <animation/animation_manager.h>
#include "enemy_utils.h"

void EnemyDead::startup(StateMachine *machine, std::string name) {
	SPDLOG_INFO("EnemyDead::startup");
	this->name = name;
	set_state_machine(machine);
}

void EnemyDead::enter() {
	SPDLOG_INFO("EnemyDead::enter");
}

void EnemyDead::update(World *world, uint32_t entity_id, float dt) {
	auto &enemy_data = world->get_component<EnemyData>(entity_id);
	if (enemy_data.is_dead) {
		return;
	}
	// all of this happens only once. dead enemy does not need any logic.
	auto &transform = world->get_component<Transform>(entity_id);
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &res = ResourceManager::get();
	auto &anim = world->get_component<AnimationInstance>(entity_id);
	auto &enemy_path = world->get_component<EnemyPath>(entity_id);
	auto &dd = world->get_parent_scene()->get_render_scene().debug_draw;

	enemy_data.detection_level = 0.0f;
	GameplayManager::get().add_detection_level(enemy_data.detection_level);

	// change animation
	if (anim.animation_handle.id != res.get_animation_handle("enemy/enemy_ANIM_GLTF/enemy_death.anim").id) {
		animation_manager.change_animation(entity_id, "enemy/enemy_ANIM_GLTF/enemy_death.anim");
		anim.is_looping = false;
		UIManager::get().get_ui_slider(std::to_string(entity_id) + "_detection", "detection_slider").display = false;
	}

	if (world->has_component<Taggable>(entity_id)) {
		auto &taggable = world->get_component<Taggable>(entity_id);
		taggable.enabled = false;
	}

	AudioManager::get().play_one_shot_3d(enemy_data.death_event, transform);

	enemy_data.is_dead = true;
}

void EnemyDead::exit() {
	SPDLOG_INFO("EnemyDead::exit");
}
