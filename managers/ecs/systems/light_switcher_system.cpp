#include "ecs/systems/light_switcher_system.h"
#include "components/light_switcher_component.h"
#include "ecs/world.h"
#include "gameplay/gameplay_manager.h"

void LightSwitcherSystem::startup(World &world) {
	Signature whitelist;
	whitelist.set(world.get_component_type<LightSwitcher>());
	whitelist.set(world.get_component_type<Light>());

	world.set_system_component_whitelist<LightSwitcherSystem>(whitelist);
}

void LightSwitcherSystem::switch_light(World &world, const Entity &entity, LightSwitcher &light_switcher) {
	light_switcher.current_time = 0.0f;
	light_switcher.time_to_switch = 0.0f;
	light_switcher.is_waiting = false;

	auto &light = world.get_component<Light>(entity);
	light.is_on = !light.is_on;
}

void LightSwitcherSystem::calculate_next_switch(LightSwitcher &light_switcher, bool light_is_on) {
	light_switcher.is_waiting = true;

	float switch_time;
	float switch_variance;

	if (light_is_on) {
		switch_time = light_switcher.turn_off_time;
		switch_variance = light_switcher.turn_off_variance;
	} else {
		switch_time = light_switcher.turn_on_time;
		switch_variance = light_switcher.turn_on_variance;
	}

	std::uniform_real_distribution<float> distribution(-switch_variance, switch_variance);
	float random_value = distribution(GameplayManager::random_generator);
	float next_switch_time = switch_time + random_value;

	light_switcher.time_to_switch = next_switch_time;
}

void LightSwitcherSystem::update(World &world, float dt) {
	ZoneScopedN("LightSwitcherSystem::update");
	for (auto const &entity : entities) {
		auto &light_switcher = world.get_component<LightSwitcher>(entity);
		auto &light = world.get_component<Light>(entity);

		if (!light_switcher.is_waiting) {
			calculate_next_switch(light_switcher, light.is_on);
		}

		light_switcher.current_time += dt;

		if (light_switcher.current_time >= light_switcher.time_to_switch) {
			switch_light(world, entity, light_switcher);
		}
	}
}