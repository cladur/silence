//
// Created by 7hube on 27.03.2023.
//

#ifndef SILENCE_STATE_MACHINE_SYSTEM_H
#define SILENCE_STATE_MACHINE_SYSTEM_H

#include "../../core/ai/state_machine/base_state.h"
#include "ecs/base_system.h"
class StateMachineSystem : public BaseSystem {
private:
	BaseState* current_state;
	BaseState* states[2];
public:
	void startup();
	void update(float dt);
};

#endif //SILENCE_STATE_MACHINE_SYSTEM_H
