//
// Created by 7hube on 27.03.2023.
//

#include "state_machine_system.h"
#include "../../core/ai/state_machine/states/test_state.h"
#include "../../core/components/rigidbody_component.h"
#include "../../core/components/state_component.h"
#include "ecs/ecs_manager.h"

extern ECSManager ecs_manager;

void StateMachineSystem::startup() {
	states[0] = new TestState("idle");
	current_state = states[0];
	current_state->enter();
	states[1] = new TestState("falling");
}
void StateMachineSystem::update(float dt) {
	for (auto &entity : entities) {
		auto &state =  ecs_manager.get_component<State>(entity);
		state.state->update();

		RigidBody &rigid_body = ecs_manager.get_component<RigidBody>(entity);
		if (rigid_body.velocity.y < -1.0f) {
			state.state->exit();
			state.state = states[1];
			state.state->enter();
		}
	}
}
