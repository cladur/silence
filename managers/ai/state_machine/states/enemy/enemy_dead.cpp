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
	auto &transform = world->get_component<Transform>(entity_id);
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &res = ResourceManager::get();
	auto &anim = world->get_component<AnimationInstance>(entity_id);
	auto &enemy_path = world->get_component<EnemyPath>(entity_id);
	auto &enemy_data = world->get_component<EnemyData>(entity_id);
	auto &dd = world->get_parent_scene()->get_render_scene().debug_draw;

	// change animation
	if (anim.animation_handle.id != res.get_animation_handle("enemy/enemy_ANIM_GLTF/enemy_death.anim").id) {
		animation_manager.change_animation(entity_id, "enemy/enemy_ANIM_GLTF/enemy_death.anim");
		anim.is_looping = false;
		UIManager::get().get_ui_button(std::to_string(entity_id) + "_detection", "detection_slider").display = false;

	}

}

void EnemyDead::exit() {
	SPDLOG_INFO("EnemyDead::exit");
}
